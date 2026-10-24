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

#include "helpers.h"
#include <sstream>
#include <iomanip>
#include <cinttypes>
#include <iostream>
#include <vector>
#include <list>
#include <string_view>
#include <ranges>
#include <codecvt>
#include <switch.h>
#include <algorithm>
#include "logger.h"
#include "database/history_db.h"
#include "database/settings_db.h"
#include "ams_bpc.h"

using namespace alefbet::pctrl::logger;
using namespace alefbet::pctrl::structs;
using namespace alefbet::pctrl::database;
using std::operator""sv;

namespace alefbet::pctrl::helpers {
    static FsFileSystem sdmc;

    std::string titleIdToString(u64 titleId) {
        if(titleId == 0) {
            return std::string("None");        
        }
        
        std::stringstream ss;
        ss << std::uppercase << std::setfill('0') << std::setw(16) << std::hex << titleId;
        return ss.str();
    }

    UserUid accountUidToString(AccountUid uid) {
        return std::to_string(uid.uid[0]) + ":" + std::to_string(uid.uid[1]);
    }

    AccountUid accountUidFromString(const UserUid& uid_str) {
        AccountUid uid;

        std::vector<std::string> parts;
        for (const auto part : std::views::split(uid_str, ":"sv)) {
            parts.push_back(std::string(part.begin(), part.end()));
        }
        
        if(parts.size() != 2) {
            logError("[Helpers] Incorrect split of AccountUid %s.\n", uid_str.c_str());
            return uid;
        }

        uid.uid[0] = std::stoull(parts[0]);
        uid.uid[1] = std::stoull(parts[1]);

        return uid;
    }

    UserData getUserFromAccountUid(AccountUid uid) {
        UserData user;

        ::Result rc = accountInitialize(AccountServiceType_Administrator);
        if(R_FAILED(rc)) {
            logError("[Helpers] Could not initialize account service: %i:%i\n", R_MODULE(rc), R_DESCRIPTION(rc));
            user.nickname = UserNickname("ERR#003");
            return user;
        }

        AccountProfile profile;
        AccountUserData user_data;
        AccountProfileBase base;
        rc = accountGetProfile(&profile, uid); 
        if(R_FAILED(rc)) {
            logError("[Helpers] Could not get account profile: %i\n", rc);
            accountExit();
            user.nickname = UserNickname("ERR#005");
            return user;
        } else {
            logDebug("[Helpers] accountGetProfile() ok\n");
        }

        rc = accountProfileGet(&profile, &user_data, &base);
        if(R_FAILED(rc)) {
            logError("[Helpers] Could not get user data: %i\n", rc);
            accountProfileClose(&profile);
            accountExit();
            user.nickname = UserNickname("ERR#006");
            return user;
        } else {
            logDebug("[Helpers] accountProfileGet() ok\n");
        }
        
        user.uid = uid;
        user.nickname = UserNickname(base.nickname);
        UserUid uidstr = accountUidToString(uid);
        logDebug("[Helpers] uid=%s, Nickname=%s\n", uidstr.c_str(), user.nickname.c_str());

        accountProfileClose(&profile);
        accountExit();

        return user;
    }

    UserData getCurrentUser() {
        UserData user;

        ::Result rc = accountInitialize(AccountServiceType_Administrator);
        if(R_FAILED(rc)) {
            logError("[Helpers] Could not initialize account service: %i:%i\n", R_MODULE(rc), R_DESCRIPTION(rc));
            user.nickname = UserNickname("ERR#003");
            return user;
        }

        AccountUid uid;
        rc = accountGetPreselectedUser(&uid);
        //logToFile("rc=%i, Uid=%i.%i\n", rc, uid.uid[0], uid.uid[1]);
        rc = accountGetLastOpenedUser(&uid);            
        if(R_FAILED(rc)) {
            logError("[Helpers] Could not get preselected user: %i:%i\n", R_MODULE(rc), R_DESCRIPTION(rc));
            accountExit();
            user.nickname = UserNickname("ERR#004");
            return user;
        }

        logDebug("[Helpers] user uid: %i.%i\n", uid.uid[0], uid.uid[1]);
        user.uid = uid;

        AccountProfile profile;
        AccountUserData user_data;
        AccountProfileBase base;
        rc = accountGetProfile(&profile, uid); 
        if(R_FAILED(rc)) {
            logError("[Helpers] Could not get account profile: %i\n", rc);
            accountExit();
            user.nickname = UserNickname("ERR#005");
            return user;
        } else {
            logDebug("[Helpers] accountGetProfile() ok\n");
        }

        rc = accountProfileGet(&profile, &user_data, &base);
        if(R_FAILED(rc)) {
            logError("[Helpers] Could not get user data: %i\n", rc);
            accountProfileClose(&profile);
            accountExit();
            user.nickname = UserNickname("ERR#006");
            return user;
        } else {
            logDebug("[Helpers] accountProfileGet() ok\n");
        }
        
        user.nickname = UserNickname(base.nickname);
        logDebug("[Helpers] uid=%i:%i, Nickname=%s\n", user.uid.uid[0], user.uid.uid[1], user.nickname.c_str());

        accountProfileClose(&profile);
        accountExit();

        return user;
    }

    u64 getRunningApplicationPid() {
        u64 process_id = 0;                        

        ::Result rc = pmdmntGetApplicationProcessId(&process_id);
        if(R_FAILED(rc)) {
            //logToFile("[Helpers] Could not get application process ID: %i:%i\n", R_MODULE(rc), R_DESCRIPTION(rc));
            return 0;
        }

        //logToFile("[Helpers] process ID=%i\n", process_id);

        return process_id;
    }

    u64 getRunningApplicationTitleId(u64 process_id) {
        u64 title_id = 0;

        ::Result rc = pmdmntGetProgramId(&title_id, process_id);
        if(R_FAILED(rc)) {
            //logToFile("[Helpers] Could not get the title ID for the process %i:%i\n", R_MODULE(rc), R_DESCRIPTION(rc));
            return 0;
        }
        
        auto formatted_title_id = titleIdToString(title_id);
        //logToFile("[Helpers] title formatted ID=%s, ID=%i, pid=%i\n", formatted_title_id.c_str(), title_id, process_id);

        return title_id;
    }

    std::string getApplicationName(u64 title_id) {
        NsApplicationControlData control;
        u64 actual_size = 0;
        
        ::Result rc = nsGetApplicationControlData(NsApplicationControlSource_Storage, title_id, std::addressof(control), sizeof(control), &actual_size);
        if(R_FAILED(rc)) {
            logError("[Helpers] Could not get application control data for title %s\n", titleIdToString(title_id));
            return "Unknown";
        }

        NacpLanguageEntry* langEntry = nullptr;
        rc = nacpGetLanguageEntry(&control.nacp, &langEntry);
        if(R_FAILED(rc) || langEntry == nullptr) {
            return "Unknown";
        }

        return std::string(langEntry->name);
    }

    std::string today() {        
        ::Result rc = timeInitialize();
        if(R_FAILED(rc)) {
            logError("[Helpers] Could not connect to time service\n");
            return "";
        }

        u64 ts;
        TimeCalendarTime time;
        TimeCalendarAdditionalInfo info;
        rc = timeGetCurrentTime(TimeType_LocalSystemClock, &ts);        
        if(R_FAILED(rc)) {
            logError("[Helpers] Could not get current time (err %i:%i)\n", R_MODULE(rc), R_DESCRIPTION(rc));
            return "";
        }

        rc = timeToCalendarTimeWithMyRule(ts, &time, &info);
        if(R_FAILED(rc)) {
            logError("[Helpers] Could not convert timestamp\n");
            return "";
        }
        
        char buffer[13] = {0};
        std::snprintf(buffer, sizeof(buffer), "%04d%02d%02d",
            time.year,
            time.month,
            time.day
        );

        return std::string(buffer);
    }

    bool readPayloadFile(u8* buffer, u64 buffer_size) {
        FsFile handle;
        
        ::Result res = fsOpenSdCardFileSystem(&sdmc);
        if(R_FAILED(res)) {
            logError("[Helpers] Could not open SDMC\n");
            return false;
        }

        res = fsFsOpenFile(&sdmc, "/atmosphere/reboot_payload.bin", FsOpenMode_Read, &handle);
        if(R_FAILED(res)) {
            logError("[Helpers] Could not open payload file\n");
            return false;
        }

        s64 fileSize = 0;
        res = fsFileGetSize(&handle, &fileSize);
        if(R_FAILED(res)) {
            logError("[Helpers] Could not get file size\n");
            fsFileClose(&handle);
            return false;
        }

        logDebug("[Helpers] Payload file size is %i bytes\n", fileSize);

        u64 dataRead = 0;
        res = fsFileRead(&handle, 0, buffer, buffer_size, FsReadOption_None, &dataRead);
        if(R_FAILED(res)) {
            logError("[Helpers] Could not read %i bytes from payload file\n", buffer_size);
            fsFileClose(&handle);
            return false;
        }

        fsFileClose(&handle);

        logDebug("[Helpers] Read %i bytes from payload file\n", dataRead);
        return true;
    }

    #define IRAM_PAYLOAD_MAX_SIZE 0x24000
    bool rebootToPayload() {
        logInfo("[Helpers] Try to reboot to payload\n");
      
        u8 *g_reboot_payload = new u8[IRAM_PAYLOAD_MAX_SIZE];
        smInitialize();
        if(!readPayloadFile(g_reboot_payload, IRAM_PAYLOAD_MAX_SIZE)) {
            logError("[Helpers] No payload, shutting down.");
            bpcShutdownSystem();
            return false;
        }

        ::Result rc = spsmInitialize();        
        if(R_FAILED(rc)) {
            logError("[Helpers] Failed to initialize SPSM\n");
            bpcShutdownSystem();
            return false;
        }
        
        smExit(); //Required to connect to ams:bpc       

        rc = amsBpcInitialize();
        if (R_FAILED(rc)) {
            logError("[Helpers] Failed to initialize ams:bpc: %i\n", rc);
            logError("[Helpers] Shutting down.\n");
            bpcShutdownSystem();
            return false;
        }    

        logDebug("[Helpers] Calling amsBpcSetRebootPayload\n");        
        if (R_FAILED(amsBpcSetRebootPayload(g_reboot_payload, IRAM_PAYLOAD_MAX_SIZE))) {
            logError("[Helpers] Failed to set reboot to payload: %i\n", rc);
            logError("[Helpers] Shutting down.\n");

            bpcShutdownSystem();
        } else {
            logInfo("[Helpers] Rebooting to payload\n");

            spsmShutdown(true);
        }

        return true;
    }

    bool terminateCurrentApplication() {
        logInfo("[Helpers] Trying to terminate the current title\n");

        //Get the current title
        u64 pid = getRunningApplicationPid();
        if(pid > 0) {
            u64 titleId = getRunningApplicationTitleId(pid);
            if(titleId > 0) {
                pmshellInitialize();
                if(R_FAILED(pmshellTerminateProgram(titleId))) {
                    logError("[Helpers] Could not terminate the title with titleId %s\n", titleIdToString(titleId).c_str());
                    pmshellExit();
                    return false;
                }
                pmshellExit();
            } else {
                logDebug("[Helpers] No title id found for PID %llu\n", pid);
                return false;
            }
        } else {
            logDebug("[Helpers] No PID found (No running app?)\n");
            return false;
        }

        return true;
    }

    bool isCurrentTitleBlacklisted() {
        logDebug("[Helpers] Checking whether the current title is blacklisted\n");

        const auto& user = getCurrentUser();
        u64 pid = getRunningApplicationPid();

        if(pid > 0) {
            const auto& titleId = getRunningApplicationTitleId(pid);

            if(titleId > 0) {
                const auto& userId = accountUidToString(user.uid);
                const auto& blacklist = getBlacklistedTitlesForUser(userId);

                const auto& val = std::find_if(blacklist.begin(), blacklist.end(), [titleId](const u64& title) {
                    return titleId == title;
                });

                return val != blacklist.end();
            }
        }

        return false;
    }

    /*!
        \brief Returns the list of blacklisted titles for a user
    */
    std::vector<u64> getBlacklistedTitlesForUser(const std::string& userId) {
        std::vector<u64> titles;

        auto& settings = loadSettings();
        if(!settings.contains(SETTING_BLACKLIST)) {
            return titles;
        }

        const auto& blacklistStr = settings[SETTING_BLACKLIST].string_value;
        if(blacklistStr.empty()) {
            logDebug("[Helpers] No title blacklisted for user %s\n", userId.c_str());
            return titles;
        }

        std::istringstream stream(blacklistStr);
        std::string line;
        while(std::getline(stream, line, ';')) {
            if(line.empty()) continue;
            auto eqPos = line.find('=');
            if(eqPos == std::string::npos) continue;
            std::string lineUser = line.substr(0, eqPos);
            std::string lineTitles = line.substr(eqPos + 1);
            if(lineUser != userId) continue;
            std::istringstream titleStream(lineTitles);
            std::string titlePart;
            while(std::getline(titleStream, titlePart, ',')) {
                if(!titlePart.empty()) {
                    titles.push_back(std::stoull(titlePart));
                }
            }
        }

        return titles;
    }

    static std::string serializeBlacklist(const std::map<std::string, std::vector<u64>>& blacklistMap) {
        std::ostringstream oss;
        bool firstUser = true;
        for(const auto& [userId, titleIds] : blacklistMap) {
            if(!firstUser) oss << ';';
            firstUser = false;
            oss << userId << '=';
            bool firstTitle = true;
            for(u64 titleId : titleIds) {
                if(!firstTitle) oss << ',';
                firstTitle = false;
                oss << titleId;
            }
        }
        return oss.str();
    }

    static std::map<std::string, std::vector<u64>> deserializeBlacklist(const std::string& data) {
        std::map<std::string, std::vector<u64>> result;
        std::istringstream stream(data);
        std::string line;
        while(std::getline(stream, line, ';')) {
            if(line.empty()) continue;
            auto eqPos = line.find('=');
            if(eqPos == std::string::npos) continue;
            std::string userId = line.substr(0, eqPos);
            std::string titlesStr = line.substr(eqPos + 1);
            std::istringstream titleStream(titlesStr);
            std::string titlePart;
            while(std::getline(titleStream, titlePart, ',')) {
                if(!titlePart.empty()) {
                    result[userId].push_back(std::stoull(titlePart));
                }
            }
        }
        return result;
    }

    void addToBlacklist(const std::string& userId, u64 titleId) {
        auto blacklistMap = deserializeBlacklist("");
        
        auto& settings = loadSettings();
        if(settings.contains(SETTING_BLACKLIST)) {
            blacklistMap = deserializeBlacklist(settings[SETTING_BLACKLIST].string_value);
        }

        auto& userTitles = blacklistMap[userId];
        
        const auto& title = std::find_if(userTitles.begin(), userTitles.end(), [titleId](u64 title) {
            return title == titleId;
        });

        if(title == userTitles.end()) {
            logDebug("[Database] add title\n");
            userTitles.push_back(titleId);
        }

        Setting setting {
            .key = SETTING_BLACKLIST,
            .type = STRING,
            .string_value = serializeBlacklist(blacklistMap)
        };

        saveSetting(setting);
    }
        
    void removeFromBlacklist(const std::string& userId, u64 titleId) {
        auto blacklistMap = deserializeBlacklist("");
        
        auto& settings = loadSettings();
        if(settings.contains(SETTING_BLACKLIST)) {
            blacklistMap = deserializeBlacklist(settings[SETTING_BLACKLIST].string_value);
        }

        auto& userTitles = blacklistMap[userId];

        const auto& title = std::find_if(userTitles.begin(), userTitles.end(), [titleId](u64 title) {
            return title == titleId;
        });

        if(title == userTitles.end()) {
            logDebug("[Helpers] Title %llu not found in blacklist\n", titleId);
            return;
        }

        logDebug("[Helpers]  Remove title\n");
        userTitles.erase(std::remove_if(userTitles.begin(), userTitles.end(), [titleId](u64 title) {
            return titleId == title;
        }), userTitles.end());

        Setting setting {
            .key = SETTING_BLACKLIST,
            .type = STRING,
            .string_value = serializeBlacklist(blacklistMap)
        };

        saveSetting(setting);
    }

    static std::string serializeDailyLimits(const std::map<std::string, u16>& limitsMap) {
        std::ostringstream oss;
        bool first = true;
        for(const auto& [userId, limit] : limitsMap) {
            if(!first) oss << ';';
            first = false;
            oss << userId << '=' << limit;
        }
        return oss.str();
    }

    static std::map<std::string, u16> deserializeDailyLimits(const std::string& data) {
        std::map<std::string, u16> result;
        std::istringstream stream(data);
        std::string line;
        while(std::getline(stream, line, ';')) {
            if(line.empty()) continue;
            auto eqPos = line.find('=');
            if(eqPos == std::string::npos) continue;
            std::string userId = line.substr(0, eqPos);
            u16 limit = static_cast<u16>(std::stoul(line.substr(eqPos + 1)));
            result[userId] = limit;
        }
        return result;
    }

    u16 getDailyLimitForUser(const std::string& userId) {
        auto& settings = loadSettings();

        if(!settings.contains(SETTING_DAILY_LIMIT_USERS)) {
            return 0;
        }

        const auto& limitsStr = settings[SETTING_DAILY_LIMIT_USERS].string_value;
        if(limitsStr.empty()) {
            logDebug("[Helpers] No daily limit for user %s\n", userId.c_str());
            return 0;
        }

        auto limitsMap = deserializeDailyLimits(limitsStr);
        auto it = limitsMap.find(userId);
        if(it != limitsMap.end()) {
            return it->second;
        }

        logDebug("[Helpers] No daily limit for user %s\n", userId.c_str());
        return 0;
    }

    void setDailyLimitForUser(const std::string& userId, u16 limit_in_minutes) {
        auto limitsMap = deserializeDailyLimits("");
        
        auto& settings = loadSettings();
        if(settings.contains(SETTING_DAILY_LIMIT_USERS)) {
            limitsMap = deserializeDailyLimits(settings[SETTING_DAILY_LIMIT_USERS].string_value);
        }

        limitsMap[userId] = limit_in_minutes;

        Setting setting {
            .key = SETTING_DAILY_LIMIT_USERS,
            .type = STRING,
            .string_value = serializeDailyLimits(limitsMap)
        };

        saveSetting(setting);
    }

    std::string encodePassword(const std::vector<u64>& password) {
        std::string pin;

        if(password.size() != 4) {
            logError("[Helpers] The PIN size is wrong.");
            return pin;
        }

        pin = std::to_string(password[0]) +"," +std::to_string(password[1]) +"," +std::to_string(password[2]) +"," +std::to_string(password[3]);        

        return pin;    
    }

    std::vector<u64> decodePassword(const std::string& str) {
        std::vector<u64> password;
        password.reserve(4);

        std::stringstream ss(str);
        std::string part;

        while (std::getline(ss, part, ',')) {
            password.push_back(static_cast<u64>(std::stoull(part)));
        }

        if (password.size() != 4) {
            // Incorrect format
            logError("[Helpers] The PIN size is wrong.");
            return {};
        }

        return password;
    }
    
    u16 getUserUsageTimeForToday(const AccountUid& uid) {
        const auto history = getHistory(uid, today());
        auto usage_time_in_minutes = (u16)0;        

        // Compute the total usage for today 
        for(const auto& entry: history) {
            usage_time_in_minutes += entry.durationInMinutes();
        }

        return usage_time_in_minutes;
    }
}