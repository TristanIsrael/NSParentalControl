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

    /*! @brief Stops the running application if its user is not allowed to play.
     *
     * Uses the same tamper-protected decision as the monitoring loop
     * (core::evaluatePlayRules) but does not record any usage.
     */
    void Monitor::enforceRulesForRunningApplication() {
        const u64 pid = getRunningApplicationPid();
        if(pid == 0) return;

        const auto user = getCurrentUser();
        if(!user.isValid()) return;

        auto settings = loadSettings();
        const bool isEnabled = !settings.contains(SETTING_ENABLED) ||
                               settings[SETTING_ENABLED].int_value != 0;
        if(!isEnabled) return;

        const auto& userId = accountUidToString(user.uid);
        const auto eval = core::evaluatePlayRules(getDailyLimitForUser(userId),
                                                  getUserUsageTimeForToday(user.uid),
                                                  getAdjustedCurrentTime(),
                                                  getGameStartTime(userId),
                                                  getBedtime(userId),
                                                  false);

        if(eval.shouldBlock) {
            logInfo("[Monitor] Application used by %s while not allowed to play\n", user.nickname.c_str());
            service_->showScreenTimeout(pid);
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
            const auto currentTimeMinutes = getCurrentTimeInMinutesSinceMidnight();
            const auto adjustedTimeMinutes = getAdjustedTime(currentTimeMinutes, lastTime_, timeJumpOffset_);
            lastTime_ = currentTimeMinutes;
            
            // Query the active application and user
            // and update the database
            pid = getRunningApplicationPid();
            u16 daily_limit = 0;            
            
            if(pid != 0) {
                const auto title_id = getRunningApplicationTitleId(pid);
                user = getCurrentUser();

                if(user.isValid()) {
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

                    // Pure enforcement decision (tamper-protected time + play window + daily limit)
                    const auto eval = core::evaluatePlayRules(daily_limit, totalDuration, adjustedTimeMinutes, gameStartTime, bedtime, showRemainingTime);

                    logDebug("[Monitor] Remaining time for user %s is %i minutes. Daily limit=%i\n", user.nickname.c_str(), eval.remainingTimeInMinutes, daily_limit);

                    if(eval.outsidePlayWindow) {
                        logInfo("[Monitor] Current time %s is outside the play window (%s to %s) for user %s\n", formatTimeInMinutes(adjustedTimeMinutes).c_str(), formatTimeInMinutes(gameStartTime).c_str(), formatTimeInMinutes(bedtime).c_str(), user.nickname.c_str());
                    }

                    if(eval.shouldNotify) {
                        NotificationsController::notifyRemainingTime(eval.remainingTimeInMinutes);
                    } else if(eval.shouldBlock) {
                        logInfo("[Monitor] Timeout for the user %s\n", user.nickname.c_str());
                        service_->showScreenTimeout(pid);
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