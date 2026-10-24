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

#include "switch_helpers.h"
#include "logger.h"
#include <string_view>
#include <string_view>
#include <ranges>
#include <vector>

using namespace alefbet::pctrl::logger;

namespace alefbet {
    namespace pctrl {
        namespace helpers {        

            using std::operator""sv;

            /*! @brief Retrieves a list of all user accounts on the Switch.
             * @return List of UserData for all users. */
            std::list<UserData> getUsersList() {
                std::list<UserData> users;

                Result rc = accountInitialize(AccountServiceType_System);
                if(R_FAILED(rc)) {
                    logError("[Service] Could not connect to service account (%i:%i)\n", R_MODULE(rc), R_DESCRIPTION(rc));
                    return std::list<UserData>{};
                }

                AccountUid _users[ACC_USER_LIST_SIZE];
                s32 count = 0;

                rc = accountListAllUsers(_users, ACC_USER_LIST_SIZE, &count);
                if(R_FAILED(rc)) {
                    logError("[Service] Could not enumerate users (%i:%i)\n", R_MODULE(rc), R_DESCRIPTION(rc));
                    accountExit();
                    return std::list<UserData>{};
                }

                AccountProfile profile;
                AccountUserData user_data;
                AccountProfileBase base;

                for(int i = 0 ; i < count ; i++) {
                    const auto& uid = _users[i];
                    rc = accountGetProfile(&profile, uid); 
                    if(rc != 0) {
                        logError("[Helpers] Could not get account profile (%i:%i)\n", R_MODULE(rc), R_DESCRIPTION(rc));
                        continue;
                    } else {
                        logDebug("[Helpers] accountGetProfile() ok\n");
                    }

                    rc = accountProfileGet(&profile, &user_data, &base);
                    if(rc != 0) {
                        logError("[Helpers] Could not get user data (%i:%i)\n", R_MODULE(rc), R_DESCRIPTION(rc));
                        accountProfileClose(&profile);
                        continue;
                    } else {                        
                        logDebug("[Helpers] accountProfileGet() ok\n");
                    }

                    accountProfileClose(&profile);

                    UserData user;
                    user.uid = uid;
                    user.nickname = base.nickname;

                    users.push_back(user);
                }
                
                accountExit();
                return users;
            }

            /*! @brief Converts an AccountUid to a colon-separated string representation.
             * @param uid The AccountUid to convert.
             * @return String representation of the UID in "low:high" format. */
            UserUid accountUidToString(AccountUid uid) {
                return std::to_string(uid.uid[0]) + ":" + std::to_string(uid.uid[1]);
            }

            /*! @brief Converts a colon-separated UID string back to an AccountUid.
             * @param uid_str The UID string in "low:high" format.
             * @return The parsed AccountUid. */
            AccountUid accountUidFromString(const UserUid& uid_str) {
                AccountUid uid;

                std::vector<std::string> parts;
                for (const auto part : std::views::split(uid_str, ":"sv)) {
                    parts.push_back(std::string(part.begin(), part.end()));
                }
                
                if(parts.size() != 2) {
                    logError("[Helpers] Incorrect split of AccountUid : %s\n", uid_str.c_str());
                    return uid;
                }

                uid.uid[0] = std::stoull(parts[0]);
                uid.uid[1] = std::stoull(parts[1]);

                return uid;
            }

            /*! @brief Gets the application title name for a given title ID.
             * @param titleId The title ID to look up.
             * @return The application name, or an error string on failure. */
            std::string getTitleName(u64 titleId) {
                bool ok = R_SUCCEEDED(nsInitialize());
                if(!ok) {
                    logError("[Helpers] Could not initialize NS service\n");
                    return "Error #11";
                }

                s32 count = 0;
                NsApplicationRecord records[100]; 
                if(ok) {               
                    ok = R_SUCCEEDED(nsListApplicationRecord(records, sizeof(records), 0, &count));
                } 

                if(!ok) {
                    logError("[Helpers] Could not get application record count\n");
                    nsExit();
                    return "Error #12";                    
                }

                NsApplicationControlData nacp;
                NacpLanguageEntry* langEntry = nullptr;
                u64 actual_size = 0;

                if(ok) {
                    ok = R_SUCCEEDED(nsGetApplicationControlData(NsApplicationControlSource_Storage, titleId, &nacp, sizeof(nacp), &actual_size));                    

                    if(!ok) {
                        logError("[Helpers] Could not get application information for %ull\n", titleId);
                        nsExit();
                        return "Error #13";                
                    } else {                
                        if(R_SUCCEEDED(nacpGetLanguageEntry(&nacp.nacp, &langEntry)) && langEntry != nullptr) {
                            nsExit();
                            return langEntry->name;                            
                        } else {
                            nsExit();
                            return "Error #14";
                        }                
                    }
                }

                nsExit();
                return "#NoName#";
            }
        }
    }
}