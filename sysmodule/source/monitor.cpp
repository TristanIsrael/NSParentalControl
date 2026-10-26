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
//#include "pctrl_screen.hpp"

using namespace alefbet::pctrl::logger;
using namespace alefbet::pctrl::helpers;
using namespace alefbet::pctrl::database::settings;
using namespace alefbet::pctrl::database::history;
using namespace std::chrono_literals;

constexpr std::chrono::minutes MainLoopDelayInMinutes = 1min;
constexpr std::chrono::nanoseconds MainLoopDelayInNanos = std::chrono::duration_cast<std::chrono::nanoseconds>(MainLoopDelayInMinutes); 

namespace alefbet::pctrl::srv {   

    /*! @brief Starts the monitoring loop if not already running. */
    void Monitor::start() {
        if(running_) return;
        running_ = true;
    }

    /*! @brief Main monitoring loop that tracks application usage and enforces limits. */
    void Monitor::loop() {        
        logDebug("[Monitor] Starting monitoring loop\n");        
        structs::UserData user;
        u64 pid = 0;

        while(true) {
            if(!running_) {
                //Do nothing
                svcSleepThread(500'000'000); // Wait for 500ms
                continue;
            }

            if(!notified_) {
                logInfo("[Monitoring] Parental control is now enabled.\n");
                NotificationsController::notifyMonitoringStarted();
                notified_ = true;
            }
            
            logDebug("[Monitor] Monitoring loop has started\n");
            auto settings = loadSettings();

            // Verify whether the service is enabled
            if(settings.contains(SETTING_ENABLED)) {
                const auto setting = settings[SETTING_ENABLED];
                if(setting.int_value == 0) {
                    logInfo("[Monitor] Parental Control is now disabled.\n");
                    running_ = false;
                    return;
                }
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
                        logError("[Monitor] The database entry is corrupted\n");
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
            svcSleepThread(MainLoopDelayInNanos.count()); //Wait 1 minute
        }

        logInfo("[Monitor] Stopped monitoring.\n");
    }

    /*! @brief Stops the monitoring loop and resets notification state. */
    void Monitor::stop() {
        logInfo("[Monitor] Stopping monitor\n");
        running_ = false;
        notified_ = false;
    }

}