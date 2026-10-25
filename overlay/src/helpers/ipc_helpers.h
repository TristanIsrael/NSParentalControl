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

    u16 getBedtime(const UserData& user);
    bool setBedtime(const UserData& user, u16 bedtime);

    bool isDebugLogEnabled();
    bool enableDebugLog(bool enable);

    std::string getVersion();    

    u16 getAdjustedCurrentTime();

    bool isDatabaseTampered();
    bool isDatabaseNeedsUpgrade();    
}