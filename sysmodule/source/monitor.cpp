/*
 * NS Parental Control
 * Copyright (C) 2025 Tristan Israël
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "monitor.h"
#include "logger.h"
#include "helpers.h"
#include "database/settings.h"
#include "database/history_db.h"
#include "database/settings_db.h"
#include "notifications_controller.h"
#include <chrono>
#include <list>
#include <algorithm>
//#include "pctrl_screen.hpp"

using namespace alefbet::pctrl::logger;
using namespace alefbet::pctrl::helpers;
using namespace alefbet::pctrl::database::settings;
using namespace alefbet::pctrl::database::history;
using namespace std::chrono_literals;

constexpr std::chrono::minutes MainLoopDelayInMinutes = 1min;
constexpr std::chrono::nanoseconds MainLoopDelayInNanos = std::chrono::duration_cast<std::chrono::nanoseconds>(MainLoopDelayInMinutes);
constexpr u64 BlockConfirmationDelayInNanos = 10'000'000'000ULL; // Delay before confirming a block (user selected after the launch)
constexpr u64 UserSelectionPollDelayInNanos = 1'000'000'000ULL;  // Verification delay while the user is being selected
constexpr u64 UserSelectionMaxWaitInNanos = 120'000'000'000ULL;  // Maximum wait for the end of the user selection
constexpr u64 UserSelectionGracePeriodInNanos = 30'000'000'000ULL; // Maximum wait for a game to ask for the user after its launch
constexpr u64 EnforcementDelayInNanos = 5'000'000'000ULL; // Delay between two verifications of the rules while waiting

namespace {
    u64 nowInNanos() {
        return armTicksToNs(armGetSystemTick());
    }
}

namespace alefbet::pctrl::srv {

    /*! @brief Sleeps until the next iteration of the monitoring loop.
     *
     * The iterations are scheduled on fixed deadlines so that the processing time
     * of an iteration does not delay the following ones: one counted minute is
     * one real minute. After a long pause (console sleep) a new period starts.
     *
     * While waiting, the play rules are verified regularly so that a game
     * launched while the user is not allowed to play is stopped within a few
     * seconds instead of up to one minute.
     *
     * @param nextIterationNs Deadline of the current iteration (system tick in ns), updated in place.
     */
    void Monitor::waitForNextIteration(u64& nextIterationNs) {
        const u64 period = MainLoopDelayInNanos.count();
        nextIterationNs += period;

        u64 now = nowInNanos();
        if(now >= nextIterationNs) {
            nextIterationNs = now + period;
        }

        while(now < nextIterationNs) {
            svcSleepThread(std::min(nextIterationNs - now, EnforcementDelayInNanos));

            if(running_) {
                enforceRulesForRunningApplication();
            }

            now = nowInNanos();
        }
    }

    /*! @brief Checks whether the current user of the running application is known.
     *
     * Until the user is selected, the account service still returns the
     * previous user, so no decision can be taken:
     * - while the user selection ("Who is playing?") is displayed;
     * - for a title that does not have its user selected by the HOME menu
     *   before its launch, during the first seconds after the launch, until the
     *   user selection has been displayed and closed (a game may ask for the
     *   user after its loading screens). After UserSelectionGracePeriodInNanos
     *   the current user is used, for the games that never ask for the user.
     *
     * @param pid Process ID of the running application.
     * @return true if the current user can be used for the decisions.
     */
    bool Monitor::isUserContextSettled(u64 pid) {
        if(pid == 0) return false;

        if(pid != launchPid_) {
            // A new application has been launched
            launchPid_ = pid;
            launchTimeNs_ = nowInNanos();
            userSelectionSeen_ = false;
            userSelectedBeforeLaunch_ = isUserSelectedBeforeLaunch(getRunningApplicationTitleId(pid));
            logDebug("[Monitor] New application, user selected before launch: %s\n", userSelectedBeforeLaunch_ ? "yes" : "no");
        }

        if(isUserSelectionInProgress()) {
            userSelectionSeen_ = true;
            return false;
        }

        return userSelectedBeforeLaunch_ || userSelectionSeen_ ||
               nowInNanos() - launchTimeNs_ >= UserSelectionGracePeriodInNanos;
    }

    /*! @brief Checks whether the running application must be stopped for its current user.
     *
     * Uses the same tamper-protected decision as the monitoring loop
     * (core::evaluatePlayRules) but does not record any usage. The current user
     * is read again at each call: it changes when a game asks for the user after
     * its launch.
     *
     * @param pid Process ID of the application the decision is about.
     * @param titleBlocked Set to true if the application is blocked because the title is blocked for the user (optional).
     * @return true if the application is still running and its user is not allowed to play.
     */
    bool Monitor::isRunningApplicationBlocked(u64 pid, bool* titleBlocked) {
        if(titleBlocked != nullptr) *titleBlocked = false;
        if(pid == 0 || getRunningApplicationPid() != pid) return false;

        // Until the user is selected, the current user is still the previous one
        if(!isUserContextSettled(pid)) return false;

        const auto user = getCurrentUser();
        if(!user.isValid()) return false;

        auto settings = loadSettings();
        const bool isEnabled = !settings.contains(SETTING_ENABLED) ||
                               settings[SETTING_ENABLED].int_value != 0;
        if(!isEnabled) return false;

        const auto& userId = accountUidToString(user.uid);
        const auto titleId = getRunningApplicationTitleId(pid);
        const auto eval = core::evaluatePlayRules(getDailyLimitForUser(userId),
                                                  getUserUsageTimeForToday(user.uid),
                                                  getAdjustedCurrentTime(),
                                                  getGameStartTime(userId),
                                                  getBedtime(userId),
                                                  false,
                                                  isTitleBlacklistedForUser(userId, titleId));

        if(eval.shouldBlock) {
            logInfo("[Monitor] Application used by %s while not allowed to play\n", user.nickname.c_str());
        }

        if(titleBlocked != nullptr) *titleBlocked = eval.titleBlocked;
        return eval.shouldBlock;
    }

    /*! @brief Stops the running application after the timeout countdown.
     *
     * Many games are launched before the user is selected: until then, the
     * current user is the previous one. The decision is therefore confirmed a few
     * seconds later before the countdown starts, and evaluated again during the
     * countdown, which is cancelled when the current user is allowed to play.
     * When the title is blocked for the user, the user is told why before the
     * countdown starts.
     *
     * @param pid Process ID of the running application.
     */
    void Monitor::stopApplication(u64 pid) {
        svcSleepThread(BlockConfirmationDelayInNanos);

        // Wait for the end of the user selection: the decision must be taken on the selected user
        for(u64 waited = 0 ; waited < UserSelectionMaxWaitInNanos && !isUserContextSettled(pid) ; waited += UserSelectionPollDelayInNanos) {
            svcSleepThread(UserSelectionPollDelayInNanos);
        }

        bool titleBlocked = false;
        if(!isRunningApplicationBlocked(pid, &titleBlocked)) {
            logInfo("[Monitor] The block is not confirmed, the application keeps running\n");
            return;
        }

        if(titleBlocked) {
            logInfo("[Monitor] The title is blocked for the user\n");
            NotificationsController::notifyTitleNotAllowed();
        }

        service_->showScreenTimeout(pid, [this, pid]() {
            return isRunningApplicationBlocked(pid);
        });
    }

    /*! @brief Stops the running application if its user is not allowed to play. */
    void Monitor::enforceRulesForRunningApplication() {
        const u64 pid = getRunningApplicationPid();

        if(isRunningApplicationBlocked(pid)) {
            stopApplication(pid);
        }
    }

    /*! @brief Starts the monitoring loop if not already running.
     *
     * This only sets a flag: the monitoring thread (created once at startup)
     * wakes up on its own. It cannot restart a thread that has exited, which
     * is why Monitor::loop never returns.
     */
    void Monitor::start() {
        if(running_) return;
        running_ = true;
    }

    /*! @brief Main monitoring loop that tracks application usage and enforces limits.
     *
     * The loop never exits: when the service is disabled it goes idle so that
     * the monitoring thread stays alive and automatically resumes when the
     * service is enabled again.
     */
    void Monitor::loop() {        
        logDebug("[Monitor] Starting monitoring loop\n");        
        structs::UserData user;
        u64 pid = 0;
        bool wasEnabled = false;
        u64 nextIterationNs = nowInNanos();

        while(true) {
            if(!running_) {
                // Do nothing, but keep the thread alive so that the monitoring
                // can be resumed with start()
                if(wasEnabled) {
                    logInfo("[Monitor] Parental Control is now disabled.\n");
                    wasEnabled = false;
                }
                resetTimeJumpState(); // A long idle period must not be mistaken for a clock manipulation
                svcSleepThread(500'000'000); // Wait for 500ms
                nextIterationNs = nowInNanos();
                continue;
            }

            logDebug("[Monitor] Monitoring loop has started\n");
            auto settings = loadSettings();

            // Verify whether the service is enabled
            const bool isEnabled = !settings.contains(SETTING_ENABLED) ||
                                   settings[SETTING_ENABLED].int_value != 0;

            if(!isEnabled) {
                if(wasEnabled) {
                    logInfo("[Monitor] Parental Control is now disabled.\n");
                    wasEnabled = false;
                }

                // Do not exit the loop: go idle so that the monitoring
                // automatically resumes when the service is enabled again
                resetTimeJumpState();
                svcSleepThread(MainLoopDelayInNanos.count());
                nextIterationNs = nowInNanos();
                continue;
            }

            if(!wasEnabled) {
                logInfo("[Monitoring] Parental control is now enabled.\n");
                wasEnabled = true;
            }

            if(!notified_) {
                NotificationsController::notifyMonitoringStarted();
                notified_ = true;
            }

            logDebug("[Monitor] Update usages\n");
            // Update time jump offset even when no game is running
            const auto adjustedTimeMinutes = getAdjustedCurrentTime();
            
            // Query the active application and user
            // and update the database
            pid = getRunningApplicationPid();
            u16 daily_limit = 0;            
            
            if(pid != 0) {
                const auto title_id = getRunningApplicationTitleId(pid);
                user = getCurrentUser();

                if(!isUserContextSettled(pid)) {
                    // The current user is still the previous one: neither record nor evaluate this minute
                    logDebug("[Monitor] The user is being selected\n");
                } else if(user.isValid()) {
                    logDebug("[Monitor] Title %i is currently in used by %s. Updating history.\n", title_id, user.nickname.c_str());                    

                    const auto& entry = addToHistory(user.uid, title_id, MainLoopDelayInMinutes.count());

                    if(!entry.isValid()) {
                        logError("[Monitor] The usage could not be recorded\n");
                        // Never skip the wait, otherwise the loop would spin without pausing
                        waitForNextIteration(nextIterationNs);
                        continue;
                    }

                    const auto totalDuration = getUserUsageTimeForToday(user.uid);                    

                    const auto& userId = accountUidToString(user.uid);
                    daily_limit = getDailyLimitForUser(userId);

                    const auto gameStartTime = getGameStartTime(userId);
                    const auto bedtime = getBedtime(userId);

                    bool showRemainingTime = false;
                    if(settings.contains(SETTING_SHOW_REMAINING_TIME)) {
                        showRemainingTime = settings[SETTING_SHOW_REMAINING_TIME].int_value > 0;
                    }

                    // Pure enforcement decision (tamper-protected time + play window + daily limit + blocked titles)
                    const auto eval = core::evaluatePlayRules(daily_limit, totalDuration, adjustedTimeMinutes, gameStartTime, bedtime, showRemainingTime,
                                                              isTitleBlacklistedForUser(userId, title_id));

                    logDebug("[Monitor] Remaining time for user %s is %i minutes. Daily limit=%i\n", user.nickname.c_str(), eval.remainingTimeInMinutes, daily_limit);

                    if(eval.outsidePlayWindow) {
                        logInfo("[Monitor] Current time %s is outside the play window (%s to %s) for user %s\n", formatTimeInMinutes(adjustedTimeMinutes).c_str(), formatTimeInMinutes(gameStartTime).c_str(), formatTimeInMinutes(bedtime).c_str(), user.nickname.c_str());
                    }

                    if(eval.shouldNotify) {
                        NotificationsController::notifyRemainingTime(eval.remainingTimeInMinutes);
                    } else if(eval.shouldBlock) {
                        logInfo("[Monitor] Timeout for the user %s\n", user.nickname.c_str());
                        stopApplication(pid);
                    }
                } else {
                    logDebug("[Monitor] No user found\n");
                }
            } else {
                logDebug("[Monitor] No title running\n");

                if(currentTitle_ > 0) {
                    logDebug("[Monitor] The game has been closed\n");

                    currentTitle_ = 0;
                }
            }

            logDebug("[Monitoring] loop\n");
            waitForNextIteration(nextIterationNs);
        }
    }

    /*! @brief Puts the monitoring loop to sleep and resets notification state.
     *
     * The loop does not exit: it goes idle and resumes when start() is called
     * again or when the enabled setting is set back to 1.
     */
    void Monitor::stop() {
        logInfo("[Monitor] Stopping monitor\n");
        running_ = false;
        notified_ = false;
    }

}