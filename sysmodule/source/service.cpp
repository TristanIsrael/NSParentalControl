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
#include <switch.h>
#include <thread>
#include <cstring>
#include <map>
#include <list>
#include <sstream>
#include "service.h"
#include "logger.h"
#include "ipc/Command.hpp"
#include "ipc/Result.hpp"
#include "helpers.h"
#include "database/settings.h"
#include "database/history_db.h"
#include "database/settings_db.h"
#include "monitor.h"
#include "notifications_controller.h"

using namespace alefbet::pctrl::logger;
using namespace alefbet::pctrl::database::history;
using namespace alefbet::pctrl::database::settings;
using namespace alefbet::pctrl::helpers;

constexpr std::string NullString = std::string("(NULL)");
constexpr u64 DefaultPin[] = { HidNpadButton_A, HidNpadButton_A, HidNpadButton_A, HidNpadButton_A };

namespace alefbet::pctrl::srv {       

    /*! @brief Initializes the service and registers the IPC request handler.
     *
     * Checks the enabled state from the settings database on startup.
     *
     * @param ipcServer Pointer to the IPC server for request routing.
     */
    Service::Service(Ipc::Server* ipcServer)
    : ipcServer_(ipcServer) {
        logInfo("[Service] Starting service\n");        
        ipcServer_->setRequestHandler([this](Ipc::Request * r) -> uint32_t {
            return static_cast<uint32_t>(this->commandThread(r));
        });
        logInfo("[Service] service started\n");
        
        /* Verify whether the service is enabled */
        auto settings = loadSettings();

        if(settings.contains(SETTING_ENABLED)) {
            const auto setting = settings[SETTING_ENABLED];
            enabled_ = setting.int_value > 0;
        }
    }    

    /*! @brief Destructor for the service. */
    Service::~Service() {        
        timeout_cancelled_ = true;
    }

    namespace actions {        

        /*! @brief Terminates the currently running application. */
        void terminate_current_application(void) {
            helpers::terminateCurrentApplication();
        }
    }

    /*! @brief Sends countdown notifications every 10 seconds and terminates the app after 60 seconds.
     *
     * The countdown can be cancelled if the parent application closes or if the game is no longer running.
     * In that case the sysmodule returns to normal monitoring state.
     *
     * @param targetPid The PID of the game that triggered the timeout. */
    void Service::showScreenTimeout(u64 targetPid) {        
        logDebug("[Service] Starting 60s countdown with notifications for PID %llu\n", targetPid);

        timeout_target_pid_ = targetPid;
        constexpr int totalSeconds = 60;
        constexpr int intervalSeconds = 10;

        for(int seconds = totalSeconds; seconds >= 0; seconds -= intervalSeconds) {
            if(timeout_cancelled_.load()) {
                logInfo("[Service] Screen timeout cancelled, returning to normal state\n");
                return;
            }

            if(getRunningApplicationPid() != targetPid) {
                logInfo("[Service] Game PID %llu is no longer running, abandoning timeout\n", targetPid);
                return;
            }

            NotificationsController::notifyRemainingSeconds(seconds);

            if(seconds > 0) {
                svcSleepThread(10e9);
            }
        }

        logInfo("[Service] Time's up, terminating application\n");
        actions::terminate_current_application();
    }


    /*! @brief Retrieves the currently running application name. */
    Ipc::Result Service::getRunningApplication(Ipc::Request* request) {
        auto process_id = helpers::getRunningApplicationPid();
        if(process_id > 0) {
            auto title_id = helpers::getRunningApplicationTitleId(process_id);
            auto app_name = helpers::getApplicationName(title_id);
            request->appendReplyValue(app_name);
        } else {
            request->appendReplyValue(NullString);
        }
        
        return Ipc::Result::Ok;
    }

    /*! @brief Retrieves the Uid of the current user. */
    Ipc::Result Service::getCurrentUserUid(Ipc::Request* request) {   
        const auto current_title = helpers::getRunningApplicationPid();        
        if(current_title == 0) {
            // If there is no title we don"t query on user
            request->appendReplyValue(NullString);
            return Ipc::Result::Ok;
        } 

        const auto current_user = helpers::getCurrentUser();
        if(current_user.isValid()) {
            request->appendReplyValue(accountUidToString(current_user.uid));
        } else {
            request->appendReplyValue(NullString);
        }

        return Ipc::Result::Ok;
    }

    /*! @brief Retrieves the nickname of the current user. */
    Ipc::Result Service::getCurrentUserNickname(Ipc::Request* request) {    
        const auto current_title = helpers::getRunningApplicationPid();        
        if(current_title == 0) {
            // If there is no title we don"t query on user
            request->appendReplyValue(NullString);
            return Ipc::Result::Ok;
        } 

        const auto current_user = helpers::getCurrentUser();
        if(current_user.isValid()) {
            logDebug("[Service] replying UID=%s\n", current_user.nickname.c_str());
            request->appendReplyValue(current_user.nickname);
        } else {
            logDebug("[Service] replying UID=(NULL)\n");
            request->appendReplyValue(NullString);
        }

        return Ipc::Result::Ok;
    }

    /*! @brief Calculates and returns the remaining time for a user based on their daily limit.
     *
     * @param request IPC request containing an optional user Uid string.
     * @return Ipc::Result::Ok on success.
     */
    Ipc::Result Service::getUserRemainingTime(Ipc::Request* request) {
        char user_uid[40] = {0};
        structs::UserData user;

        Ipc::Result rc = request->readRequestValue(user_uid);
        if(rc != Ipc::Result::Ok) {
            logDebug("[Service] No user uid sent, using current user\n");

            user = helpers::getCurrentUser();
            if(!user.isValid()) {
                logDebug("[Service] There is no user\n");
                request->appendReplyValue((u16)0);
                return Ipc::Result::Ok;
            }
        } else {
            // Get the user from the uid sent
            logDebug("[Service] user Uid received: %s\n", user_uid);
            const auto& account_uid = accountUidFromString(UserUid(user_uid));
            user = getUserFromAccountUid(account_uid);

            if(!user.isValid()) {
                logDebug("[Service] Invalid user uid: %s\n", user_uid);
                request->appendReplyValue((u16)0);
                return Ipc::Result::Ok;
            }
        }

        logDebug("[Service] Get remaining time for user %s\n", user.nickname.c_str());

        const auto& date = today();
        if(date.empty()) {
            return Ipc::Result::Error;
        }

        const auto history = getHistory(user.uid, date);
        u16 usage_time_in_minutes = 0;

        // Compute the total usage for today 
        for(const auto& entry: history) {
            usage_time_in_minutes += entry.durationInMinutes();
        }

        // Get the daily limit
        auto settings = loadSettings();
        const auto& userId = accountUidToString(user.uid);
        const auto& daily_limit = getDailyLimitForUser(userId);
        s16 remaining_time_in_minutes = 0;
        if(daily_limit > usage_time_in_minutes) {
            remaining_time_in_minutes = daily_limit - usage_time_in_minutes;
        }

        logDebug("[Service] User=%s, usage=%i minutes, remaining=%i minutes\n", user.nickname.c_str(), usage_time_in_minutes, remaining_time_in_minutes);

        request->appendReplyValue(remaining_time_in_minutes);

        return Ipc::Result::Ok;
    }

    /*! @brief Retrieves the total usage time for a user today.
     *
     * @param request IPC request containing an optional user Uid string.
     * @return Ipc::Result::Ok on success.
     */
    Ipc::Result Service::getUserUsageTime(Ipc::Request* request) {
        char user_uid[40] = {0};
        structs::UserData user;

        Ipc::Result rc = request->readRequestValue(user_uid);
        if(rc != Ipc::Result::Ok) {
            logDebug("[Service] No user uid sent, using current user\n");

            user = helpers::getCurrentUser();
            if(!user.isValid()) {
                logDebug("[Service] There is no user\n");
                return Ipc::Result::Ok;
            }
        } else {
            // Get the user from the uid sent
            logDebug("[Service] user Uid received: %s\n", user_uid);
            const auto& account_uid = accountUidFromString(UserUid(user_uid));
            user = getUserFromAccountUid(account_uid);
        }

        logDebug("[Service] Get usage time for user %s\n", user.nickname.c_str());

        const auto& usage_time_in_minutes = getUserUsageTimeForToday(user.uid);

        logDebug("[Service] User=%s, usage=%i minutes\n", user.nickname.c_str(), usage_time_in_minutes);

        request->appendReplyValue(usage_time_in_minutes);

        return Ipc::Result::Ok;
    }

    /*! @brief Sets the daily usage limit for a specific user.
     *
     * @param request IPC request containing the limit in minutes and the user ID.
     * @return Ipc::Result::Ok on success.
     */
    Ipc::Result Service::setUserDailyLimit(Ipc::Request* request) {
        typedef struct __attribute__((packed)) {
            u16 limit_in_minutes;
            char userId[80];
        } Args;

        Args args{0};

        Ipc::Result rc = request->readRequestValue(args);
        if(rc != Ipc::Result::Ok) {
            logError("[Service] Could not read request value (limit)\n");
            return rc;
        }

        std::string userId = std::string(args.userId);
        logDebug("[Service] SetUserDailyLimit: userId=%s, limit=%u\n", userId.c_str(), args.limit_in_minutes);
        helpers::setDailyLimitForUser(userId, args.limit_in_minutes);
        
        return Ipc::Result::Ok;
    }

    /*! @brief Retrieves the daily usage limit for a specific user.
     *
     * @param request IPC request containing the user ID.
     * @return Ipc::Result::Ok on success.
     */
    Ipc::Result Service::getUserDailyLimit(Ipc::Request* request) {                
        char _userId[80] = {0};

        Ipc::Result rc = request->readRequestValue(_userId);
        if(rc != Ipc::Result::Ok) {
            logError("[Service] Could not read request data\n");
            return rc;
        }

        std::string userId = std::string(_userId);
        const auto& limit = helpers::getDailyLimitForUser(userId);

        request->appendReplyValue(limit);
        
        return Ipc::Result::Ok;
    }

    /*! @brief Sets the admin PIN for unlocking parental control settings.
     *
     * @param request IPC request containing 4 u64 values representing the PIN digits.
     * @return Ipc::Result::Ok on success.
     */
    Ipc::Result Service::setAdminPin(Ipc::Request* request) {
        //std::string pin;
        //Ipc::Result rc = request->readRequestData(pin);
        u64 pin[4] = {0};
        
        u64 val = 0;
        Ipc::Result rc = Ipc::Result::Ok;
        for(int i = 0 ; i < 4 ; i++) {
             rc = request->readRequestValue(val);
             if(rc == Ipc::Result::Ok) {
                pin[i] = val;
             } else {
                break;
             }
        }

        if(rc != Ipc::Result::Ok) {
            logDebug("[Service] Could not read request data (PIN)\n");
            return rc;
        }

        std::string s_pin = std::to_string(pin[0]) +"," +std::to_string(pin[1]) +"," +std::to_string(pin[2]) +"," +std::to_string(pin[3]);

        logDebug("[Service] Setting admin PIN to %s\n", s_pin.c_str());
        auto settings = loadSettings();
        
        Setting settingPin {
            .key = SETTING_ADMIN_PIN,
            .type = STRING,
            .string_value = s_pin,
            .encrypted = true
        };

        saveSetting(settingPin);

        return Ipc::Result::Ok;        
    }

    /*! @brief Verifies the provided admin PIN against the stored PIN.
     *
     * @param request IPC request containing 4 u64 values representing the PIN digits.
     * @return Ipc::Result::Ok on success, BadInput if data could not be read.
     */
    Ipc::Result Service::verifyAdminPin(Ipc::Request* request) {
        //std::string pin;
        u64 pin[4] = {0};
        
        u64 val = 0;
        Ipc::Result rc = Ipc::Result::Ok;
        for(int i = 0 ; i < 4 ; i++) {
             rc = request->readRequestValue(val);
             if(rc == Ipc::Result::Ok) {
                pin[i] = val;
             } else {
                break;
             }
        }

        if(rc != Ipc::Result::Ok) {
            logError("[Service] Could not read request data (PIN)\n");
            return Ipc::Result::BadInput;
        }

        std::string s_pin = std::to_string(pin[0]) +"," +std::to_string(pin[1]) +"," +std::to_string(pin[2]) +"," +std::to_string(pin[3]);

        auto settings = loadSettings();

        auto adminPin = settings[SETTING_ADMIN_PIN].string_value;
        if(adminPin.empty()) {
            adminPin = std::to_string(DefaultPin[0]) +"," +std::to_string(DefaultPin[1]) +"," +std::to_string(DefaultPin[2]) +"," +std::to_string(DefaultPin[3]);
            logDebug("[Service] No PIN defined. Using default.\n");
        }
        logDebug("[Service] verify admin PIN. Recv=%s. ref=%s\n", s_pin.c_str(), adminPin.c_str());
        request->appendReplyValue(adminPin == s_pin ? true : false);

        return Ipc::Result::Ok;
    }


    /*! @brief Sets the working mode (Info or Blocking) for parental control.
     *
     * @param request IPC request containing the working mode value.
     * @return Ipc::Result::Ok on success, BadInput if data could not be read.
     */
    Ipc::Result Service::setWorkingMode(Ipc::Request* request) {
        WorkingMode workingMode = WorkingModeInfo;
        Ipc::Result rc = request->readRequestValue(workingMode);
        if(rc != Ipc::Result::Ok) {
            logError("[Service] Could not read request data (working mode)\n");
            return Ipc::Result::BadInput;
        }

        auto settings = loadSettings();
        auto setting = Setting{
            .key = SETTING_WORKING_MODE,
            .type = INTEGER,
            .int_value = (u64)workingMode
        };

        saveSetting(setting);

        return Ipc::Result::Ok;
    }

    /*! @brief Retrieves the current working mode (Info or Blocking).
     *
     * @param request IPC request.
     * @return Ipc::Result::Ok on success.
     */
    Ipc::Result Service::getWorkingMode(Ipc::Request* request) {
        auto settings = loadSettings();

        if(!settings.contains(SETTING_WORKING_MODE)) {
            logDebug("[Service] The setting %s is not defined.\n", SETTING_WORKING_MODE);
            request->appendReplyValue((u8)WorkingModeBlocking);
        } else {
            request->appendReplyValue((u8)settings[SETTING_WORKING_MODE].int_value);
        }

        return Ipc::Result::Ok;
    }

    /*! @brief Enables or disables the display of remaining time in the UI.
     *
     * @param request IPC request containing a boolean flag.
     * @return Ipc::Result::Ok on success, BadInput if data could not be read.
     */
    Ipc::Result Service::setShowRemainingTime(Ipc::Request* request) {        
        bool showRemainingTime = false;
        Ipc::Result rc = request->readRequestValue(showRemainingTime);
        logDebug("[Service] Show remaining time panel ? %i\n", showRemainingTime);        
        if(rc != Ipc::Result::Ok) {
            logError("[Service] Could not read data (set show remaining time)\n");
            return Ipc::Result::BadInput;
        }
        
        auto settings = loadSettings();
        auto setting = Setting{
            .key = SETTING_SHOW_REMAINING_TIME,
            .type = INTEGER,
            .int_value = showRemainingTime ? (u64)1 : (u64)0
        };

        saveSetting(setting);

        // DISABLED temporarily
        /*if(showRemainingTime) {
            gui_.showRemainingTimePanel();
        } else {
            gui_.hideRemainingTimePanel();
        }*/

        logDebug("[Service] Ok\n");
        return Ipc::Result::Ok;
    }

    /*! @brief Retrieves whether the remaining time display is enabled.
     *
     * @param request IPC request.
     * @return Ipc::Result::Ok on success.
     */
    Ipc::Result Service::getShowRemainingTime(Ipc::Request* request) {
        auto settings = loadSettings();

        if(!settings.contains(SETTING_SHOW_REMAINING_TIME)) {
            logDebug("[Service] The setting %s is not defined.\n", SETTING_SHOW_REMAINING_TIME);
            request->appendReplyValue(0);
        } else {
            request->appendReplyValue(settings[SETTING_SHOW_REMAINING_TIME].int_value);
        }

        return Ipc::Result::Ok;
    }

    /*! @brief Retrieves the current enabled state of the service.
     *
     * @param request IPC request.
     * @return Ipc::Result::Ok on success.
     */
    Ipc::Result Service::isEnabled(Ipc::Request* request) {
        logDebug("[Service] getting current service state: %i\n", enabled_);

        request->appendReplyValue(enabled_ ? (u8)1 : (u8)0);
        return Ipc::Result::Ok;
    }

    /*! @brief Enables or disables the parental control service.
     *
     * @param request IPC request containing a boolean flag.
     * @return Ipc::Result::Ok on success, BadInput if data could not be read.
     */
    Ipc::Result Service::setEnabled(Ipc::Request* request) {        
        bool isEnabled = false;        
        Ipc::Result rc = request->readRequestValue(isEnabled);
        logDebug("[Service] setting service state to %s\n", (isEnabled ? "enabled" : "disabled"));

        if(rc != Ipc::Result::Ok) {
            logError("[Service] Could not read data (enabled)\n");
            return Ipc::Result::BadInput;
        }

        auto settings = loadSettings();
        auto setting = Setting{
            .key = SETTING_ENABLED,
            .type = INTEGER,
            .int_value = isEnabled ? (u64)1 : (u64)0
        };

        saveSetting(setting);
        enabled_ = isEnabled;

        if(enabled_) {
            monitor_->start();
        } else {
            monitor_->stop();
        }

        return Ipc::Result::Ok;
    }    

    /*! @brief Retrieves the current application version string.
     *
     * @param request IPC request.
     * @return Ipc::Result::Ok on success.
     */
    Ipc::Result Service::getCurrentVersion(Ipc::Request* request) {
        logDebug("[Service] Getting current version: %s\n", VERSION);

        std::string _ver = VERSION;
        request->appendReplyValue(_ver);
        return Ipc::Result::Ok;
    }    

    /*! @brief Sets the log level for both sysmodule and overlay logging.
     *
     * @param request IPC request containing the log level value.
     * @return Ipc::Result::Ok on success, BadInput if data could not be read.
     */
    Ipc::Result Service::setLogLevel(Ipc::Request* request) {
        logDebug("[Service] Setting log level\n");

        u8 level = 0;
        Ipc::Result rc = request->readRequestValue(level);
        if(rc != Ipc::Result::Ok) {
            logError("[Service] Could not read the log level value\n");
            return Ipc::Result::BadInput;
        }

        auto settings = loadSettings();
        auto setting = Setting{
            .key = SETTING_LOGLEVEL,
            .type = INTEGER,
            .int_value = level
        };

        saveSetting(setting);
        
        logger::setLogLevel(static_cast<LogLevel>(level));
        //logInfo("[Service] Log level set to %i\n", level);        

        return Ipc::Result::Ok;
    }

    /*! @brief Retrieves the current log level.
     *
     * @param request IPC request.
     * @return Ipc::Result::Ok on success.
     */
    Ipc::Result Service::getLogLevel(Ipc::Request* request) {
        logDebug("[Service] Getting current log level\n");

        u16 logLevel = currentLogLevel();
        request->appendReplyValue(logLevel);
        
        return Ipc::Result::Ok;
    } 

    /*! @brief Checks whether the history database has been tampered with.
     *
     * @param request IPC request.
     * @return Ipc::Result::Ok on success.
     */
    Ipc::Result Service::isDatabaseTampered(Ipc::Request* request) {
        logDebug("[Service] Verify whether database has been tampered\n");

        request->appendReplyValue(static_cast<u8>(isTampered()));

        return Ipc::Result::Ok;
    }

    /*! @brief Checks whether the history database needs to be upgraded.
     *
     * @param request IPC request.
     * @return Ipc::Result::Ok on success.
     */
    Ipc::Result Service::isMustUpgradeDatabase(Ipc::Request* request) {
        logDebug("[Service] Verify whether database needs to be upgraded\n");

        request->appendReplyValue(static_cast<u8>(upgradeNeeded()));

        return Ipc::Result::Ok;
    }

    /*! @brief Delayed background task that triggers the timeout screen after 1 second.
     *
     * Used as a thread argument to defer the timeout screen display.
     *
     * @param arg Pointer to the Service instance.
     */
    void delayedTimeout(void* arg) {
        logDebug("[Service] Delayed task\n");
        Service* svc = static_cast<Service*>(arg);
        svcSleepThread(1'000'000'000LL);
        u64 currentPid = getRunningApplicationPid();
        svc->showScreenTimeout(currentPid);
    }

    /*! @brief Main IPC request dispatcher that routes commands to handler methods.
     *
     * @param request Pointer to the IPC request object.
     * @return Ipc::Result indicating success or failure.
     */
    Ipc::Result Service::commandThread(Ipc::Request* request) {
        const auto cmd = static_cast<Ipc::Command>(request->cmd());
        logDebug("[Service] request received: %s\n", Ipc::commandToString(cmd).c_str());

        switch (cmd) {   
            case Ipc::Command::Version: {
                return getCurrentVersion(request);
            }
            case Ipc::Command::Test: {
                /*logToFile("[Service] Schedule timeout screen in 5 seconds...\n");
                Thread t;
                threadCreate(&t, delayedTimeout, this, NULL, 0x4000, 0x2c, -2);
                threadStart(&t);*/                
                //NotificationsController::notifyRemainingTime(15);
                //helpers::terminateCurrentApplication();
                const auto& pid = helpers::getRunningApplicationPid();
                if(pid == 0) {
                    return Ipc::Result::Ok;
                }
                showScreenTimeout(pid);
                break;
            }
            case Ipc::Command::GetCurrentUserUid: {
                return getCurrentUserUid(request);
            }
            case Ipc::Command::GetCurrentUserNickname: {
                return getCurrentUserNickname(request);
            }                   
            case Ipc::Command::GetUserUsageTime: {                                
                return getUserUsageTime(request);                
            }
            case Ipc::Command::GetUserRemainingTime: {                
                return getUserRemainingTime(request);   
            }
            case Ipc::Command::GetRunningApplication: {
                return getRunningApplication(request);
            }
            case Ipc::Command::SetUserDailyLimit: {
                return setUserDailyLimit(request);
            }            
            case Ipc::Command::GetUserDailyLimit: {
                return getUserDailyLimit(request);
            }
            case Ipc::Command::SetAdminPin: {                
                return setAdminPin(request);
            }
            case Ipc::Command::VerifyAdminPin: {                
                return verifyAdminPin(request);
            }
            case Ipc::Command::SetWorkingMode: {
                return setWorkingMode(request);
            }
            case Ipc::Command::GetWorkingMode: {
                return getWorkingMode(request);
            }
            case Ipc::Command::SetShowRemainingTime: {
                return setShowRemainingTime(request);
            }
            case Ipc::Command::GetShowRemainingTime: {
                return getShowRemainingTime(request);
            }
            case Ipc::Command::IsEnabled: {
                return isEnabled(request);
            }
            case Ipc::Command::SetEnabled: {
                return setEnabled(request);
            }
            case Ipc::Command::SetLogLevel: {
                return setLogLevel(request);
            }
            case Ipc::Command::GetLogLevel: {
                return getLogLevel(request);
            }
            case Ipc::Command::IsTampered: {
                return isDatabaseTampered(request);
            } 
            case Ipc::Command::MustUpgradeDatabase: {
                return isMustUpgradeDatabase(request);
            }
            case Ipc::Command::CancelScreenTimeout: {
                return cancelScreenTimeout(request);
            }
            default: {
                logError("[Service] command %i not supported.\n", request->cmd());                
                return Ipc::Result::BadInput;
            }
        }        

        return Ipc::Result::Ok;
    }

    /*! @brief Cancels the active screen timeout countdown. */
    Ipc::Result Service::cancelScreenTimeout(Ipc::Request* request) {
        logInfo("[Service] Screen timeout cancelled by user\n");
        timeout_cancelled_ = true;
        return Ipc::Result::Ok;
    }

    /*! @brief Starts the IPC server listener thread. */
    void Service::listen() {                
        logInfo("[Service] Starting listening\n");

        while(true) {            
            if (!ipcServer_->process()) {
                // When an error occurs we exit
                return;
            }

            svcSleepThread(5'000);
        }

        logInfo("[Service] Stopped listening\n");
    }

}