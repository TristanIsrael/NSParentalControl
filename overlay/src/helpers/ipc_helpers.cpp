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

    bool isAvailable() {
        return getAppContext().is_available;
    }

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
            logDebug("Query usage time for uid %i\n", _uid);
            res = serviceDispatchInOut(&service, (u32)Ipc::Command::GetUserRemainingTime, _uid, remainingTime);
        }

        if(R_FAILED(res)) {
            logError("[IPC] An error occured while queriying the remaining time for the user\n");
        }

        return std::chrono::minutes(remainingTime);
    }

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

    /*std::vector<u64> decodeAdminPin(const std::string& pin) {
        std::vector<u64> keys;

        for (const auto part : std::views::split(pin, ",")) {
            std::string val = std::string(part.begin(), part.end());
            keys.push_back(std::stoull(val));
        }

        if(keys.size() != 4) {
            logToFile("[IPC] The decoded PIN is not 4 digits long.");
            return keys;
        }

        logToFile("[IPC] Decoded Admin PIN:");
        logIntToFile(keys[0]);
        logIntToFile(keys[1]);
        logIntToFile(keys[2]);
        logIntToFile(keys[3]);

        return keys;
    }*/

    bool verifyPin(const std::vector<u64>& pin) {
        logDebug("[IPC] Verify Admin PIN\n");

        if(!isAvailable()) {
            logError("[IPC] service not available\n");
            return false;
        }

        auto& service = getAppContext().pctrl_service;
        bool verified = false;
        u64 a_pin[4] = { 
            pin[0], pin[1], pin[2], pin[3]
        };        
        //std::strncpy(pin_str, pin.c_str(), pin.size());
        logDebug("PIN = %s\n", encodeAdminPin(pin).c_str());
        Result res = serviceDispatchInOut(&service, (u32)Ipc::Command::VerifyAdminPin, a_pin, verified);

        if(R_FAILED(res)) {
            logError("[IPC] An error occured during the Admin PIN verification.\n");
            return false;
        }

        return verified;
    }

    bool setupPin(const std::vector<u64>& pin) {
        logDebug("[IPC] Setup new PIN\n");

        if(!isAvailable()) {
            logError("[IPC] service not available\n");
            return false;
        }

        auto& service = getAppContext().pctrl_service;
        //char pin_str[PIN_LEN_MAX+1] = {0};
        //std::strncpy(pin_str, pin.c_str(), pin.size());
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

    u16 getDailyLimit() {
        logDebug("[IPC] Getting daily limit\n");

        if(!isAvailable()) {
            logError("[IPC] service not available\n");
            return 0;
        }

        u16 limit = 0;

        auto& service = getAppContext().pctrl_service;
        Result res = serviceDispatchOut(&service, (u32)Ipc::Command::GetDailyLimit, limit);

        if(R_FAILED(res)) {
            logError("[IPC] An error occured while getting the daily limit.\n");
        } 

        logDebug("[IPC] Daily limit is %i\n", limit);

        return limit;
    }

    bool setDailyLimit(const u16& limit) {
        logDebug("[IPC] Setting the daily limit\n");

        if(!isAvailable()) {
            logError("[IPC] service not available\n");
            return false;
        }

        auto& service = getAppContext().pctrl_service;
        Result res = serviceDispatchIn(&service, (u32)Ipc::Command::SetDailyLimit, limit);

        if(R_FAILED(res)) {
            logError("[IPC] An error occured while setting the daily limit.\n");
            return false;
        }

        return true;
    }

    bool isDebugLogEnabled() {
        logDebug("[IPC] Getting log level\n");

        if(!isAvailable()) {
            logError("[IPC] service not available\n");
            return 0;
        }

        u16 level = 0;

        auto& service = getAppContext().pctrl_service;
        Result res = serviceDispatchOut(&service, (u32)Ipc::Command::GetLogLevel, level);

        if(R_FAILED(res)) {
            logError("[IPC] An error occured while getting the daily limit.\n");
        } 

        logDebug("[IPC] Log level is %i\n", level);

        return level == DEBUG;
    }

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

    bool isDatabaseTampered() {
        logDebug("[IPC] Verifying whether database has been tampered\n");

        if(!isAvailable()) {
            logError("[IPC] service not available\n");
            return 0;
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

    bool isDatabaseNeedsUpgrade() {
        logDebug("[IPC] Verifying whether database needs upgrade\n");

        if(!isAvailable()) {
            logError("[IPC] service not available\n");
            return 0;
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

    std::vector<u64> getBlacklistedTitles(const UserUid& userId) {
        std::vector<u64> titles;
        
        logDebug("[IPC] Getting blacklisted titles list for user %s\n", userId.c_str());
        char chUserId[80] = {0};
        std::memcpy(chUserId, userId.data(), userId.size());        
        
        logDebug("[IPC] Getting blacklisted titles count\n");
        u8 titlesCount = 0;
        auto& service = getAppContext().pctrl_service;
        Result res = serviceDispatchInOut(&service, (u32)Ipc::Command::GetBlacklistedTitlesCount, chUserId, titlesCount);

        if(R_FAILED(res)) {
            logError("[IPC] An error occured while getting the blacklist size.\n");
            return titles;
        } 
        
        typedef struct {
            u8 index;
            char userid[80];            
        } Args;
        
        for(u8 i = 0 ; i < titlesCount ; i++) {
            logDebug("[IPC] Getting blacklist entry %i\n", i);

            Args args {
                .index = i
            };
            //std::memcpy(args.userid, userId.data(), sizeof(Args)-sizeof(args.index));
            std::snprintf(args.userid, sizeof(args.userid), "%s", userId.c_str());

            u64 titleId = 0;
            res = serviceDispatchInOut(&service, (u32)Ipc::Command::GetBlacklistedTitle, args, titleId);
            if(R_FAILED(res)) {
                logError("[IPC] An error occured while getting the blacklist title at index %i.\n", i);
            } else {
                titles.push_back(titleId);
            }

        }

        return titles;
    }

    bool addTitleToBlacklist(const UserData& user, u64 titleId) {
        logDebug("[IPC] add title %llu to blacklist for user %s\n", titleId, user.nickname.c_str());

        typedef struct {
            u64 titleId;
            char userid[80];
        } Args;

        Args args {
            .titleId = titleId,            
        };
        const auto& userId = accountUidToString(user.uid);
        //std::memcpy(args.userid, userId.data(), sizeof(args)-sizeof(args.titleId));
        std::snprintf(args.userid, sizeof(args.userid), "%s", userId.c_str());

        auto& service = getAppContext().pctrl_service;        
        u8 ok = 0;

        Result res = serviceDispatchInOut(&service, (u32)Ipc::Command::AddTitleToBlacklist, args, ok);
        if(R_FAILED(res)) {
            logError("[IPC] An error occured when adding title to blacklist\n");
            return false;
        }

        return true;
    }

    bool removeTitleFromBlacklist(const UserData& user, u64 titleId) {
        logDebug("[IPC] remove title %llu from blacklist for user %s\n", titleId, user.nickname.c_str());

        typedef struct {
            u64 titleId;
            char userid[80];
        } Args;

        Args args {
            .titleId = titleId,            
        };
        const auto& userId = accountUidToString(user.uid);
        //std::memcpy(args.userid, userId.data(), sizeof(args)-sizeof(args.titleId));
        std::snprintf(args.userid, sizeof(args.userid), "%s", userId.c_str());

        auto& service = getAppContext().pctrl_service;        
        u8 ok = 0;

        Result res = serviceDispatchInOut(&service, (u32)Ipc::Command::RemoveTitleFromBlacklist, args, ok);
        if(R_FAILED(res)) {
            logError("[IPC] An error occured when removing title to blacklist\n");
            return false;
        }

        return true;
    }
}