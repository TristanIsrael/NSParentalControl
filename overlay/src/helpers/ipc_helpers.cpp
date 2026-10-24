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

#include "helpers/ipc_helpers.h"
#include "logger.h"
#include "AppContext.h"
#include "Command.hpp"
#include <string>
#include <cstring>

namespace alefbet::pctrl::ipc {
    
    using namespace std::chrono;
    using namespace alefbet::pctrl::logger;

    constexpr u8 PIN_LEN_MAX = 20*4+3;
    constexpr u8 ACC_USER_NICKNAME_MAX = 0x20;

    /*! @brief Checks if the pctrl service is available.
     * @return true if the service is available, false otherwise. */
    bool isAvailable() {
        return getAppContext().is_available;
    }

    /*! @brief Sends a test request to the sysmodule. */
    void startTest() {
        logInfo("[IPC] starting test\n");

        if(!isAvailable()) {
            logError("[IPC] service not available\n");
            return;
        }
            
        auto& service = getAppContext().pctrl_service;
        Result res = serviceDispatch(&service, (u32)Ipc::Command::Test); 
        if(R_FAILED(res)) {
            logError("[IPC] serviceDispatch failed (%i.%i)\n", R_MODULE(res), R_DESCRIPTION(res));
            return;
        }

        logDebug("[IPC] service dispatch OK\n");
    }
    
    /*! @brief Gets the current user's account UID via IPC.
     * @return The user's UID as a string, or empty on failure. */
    UserUid getCurrentUserUid() {
        logDebug("[IPC] Get current user Uid\n");

        if(!isAvailable()) {
            logError("[IPC] service not available\n");
            return "";
        }

        auto& service = getAppContext().pctrl_service;
        char currentUser[50] = {0};
        Result res = serviceDispatchOut(&service, (u32)Ipc::Command::GetCurrentUserUid, currentUser);
        if(R_FAILED(res)) {
            return UserUid("Err#102");
        }

        UserUid user_id = UserUid(currentUser);
        logDebug("Recv uid = %s\n", user_id.c_str());
        if(user_id == "(NULL)") {
            return "";
        }

        return user_id;
    }

    /*! @brief Gets the current user's nickname via IPC.
     * @return The user's nickname as a string, or empty on failure. */
    UserNickname getCurrentUserNickname() {
        logDebug("[IPC] Get current user Nickname\n");

        if(!isAvailable()) {
            logError("[IPC] service not available\n");
            return "";
        }

        auto& service = getAppContext().pctrl_service;
        char currentUser[21] = {0};
        Result res = serviceDispatchOut(&service, (u32)Ipc::Command::GetCurrentUserNickname, currentUser);
        if(R_FAILED(res)) {
            return UserNickname("Err#102");
        }

        UserNickname nickname = currentUser;
        if(nickname == "(NULL)") {
            return "";
        }

        return nickname;
    }

    /*! @brief Gets the name of the currently running application via IPC.
     * @return The title name as a string, or empty on failure. */
    std::string getCurrentTitle() {
        logDebug("[IPC] Get current title\n");

        /*logToFile("Test"); 
        std::string _name = helpers::getTitleName(418111488);
        logToFile(_name.c_str());*/

        if(!isAvailable()) {
            logError("[IPC] service not available\n");
            return "";
        }

        auto& service = getAppContext().pctrl_service;
        char cAppName[512] = {0};
        Result res = serviceDispatchOut(&service, (u32)Ipc::Command::GetRunningApplication, cAppName);
        if(R_FAILED(res)) {
            return std::string("Err#103");
        }

        std::string title = cAppName;
        if(title == "(NULL)") {
            return "";
        }

        return title;
    }

    /*! @brief Gets the total usage time for a user via IPC.
     * @param accountUid The account UID to query (empty for current user).
     * @return The usage time as a chrono minutes object. */
    minutes getUserUsageTime(const AccountUid& accountUid) {
        logDebug("[IPC] Get usage time for the user\n");
                
        if(!isAvailable()) {
            logError("[IPC] service not available\n");
            return minutes(0);
        }

        auto& service = getAppContext().pctrl_service;
        u16 usageTime = 0;
        Result res = 0;
        UserUid uid = accountUidToString(accountUid);

        if(!accountUidIsValid(&accountUid)) {
            logDebug("Query usage time for current user\n");
            res = serviceDispatchOut(&service, (u32)Ipc::Command::GetUserUsageTime, usageTime);
        } else {
            
            char _uid[40] = {0};
            std::memcpy(_uid, uid.data(), uid.length());
            logDebug("Query usage time for uid %i\n", _uid);
            res = serviceDispatchInOut(&service, (u32)Ipc::Command::GetUserUsageTime, _uid, usageTime);
        }

        if(R_FAILED(res)) {
            logError("[IPC] An error occured while queriying the usage time for the user\n");
        }

        return std::chrono::minutes(usageTime);
    }

    /*! @brief Gets the remaining time for a user via IPC.
     * @param accountUid The account UID to query (empty for current user).
     * @return The remaining time as a chrono minutes object. */
    minutes getUserRemainingTime(const AccountUid& accountUid) {
        logDebug("[IPC] Get remaining time for the user\n");
        
        if(!isAvailable()) {
            logError("[IPC] service not available\n");
            return minutes(0);
        }

        auto& service = getAppContext().pctrl_service;
        u16 remainingTime = 0;
        UserUid uid = accountUidToString(accountUid);
        Result res = 0;
        
        if(!accountUidIsValid(&accountUid)) {
            logDebug("Query usage time for current user\n");
            res = serviceDispatchOut(&service, (u32)Ipc::Command::GetUserRemainingTime, remainingTime);
        } else {
            
            char _uid[40] = {0};
            std::memcpy(_uid, uid.data(), uid.length());
            logDebug("Query usage time for uid %s\n", _uid);
            res = serviceDispatchInOut(&service, (u32)Ipc::Command::GetUserRemainingTime, _uid, remainingTime);
        }

        if(R_FAILED(res)) {
            logError("[IPC] An error occured while queriying the remaining time for the user\n");
        }

        return std::chrono::minutes(remainingTime);
    }

    /*! @brief Encodes a PIN (array of 4 u64 values) into a comma-separated string.
     * @param keys Array of 4 u64 values representing the PIN.
     * @return Comma-separated PIN string, or empty on failure. */
    std::string encodeAdminPin(const std::vector<u64>& keys) {
        std::string pin;

        if(!isAvailable()) {
            logError("[IPC] service not available\n");
            return "";
        }

        if(keys.size() != 4) {
            logError("[IPC] The PIN size is wrong.\n");
            return pin;
        }

        pin = std::to_string(keys[0]) +"," +std::to_string(keys[1]) +"," +std::to_string(keys[2]) +"," +std::to_string(keys[3]);
        logDebug("[IPC] Encoded PIN is %s", pin.c_str());

        return pin;
    }

    /*! @brief Verifies the admin PIN via IPC.
     * @param pin Array of 4 u64 values representing the PIN.
     * @return true if verified, false otherwise. */
    bool verifyPin(const std::vector<u64>& pin) {
        logDebug("[IPC] Verify Admin PIN\n");

        if(pin.size() < 4) {
            logError("[IPC] The pin size is incorrect\n");
            return false;
        }

        if(!isAvailable()) {
            logError("[IPC] service not available\n");
            return false;
        }

        auto& service = getAppContext().pctrl_service;
        bool verified = false;
        u64 a_pin[4] = { 
            pin[0], pin[1], pin[2], pin[3]
        };        

        logDebug("PIN = %s\n", encodeAdminPin(pin).c_str());
        Result res = serviceDispatchInOut(&service, (u32)Ipc::Command::VerifyAdminPin, a_pin, verified);

        if(R_FAILED(res)) {
            logError("[IPC] An error occured during the Admin PIN verification.\n");
            return false;
        }

        return verified;
    }

    /*! @brief Sets a new admin PIN via IPC.
     * @param pin Array of 4 u64 values representing the new PIN.
     * @return true if successful, false otherwise. */
    bool setupPin(const std::vector<u64>& pin) {
        logDebug("[IPC] Setup new PIN\n");

        if(pin.size() < 4) {
            logError("[IPC] The pin size is incorrect\n");
            return false;
        }

        if(!isAvailable()) {
            logError("[IPC] service not available\n");
            return false;
        }

        auto& service = getAppContext().pctrl_service;
        
        u64 a_pin[4] = { 
            pin[0], pin[1], pin[2], pin[3]
        }; 
        logDebug("PIN = %s\n", encodeAdminPin(pin).c_str());
        Result res = serviceDispatchIn(&service, (u32)Ipc::Command::SetAdminPin, a_pin);

        if(R_FAILED(res)) {
            logError("[IPC] An error occured during the setup of the Admin PIN.\n");
            return false;
        }

        return true;
    }

    /*! @brief Sets the working mode via IPC.
     * @param mode The working mode to set.
     * @return true if successful, false otherwise. */
    bool setWorkingMode(const WorkingMode& mode) {
        logDebug("[IPC] Setting working mode\n");

        if(!isAvailable()) {
            logError("[IPC] service not available\n");
            return false;
        }

        auto& service = getAppContext().pctrl_service;
        int _mode = (u8)mode;
        Result res = serviceDispatchIn(&service, (u32)Ipc::Command::SetWorkingMode, _mode);

        if(R_FAILED(res)) {
            logError("[IPC] An error occured while setting up the working mode.\n");
            return false;
        }

        return true;
    }

    /*! @brief Gets the current working mode via IPC.
     * @return The current working mode. */
    WorkingMode getWorkingMode() {
        logDebug("[IPC] Getting working mode\n");

        if(!isAvailable()) {
            logError("[IPC] service not available\n");
            return WorkingModeBlocking;
        }

        u8 workingMode = WorkingModeInfo;

        auto& service = getAppContext().pctrl_service;
        Result res = serviceDispatchOut(&service, (u32)Ipc::Command::GetWorkingMode, workingMode);

        if(R_FAILED(res)) {
            logError("[IPC] An error occured while getting the working mode.\n");
        }

        switch(workingMode) {
            case 0: return WorkingModeInfo;
            case 1: return WorkingModeBlocking;
        }

        return WorkingModeBlocking;
    }
    
    /*! @brief Sets the remaining time visibility setting via IPC.
     * @param active Whether to show remaining time.
     * @return true if successful, false otherwise. */
    bool setShowRemainingTime(const bool& active) {
        logDebug("[IPC] Setting remaining time visibility: %s\n", active ? "true" : "false");

        if(!isAvailable()) {
            logError("[IPC] service not available\n");
            return false;
        }

        auto& service = getAppContext().pctrl_service;
        Result res = serviceDispatchIn(&service, (u32)Ipc::Command::SetShowRemainingTime, active);

        if(R_FAILED(res)) {
            logError("[IPC] An error occured while setting up remaining time visibility.\n");
            return false;
        }

        return true;
    }

    /*! @brief Gets the remaining time visibility setting via IPC.
     * @return true if remaining time is visible, false otherwise. */
    bool getShowRemainingTime() {
        logDebug("[IPC] Getting remaining time visibility\n");

        if(!isAvailable()) {
            logError("[IPC] service not available\n");
            return false;
        }

        u8 visible = 0;

        auto& service = getAppContext().pctrl_service;
        Result res = serviceDispatchOut(&service, (u32)Ipc::Command::GetShowRemainingTime, visible);

        if(R_FAILED(res)) {
            logError("[IPC] An error occured while getting the remaining time visibility.\n");
        }

        return visible == 1 ? true : false;
    }

    /*! @brief Enables or disables the service via IPC.
     * @param enabled Whether to enable the service.
     * @return true if successful, false otherwise. */
    bool setEnabled(const bool& enabled) {
        if(enabled)
            logDebug("[IPC] Setting service enabled\n");
        else 
            logDebug("[IPC] Setting service disabled\n");

        if(!isAvailable()) {
            logError("[IPC] service not available\n");
            return false;
        }

        auto& service = getAppContext().pctrl_service;
        Result res = serviceDispatchIn(&service, (u32)Ipc::Command::SetEnabled, enabled);

        if(R_FAILED(res)) {
            logError("[IPC] An error occured while change the state of the service.\n");
            return false;
        }

        return true;
    }

    /*! @brief Gets the current enabled state of the service via IPC.
     * @return true if the service is enabled, false otherwise. */
    bool isEnabled() {
        logDebug("[IPC] Getting current state of the service\n");

        if(!isAvailable()) {
            logError("[IPC] service not available\n");
            return false;
        }

        u8 enabled = 0;

        auto& service = getAppContext().pctrl_service;
        Result res = serviceDispatchOut(&service, (u32)Ipc::Command::IsEnabled, enabled);

        if(R_FAILED(res)) {
            logError("[IPC] An error occured while getting the current service state.\n");
        } else {
            logDebug("[IPC] Service state is %s\n", enabled ? "enabled" : "disabled");
        }

        return enabled == 1 ? true : false;
    }
 
    /*! @brief Gets the sysmodule version via IPC.
     * @return The version string, or empty on failure. */
    std::string getVersion() {
        logDebug("[IPC] Getting sysmodule version\n");

        if(!isAvailable()) {
            logError("[IPC] service not available\n");
            return "";
        }

        char version[10] = {0};

        auto& service = getAppContext().pctrl_service;
        Result res = serviceDispatchOut(&service, (u32)Ipc::Command::Version, version);

        if(R_FAILED(res)) {
            logError("[IPC] An error occured while getting the sysmodule version.\n");
        } 

        logDebug("[IPC] Sysmodule version is %s\n", version);

        return std::string(version);
    }

    /*! @brief Gets the daily time limit for a user via IPC.
     * @param user The user data containing the UID.
     * @return The daily limit in minutes, or 0 on failure. */
    u16 getDailyLimit(const UserData& user) {
        logDebug("[IPC] Getting daily limit\n");

        if(!isAvailable()) {
            logError("[IPC] service not available\n");
            return 0;
        }

        u16 limit = 0;        
        char chUserId[80] = {0};

        const auto& userId = helpers::accountUidToString(user.uid);
        std::snprintf(chUserId, sizeof(chUserId), "%s", userId.c_str());

        auto& service = getAppContext().pctrl_service;
        Result res = serviceDispatchInOut(&service, (u32)Ipc::Command::GetUserDailyLimit, chUserId, limit);

        if(R_FAILED(res)) {
            logError("[IPC] An error occured while getting the daily limit.\n");
        } 

        logDebug("[IPC] Daily limit is %i\n", limit);

        return limit;
    }

    /*! @brief Sets the daily time limit for a user via IPC.
     * @param user The user data containing the UID.
     * @param limit The daily limit in minutes.
     * @return true if successful, false otherwise. */
    bool setDailyLimit(const UserData& user, u16 limit) {
        logDebug("[IPC] Setting the daily limit to %i\n", limit);

        if(!isAvailable()) {
            logError("[IPC] service not available\n");
            return false;
        }

        typedef struct __attribute__((packed)) {
            u16 limit_in_minutes;
            char userId[80];
        } Args;

        Args args {
            .limit_in_minutes = limit,            
        };

        const auto& userId = helpers::accountUidToString(user.uid);
        std::snprintf(args.userId, sizeof(args.userId), "%s", userId.c_str());

        auto& service = getAppContext().pctrl_service;
        Result res = serviceDispatchInOut(&service, (u32)Ipc::Command::SetUserDailyLimit, args);

        if(R_FAILED(res)) {
            logError("[IPC] An error occured while setting the daily limit.\n");
            return false;
        }

        return true;
    }

    /*! @brief Checks if debug logging is enabled via IPC.
     * @return true if debug logging is enabled, false otherwise. */
    bool isDebugLogEnabled() {
        logDebug("[IPC] Getting log level\n");

        if(!isAvailable()) {
            logError("[IPC] service not available\n");
            return false;
        }

        u16 level = 0;

        auto& service = getAppContext().pctrl_service;
        Result res = serviceDispatchOut(&service, (u32)Ipc::Command::GetLogLevel, level);

        if(R_FAILED(res)) {
            logError("[IPC] An error occured while getting the log level.\n");
        } 

        logDebug("[IPC] Log level is %i\n", level);

        return level == DEBUG;
    }

    /*! @brief Enables or disables debug logging via IPC.
     * @param enable Whether to enable debug logging.
     * @return true if successful, false otherwise. */
    bool enableDebugLog(bool enable) {
        logDebug("[IPC] Setting log level\n");

        if(!isAvailable()) {
            logError("[IPC] service not available\n");
            return false;
        }

        auto& service = getAppContext().pctrl_service;
        const u8 level = enable ? DEBUG : INFO;
        Result res = serviceDispatchIn(&service, (u32)Ipc::Command::SetLogLevel, level); 

        if(R_FAILED(res)) {
            logError("[IPC] An error occured while setting the log level to DEBUG.\n");
            return false;
        }

        return true;
    }

    /*! @brief Checks if the database has been tampered with via IPC.
     * @return true if tampered, false otherwise. */
    bool isDatabaseTampered() {
        logDebug("[IPC] Verifying whether database has been tampered\n");

        if(!isAvailable()) {
            logError("[IPC] service not available\n");
            return false;
        }

        u8 tampered = 0;

        auto& service = getAppContext().pctrl_service;
        Result res = serviceDispatchOut(&service, (u32)Ipc::Command::IsTampered, tampered);

        if(R_FAILED(res)) {
            logError("[IPC] An error occured while getting the tampered state.\n");
        } 

        logDebug("[IPC] Tampered state is %i\n", tampered ? "tampered" : "clean");

        return tampered != 0;
    }

    /*! @brief Checks if the database needs an upgrade via IPC.
     * @return true if upgrade is needed, false otherwise. */
    bool isDatabaseNeedsUpgrade() {
        logDebug("[IPC] Verifying whether database needs upgrade\n");

        if(!isAvailable()) {
            logError("[IPC] service not available\n");
            return false;
        }

        u8 mustUpgrade = 0;

        auto& service = getAppContext().pctrl_service;
        Result res = serviceDispatchOut(&service, (u32)Ipc::Command::MustUpgradeDatabase, mustUpgrade);

        if(R_FAILED(res)) {
            logError("[IPC] An error occured while getting the tampered state.\n");
        } 

        logDebug("[IPC] Needs upgrade is %i\n", mustUpgrade ? "true" : "false");

        return mustUpgrade != 0;
    }        

}