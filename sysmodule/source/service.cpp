/*
 * Copyright (c) Atmosphère-NX
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms and conditions of the GNU General Public License,
 * version 2, as published by the Free Software Foundation.
 *
 * This program is distributed in the hope it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include <switch.h>
#include <thread>
#include <cstring>
#include <map>
#include <list>
#include <sstream>
#include "service.hpp"
#include "logger.h"
#include "ipc/Command.hpp"
#include "ipc/Result.hpp"
#include "helpers.h"
#include "database/settings.h"
#include "database/database.h"
#include "monitor.h"
#include "gui/gui_controller.h"
#include "notifications_controller.h"
//#define PSEC_DEBUG 1

using namespace alefbet::pctrl::logger;
using namespace alefbet::pctrl::database;
using namespace alefbet::pctrl::helpers;

constexpr std::string NullString = std::string("(NULL)");
constexpr u64 DefaultPin[] = { HidNpadButton_A, HidNpadButton_A, HidNpadButton_A, HidNpadButton_A };

namespace alefbet::pctrl::srv {       

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

    Service::~Service() {        
    }

    namespace actions {        

        void reboot_to_payload(void) {
            helpers::rebootToPayload();
        }
    }

    void Service::showScreenTimeout() {        
        logDebug("[Service] Requested to show timeout screen\n");                        

        gui_.showScreenTimeout();

        // Wait for Vol+
        // Block unless a button has been pressed
        GpioPadSession g_volup;
        gpioInitialize();
        gpioOpenSession(&g_volup, GpioPadName_ButtonVolUp);
        GpioValue value;

        while(true) {            
            gpioPadGetValue(&g_volup, &value);

            if(value == 0) {
                logDebug("[GUI] Vol+ pressed.\n");
                actions::reboot_to_payload();
                break;
            }  
            
            // Wait a little
            svcSleepThread(50e6); // 50 ms
        }

        gpioExit();
    }

    void Service::showScreenTitleBlacklisted() {
        logDebug("[Service] Requested to show title blacklisted screen\n");

        gui_.showScreenTitleBlacklisted();

        // Terminate the game
        helpers::terminateCurrentApplication();

        // Wait for any button
        hidInitialize();
        hidsysInitialize();

        // Allow only Player 1 and handheld mode
        HidNpadIdType id_list[2] = { HidNpadIdType_No1, HidNpadIdType_Handheld };
        
        // Configure HID system to only listen to these IDs
        hidSetSupportedNpadIdType(id_list, 2);
        
        // Configure input for up to 2 supported controllers (P1 + Handheld)
        padConfigureInput(2, HidNpadStyleSet_NpadStandard | HidNpadStyleTag_NpadSystemExt);
        
        PadState pad_p1_;
        PadState pad_handheld_;

        // Initialize separate pad states for both controllers    
        padInitialize(&pad_p1_, HidNpadIdType_No1);
        padInitialize(&pad_handheld_, HidNpadIdType_Handheld);
                
        // Clear any stale input from both controllers
        padUpdate(&pad_p1_);
        padUpdate(&pad_handheld_);

        while(true) {
            padUpdate(&pad_p1_);
            padUpdate(&pad_handheld_);

            const u64 kDown_p1 = padGetButtonsDown(&pad_p1_);
            const u64 kDown_handheld = padGetButtonsDown(&pad_handheld_);

            u64 keysDown = kDown_p1 | kDown_handheld;

            if(keysDown != 0) {
                logDebug("[Service] The user pressed a button. Exiting screen\n");

                gui_.hideAll();

                return;
            }

        }
        
    }

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

    Ipc::Result Service::getUsersList(Ipc::Request* request) {
        logError("[Service] WARNING! GetUsersList command is deprecated\n");
        return Ipc::Result::Ok;
    }

    Ipc::Result Service::getUserRemainingTime(Ipc::Request* request) {
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

        const auto& date = today();
        if(date.empty()) {
            return Ipc::Result::Error;
        }

        const auto history = getHistory(user.uid, date);
        auto usage_time_in_minutes = (u16)0;        

        // Compute the total usage for today 
        for(const auto& entry: history) {
            usage_time_in_minutes += entry.durationInMinutes();
        }

        // Get the daily limit
        auto settings = loadSettings();
        const auto daily_limit = settings[SETTING_DAILY_LIMIT_GLOBAL].int_value;
        s16 remaining_time_in_minutes = 0;
        if(daily_limit > usage_time_in_minutes) {
            remaining_time_in_minutes = daily_limit - usage_time_in_minutes;
        }

        logDebug("[Service] User=%s, usage=%i minutes, remaining=%i minutes\n", user.nickname.c_str(), usage_time_in_minutes, remaining_time_in_minutes);

        request->appendReplyValue(remaining_time_in_minutes);

        return Ipc::Result::Ok;
    }

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

        const auto history = getHistory(user.uid, today());
        auto usage_time_in_minutes = (u16)0;        

        // Compute the total usage for today 
        for(const auto& entry: history) {
            usage_time_in_minutes += entry.durationInMinutes();
        }

        logDebug("[Service] User=%s, usage=%i minutes\n", user.nickname.c_str(), usage_time_in_minutes);

        request->appendReplyValue(usage_time_in_minutes);

        return Ipc::Result::Ok;
    }

    Ipc::Result Service::setUserLimits(Ipc::Request* request) {
        // Now the limit is global not per user so we don't use the first argument
        u16 limit_in_minutes;
        Ipc::Result rc = request->readRequestData(limit_in_minutes);
        if(rc != Ipc::Result::Ok) {
            logError("[Service] Could not read request data (limit)\n");
            return rc;
        }

        auto settings = loadSettings();

        Setting setting {        
            .key = SETTING_DAILY_LIMIT_GLOBAL,
            .type = INTEGER,
            .int_value = limit_in_minutes
        };

        saveSetting(setting);

        return Ipc::Result::Ok;
    }

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

    Ipc::Result Service::isEnabled(Ipc::Request* request) {
        logDebug("[Service] getting current service state: %i\n", enabled_);

        request->appendReplyValue(enabled_ ? (u8)1 : (u8)0);
        return Ipc::Result::Ok;
    }

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

    Ipc::Result Service::getDailyLimit(Ipc::Request* request) {
        logDebug("[Service] Getting the daily limit\n");

        auto settings = loadSettings();        
        u16 limit_in_minutes = 0;
        
        if(settings.contains(SETTING_DAILY_LIMIT_GLOBAL)) {
            limit_in_minutes = settings[SETTING_DAILY_LIMIT_GLOBAL].int_value;
        }

        request->appendReplyValue(limit_in_minutes);

        return Ipc::Result::Ok;
    }

    Ipc::Result Service::setDailyLimit(Ipc::Request* request) {
        logDebug("[Service] Setting the daily limit\n");

        u16 limit = 0;
        Ipc::Result rc = request->readRequestValue(limit);
        if(rc != Ipc::Result::Ok) {
            logError("[Service] Could not read the daily limit\n");
            return Ipc::Result::BadInput;
        }        

        auto settings = loadSettings();
        auto setting = Setting{
            .key = SETTING_DAILY_LIMIT_GLOBAL,
            .type = INTEGER,
            .int_value = limit
        };

        saveSetting(setting);
        logDebug("[Service] Daily limit is set to %i minutes\n", limit);

        return Ipc::Result::Ok;
    }

    Ipc::Result Service::getCurrentVersion(Ipc::Request* request) {
        logDebug("[Service] Getting current version: %s\n", VERSION);

        std::string _ver = VERSION;
        request->appendReplyValue(_ver);
        return Ipc::Result::Ok;
    }    

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

    Ipc::Result Service::getLogLevel(Ipc::Request* request) {
        logDebug("[Service] Getting current log level\n");

        u16 logLevel = currentLogLevel();
        request->appendReplyValue(logLevel);
        
        return Ipc::Result::Ok;
    } 

    Ipc::Result Service::isDatabaseTampered(Ipc::Request* request) {
        logDebug("[Service] Verify whether database has been tampered\n");

        request->appendReplyValue(static_cast<u8>(isTampered()));

        return Ipc::Result::Ok;
    }

    Ipc::Result Service::isMustUpgradeDatabase(Ipc::Request* request) {
        logDebug("[Service] Verify whether database needs to be upgraded\n");

        request->appendReplyValue(static_cast<u8>(upgradeNeeded()));

        return Ipc::Result::Ok;
    }

    void delayedTimeout(void* arg) {
        logDebug("[Service] Delayed task\n");
        Service* svc = static_cast<Service*>(arg);
        svcSleepThread(1'000'000'000LL);
        svc->showScreenTimeout();
    }

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
                helpers::terminateCurrentApplication();

                break;
            }
            case Ipc::Command::GetCurrentUserUid: {
                return getCurrentUserUid(request);
            }
            case Ipc::Command::GetCurrentUserNickname: {
                return getCurrentUserNickname(request);
            }
            case Ipc::Command::GetUsersList: {
                return getUsersList(request);
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
            case Ipc::Command::SetUserLimits: {                           
                return setUserLimits(request);
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
            case Ipc::Command::GetDailyLimit: {
                return getDailyLimit(request);
            }
            case Ipc::Command::SetDailyLimit: {
                return setDailyLimit(request);
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
            default: {
                logError("[Service] command %i not handled.\n", request->cmd());                
                return Ipc::Result::UnknownCommand;
            }
        }        

        return Ipc::Result::Ok;
    }
    
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