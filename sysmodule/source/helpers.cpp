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
using namespace alefbet::pctrl::database::history;
using namespace alefbet::pctrl::database::settings;
using std::operator""sv;

namespace alefbet::pctrl::helpers {
    static FsFileSystem sdmc;

    /*! @brief Converts a title ID to a hexadecimal string representation.
     *
     * @param titleId The title ID to convert.
     * @return Hexadecimal string (16 chars uppercase), or "None" if titleId is 0.
     */
    std::string titleIdToString(u64 titleId) {
        if(titleId == 0) {
            return std::string("None");        
        }
        
        std::stringstream ss;
        ss << std::uppercase << std::setfill('0') << std::setw(16) << std::hex << titleId;
        return ss.str();
    }

    /*! @brief Converts an AccountUid struct to a colon-separated string.
     *
     * @param uid The AccountUid to convert.
     * @return String in format "uid[0]:uid[1]".
     */
    UserUid accountUidToString(AccountUid uid) {
        return std::to_string(uid.uid[0]) + ":" + std::to_string(uid.uid[1]);
    }

    /*! @brief Parses a colon-separated string into an AccountUid struct.
     *
     * @param uid_str String in format "uid[0]:uid[1]".
     * @return Parsed AccountUid, or zeros on failure.
     */
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

    /*! @brief Retrieves full user data (UID and nickname) from an AccountUid.
     *
     * Initializes the account service, fetches the profile, and returns
     * a UserData struct with the retrieved information.
     *
     * @param uid The AccountUid to query.
     * @return UserData struct with uid and nickname, or error nickname on failure.
     */
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

    /*! @brief Retrieves the currently active user's data.
     *
     * Queries the last opened user account and returns their UID and nickname.
     *
     * @return UserData struct, or error nickname on failure.
     */
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

    /*! @brief Retrieves the process ID of the currently running application.
     *
     * @return The application process ID, or 0 on failure.
     */
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

    /*! @brief Retrieves the title ID for a given process ID.
     *
     * @param process_id The process ID to query.
     * @return The title ID, or 0 on failure.
     */
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

    /*! @brief Retrieves the application name for a given title ID.
     *
     * @param title_id The title ID to query.
     * @return The application name, or "Unknown" on failure.
     */
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

    /*! @brief Retrieves today's date as a string in YYYYMMDD format.
     *
     * @return Date string, or empty string on failure.
     */
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

    /*! @brief Retrieves the current time in minutes since midnight.
     *
     * @return Current time in minutes since midnight (0-1439).
     */
    u16 getCurrentTimeInMinutesSinceMidnight() {
        ::Result rc = timeInitialize();
        if(R_FAILED(rc)) {
            logError("[Helpers] Could not connect to time service\n");
            return 0;
        }

        u64 ts;
        TimeCalendarTime time;
        TimeCalendarAdditionalInfo info;
        rc = timeGetCurrentTime(TimeType_LocalSystemClock, &ts);
        if(R_FAILED(rc)) {
            logError("[Helpers] Could not get current time (err %i:%i)\n", R_MODULE(rc), R_DESCRIPTION(rc));
            return 0;
        }

        rc = timeToCalendarTimeWithMyRule(ts, &time, &info);
        if(R_FAILED(rc)) {
            logError("[Helpers] Could not convert timestamp\n");
            return 0;
        }

        return static_cast<u16>(time.hour * 60 + time.minute);
    }

    /*! @brief Formats minutes since midnight as HH:mm string.
     *
     * @param minutesSinceMidnight Minutes since midnight (0-1439).
     * @return Formatted time string (e.g. "21:30").
     */
    std::string formatTimeInMinutes(u16 minutesSinceMidnight) {
        u16 hours = minutesSinceMidnight / 60;
        u16 minutes = minutesSinceMidnight % 60;
        
        char buffer[12] = {0};
        std::snprintf(buffer, sizeof(buffer), "%02d:%02d", hours, minutes);
        
        return std::string(buffer);
    }

    /*! @brief Reads the reboot payload file from the SD card.
     *
     * @param buffer Output buffer to receive the payload data.
     * @param buffer_size Size of the output buffer.
     * @return true on success, false on failure.
     */
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
    /*! @brief Configures and reboots the console to the recovery payload.
     *
     * Initializes SPSM and AMS:BPC services, loads the payload from SD card,
     * and triggers a reboot. Shuts down the system if payload is missing.
     *
     * @return true on success, false on failure.
     */
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

    /*! @brief Terminates the currently running application title.
     *
     * @return true on success, false on failure.
     */
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

    /*! @brief Checks whether the currently running title is blacklisted for the current user.
     *
     * @return true if the title is blacklisted, false otherwise.
     */
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

    /*! @brief Adds a title to the blacklist for a specific user.
     *
     * @param userId The user ID string.
     * @param titleId The title ID to black list.
     */
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
        
    /*! @brief Removes a title from the blacklist for a specific user.
     *
     * @param userId The user ID string.
     * @param titleId The title ID to remove from the blacklist.
     */
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

    static std::string serializeBedtimes(const std::map<std::string, u16>& bedtimesMap) {
        std::ostringstream oss;
        bool first = true;
        for(const auto& [userId, limit] : bedtimesMap) {
            if(!first) oss << ';';
            first = false;
            oss << userId << '=' << limit;
        }
        return oss.str();
    }

    static std::map<std::string, u16> deserializeBedtimes(const std::string& data) {
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

    /*! @brief Retrieves the daily usage limit for a specific user.
     *
     * @param userId The user ID string.
     * @return The daily limit in minutes, or 0 if not set.
     */
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

    /*! @brief Sets the daily usage limit for a specific user.
     *
     * @param userId The user ID string.
     * @param limit_in_minutes The daily limit in minutes.
     */
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

    /*! @brief Gets the bed time for a user
     *
     * @param userId The user ID string.
     */
    u16 getBedtime(const std::string& userId) {
        auto& settings = loadSettings();

        if(!settings.contains(SETTING_BEDTIME_USERS)) {
            return 0;
        }

        const auto& bedtimesStr = settings[SETTING_BEDTIME_USERS].string_value;
        if(bedtimesStr.empty()) {
            logDebug("[Helpers] No bedtime for user %s\n", userId.c_str());
            return 0;
        }

        auto limitsMap = deserializeBedtimes(bedtimesStr);
        auto it = limitsMap.find(userId);
        if(it != limitsMap.end()) {
            return it->second;
        }

        logDebug("[Helpers] No bedtime for user %s\n", userId.c_str());
        return 0;
    }
        
    /*! @brief Sets the bed time for a specific user.
     *
     * @param userId The user ID string.
     * @param bedtime_in_minutes The bed time in minutes from midnight.
     */
    void setBedtime(const std::string& userId, u16 bedtime_in_minutes) {
        auto limitsMap = deserializeBedtimes("");
        
        auto& settings = loadSettings();
        if(settings.contains(SETTING_BEDTIME_USERS)) {
            limitsMap = deserializeBedtimes(settings[SETTING_BEDTIME_USERS].string_value);
        }

        limitsMap[userId] = bedtime_in_minutes;

        Setting setting {
            .key = SETTING_BEDTIME_USERS,
            .type = STRING,
            .string_value = serializeBedtimes(limitsMap)
        };

        saveSetting(setting);
    }

    /*! @brief Encodes a 4-digit PIN vector into a comma-separated string.
     *
     * @param password Vector of 4 u64 values representing the PIN digits.
     * @return Comma-separated string (e.g., "1234,5678,9012,3456"), or empty on failure.
     */
    std::string encodePassword(const std::vector<u64>& password) {
        std::string pin;

        if(password.size() != 4) {
            logError("[Helpers] The PIN size is wrong.");
            return pin;
        }

        pin = std::to_string(password[0]) +"," +std::to_string(password[1]) +"," +std::to_string(password[2]) +"," +std::to_string(password[3]);        

        return pin;    
    }

    /*! @brief Decodes a comma-separated PIN string into a vector of 4 u64 values.
     *
     * @param str Comma-separated PIN string (e.g., "1234,5678,9012,3456").
     * @return Vector of 4 u64 values, or empty vector on failure.
     */
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
    
    /*! @brief Retrieves the total usage time in minutes for a user today.
     *
     * @param uid The AccountUid of the user.
     * @return Total usage time in minutes for today.
     */
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