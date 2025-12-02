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
                        accountExit();                        
                    } else {
                        logDebug("[Helpers] accountGetProfile() ok\n");
                    }

                    rc = accountProfileGet(&profile, &user_data, &base);
                    if(rc != 0) {
                        logError("[Helpers] Could not get user data (%i:%i)\n", R_MODULE(rc), R_DESCRIPTION(rc));
                        accountProfileClose(&profile);
                        accountExit();                        
                    } else {
                        logDebug("[Helpers] accountProfileGet() ok\n");
                    }

                    UserData user;
                    user.uid = uid;
                    user.nickname = base.nickname;

                    users.push_back(user);
                }

                return users;
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
                    logError("[Helpers] Incorrect split of AccountUid : %s\n", uid_str.c_str());
                    return uid;
                }

                uid.uid[0] = std::stoull(parts[0]);
                uid.uid[1] = std::stoull(parts[1]);

                return uid;
            }

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