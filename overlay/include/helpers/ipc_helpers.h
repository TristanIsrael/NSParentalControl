#pragma once
#include <string>
#include <vector>
#include <list>
#include <switch.h>
#include <chrono>
#include "switch_helpers.h"

using namespace alefbet::pctrl::helpers;

namespace alefbet::pctrl::ipc {
    typedef enum {
        WorkingModeInfo = 0,
        WorkingModeBlocking = 1
    } WorkingMode;
    
    typedef enum {
        DEBUG = 0,                
        INFO = 5,
        ERROR = 10
    } LogLevel;

    constexpr int UserUidMaxLength = 39;

    bool isAvailable();

    void startTest();    
    UserUid getCurrentUserUid();
    UserNickname getCurrentUserNickname();
    std::string getCurrentTitle();
    std::chrono::minutes getUserUsageTime(const AccountUid& accountUid = AccountUid{});
    std::chrono::minutes getUserRemainingTime(const AccountUid& accountUid = AccountUid{});

    std::string encodeAdminPin(const std::vector<u64>&);
    bool verifyPin(const std::vector<u64>&);
    bool setupPin(const std::vector<u64>&);

    bool setWorkingMode(const WorkingMode&);
    WorkingMode getWorkingMode();
    bool setShowRemainingTime(const bool&);
    bool getShowRemainingTime();
    bool isEnabled();
    bool setEnabled(const bool&);
    
    u16 getDailyLimit(const UserData& user);
    bool setDailyLimit(const UserData& user, u16 limit);

    bool isDebugLogEnabled();
    bool enableDebugLog(bool enable);

    std::string getVersion();    

    bool isDatabaseTampered();
    bool isDatabaseNeedsUpgrade();

    std::vector<u64> getBlacklistedTitles(const UserUid&);
    bool addTitleToBlacklist(const UserData& user, u64 titleId);
    bool removeTitleFromBlacklist(const UserData& user, u64 titleId);

    bool isAuthenticationEnabled();
    bool setAuthenticationState(bool);
}