#include "monitor.h"
#include "logger.h"
#include "helpers.h"
#include "database/settings.h"
#include "database/database.h"
#include "notifications_controller.h"
#include <chrono>
#include <list>

using namespace alefbet::pctrl::logger;
using namespace alefbet::pctrl::helpers;
using namespace alefbet::pctrl::database;
using namespace std::chrono_literals;

constexpr std::chrono::minutes MainLoopDelayInMinutes = 1min;
constexpr std::chrono::nanoseconds MainLoopDelayInNanos = std::chrono::duration_cast<std::chrono::nanoseconds>(MainLoopDelayInMinutes); 
constexpr std::chrono::milliseconds SubLoopDelayInMillis = 5s;
constexpr std::chrono::nanoseconds SubLoopDelayInNanos = std::chrono::duration_cast<std::chrono::nanoseconds>(SubLoopDelayInMillis); 

namespace alefbet::pctrl::srv {

    /*! \brief Returns the remaining time for the current user in minutes
    The remaining time can be negative when the user outpass his limit.
    */
    /*s16 Monitor::remainingTimeInMinutes(const HistoryEntry& entry, const u16& daily_limit) {        
        logDebug("[Monitor] daily limit=%i\n", daily_limit);
        logDebug("[Monitor] duration=%i\n", entry.durationInMinutes());

                 
    }*/

    void Monitor::start() {
        if(running_) return;
        running_ = true;
    }

    void Monitor::loop() {        
        logDebug("[Monitor] Starting monitoring loop\n");        
        //WorkingMode workingMode = WorkingModeInfo;        
        u64 pid = 0;
        structs::UserData user;

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
            // Query the active application and user
            // and update the database
            pid = getRunningApplicationPid();  
            u16 daily_limit = 0;          
            
            //const auto daily_limit = settings[SETTING_DAILY_LIMIT_GLOBAL].int_value;            

            if(pid != 0) { 
                handleAuthentication(pid);

                const auto title_id = getRunningApplicationTitleId(pid);
                user = getCurrentUser();                

                if(user.isValid()) {
                    logDebug("[Monitor] Title %i is currently in used by %s. Updating history.\n", title_id, user.nickname.c_str());                    

                    //Update working mode
                    //Show remaining time is disabled
                    /*if(settings.contains(SETTING_WORKING_MODE)) {
                        workingMode = (WorkingMode)settings[SETTING_WORKING_MODE].int_value;
                        logDebug("[Monitor] Working mode is %i\n", workingMode);
                    }

                    if(settings.contains(SETTING_SHOW_REMAINING_TIME)) {
                        if(settings[SETTING_SHOW_REMAINING_TIME].int_value == 1) {
                            logDebug("[Monitor] Show remaining time.\n");
                            service_->gui().ShowRemainingTime();
                        }
                    } else {
                        logDebug("[Monitor] Force show remaining time (TEST)\n");
                        service_->gui().ShowRemainingTime();
                    }*/

                    const auto entry = addToHistory(user.uid, title_id, MainLoopDelayInMinutes.count());                    

                    if(!entry.isValid()) {
                        continue;
                    }                    

                    const auto& userId = accountUidToString(user.uid);
                    daily_limit = getDailyLimitForUser(userId);

                    //const auto uid_str = accountUidToString(user.uid);
                    u16 remainingTimeInMinutes = daily_limit > entry.durationInMinutes() ? daily_limit - entry.durationInMinutes() : 0;
                    logDebug("[Monitor] Remaining time for user %s is %i minutes. Daily limit=%i\n", user.nickname.c_str(), remainingTimeInMinutes, daily_limit);

                    // DISABLED
                    if(settings.contains(SETTING_SHOW_REMAINING_TIME)) {
                        const auto& showRemainingTime = settings[SETTING_SHOW_REMAINING_TIME].int_value > 0;                                                
                        if(showRemainingTime) {                            
                            if(shouldSendNotification(remainingTimeInMinutes)) {
                                NotificationsController::notifyRemainingTime(remainingTimeInMinutes);
                            }
                        }
                    }
                    
                    // After database update we need to verify the limits
                    //const auto remaining_time = remainingTimeInMinutes(entry);                                    

                    if(remainingTimeInMinutes <= 0) {
                        logInfo("[Monitor] Timeout for the user %s\n", user.nickname.c_str());
                        //service_->gui().ShowScreenTimeout();
                        service_->showScreenTimeout();
                    } /*else { // DISABLED
                        logDebug("[Monitor] Remaining time for user %s is %i minutes.\n", user.nickname.c_str(), remainingTimeInMinutes);
                        service_->gui().updateRemainingTimePanel(remainingTimeInMinutes, daily_limit);
                    }*/
                } else {
                    logDebug("[Monitor] No user found\n");
                }
            } else {
                logDebug("[Monitor] No title running\n");
                /*if(service_->gui().isRemainingTimePanelVisible()) { // DISABLED
                    service_->gui().hideRemainingTimePanel();
                }*/
                if(currentTitle_ > 0) {
                    logDebug("[Monitor] The game has been closed\n");

                    // Hide the panel                    
                    service_->hideAllScreens();

                    currentTitle_ = 0;
                    currentUser_.clear();
                }
            }

            // DISABLED
            // Sub-loop to monitor games usage and show/hide the remaining time panel
            /*for(int i = 0 ; i < MainLoopDelayInNanos.count() / SubLoopDelayInNanos.count() ; ++i) {
                pid = getRunningApplicationPid();   
                if(pid == 0 && service_->gui().isRemainingTimePanelVisible()) {
                    // If no app is running we hide the remaining time panel
                    service_->gui().hideRemainingTimePanel();
                }

                svcSleepThread(SubLoopDelayInNanos.count()); // Wait a little
            }*/

            // Sub-loop to monitor blacklisted games
            for(int i = 0 ; i < MainLoopDelayInNanos.count() / SubLoopDelayInNanos.count() ; ++i) {
                pid = getRunningApplicationPid();   
                if(pid != 0 && helpers::isCurrentTitleBlacklisted()) {
                    service_->showPanelTitleBlacklisted();
                }

                svcSleepThread(SubLoopDelayInNanos.count()); // Wait a little
            }

            logDebug("[Monitoring] loop\n");
            //svcSleepThread(MainLoopDelayInNanos.count()); // Wait 1 minute
        }

        logInfo("[Monitor] Stopped monitoring.\n");
    }

    void Monitor::stop() {
        logInfo("[Monitor] Stopping monitor\n");
        running_ = false;
        notified_ = false;
    }

    bool Monitor::shouldSendNotification(int remainingTimeInMinutes) {
        // Remaining time notitications are sent every 15 minutes and every minute during the last 5 minutes
        return remainingTimeInMinutes % 15 == 0 || remainingTimeInMinutes <= 5;
    }

    void Monitor::handleAuthentication(u64 pid) {
        auto settings = loadSettings();
        if(!settings.contains(SETTING_AUTHENTICATION)) {
            // The authentication is an optional feature
            logDebug("[Monitor] Authentication is not set\n");
            return; 
        }

        if(settings[SETTING_AUTHENTICATION].int_value == 0) {
            logDebug("[Monitor] Authentication is disabled\n");
            return;
        }

        const auto& currentTitle = getRunningApplicationTitleId(pid);
        const auto& currentUser = getCurrentUser();

        if(currentTitle != currentTitle_ && currentUser != currentUser_) {
            logDebug("[Monitor] handle running app %i\n", pid);

            currentTitle_ = currentTitle;
            currentUser_ = currentUser;

            logDebug("[Monitor] The current game and/or user has changed\n");
            
            service_->showPanelAuthentication();
        }
    }
}