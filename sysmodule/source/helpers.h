#pragma once
#include <string>
#include <vector>
#include "types.h"
#include "logger.h"

namespace alefbet::pctrl {

    #define TRY_AND_LOG(res_expr, message)                                      \
    {                                                                           \
        const auto _tmp_r_try_rc = (res_expr);                                  \
        if (R_FAILED(_tmp_r_try_rc)) {                                          \
            logger::logError("%s. Error %i:%i\n", message, R_MODULE(_tmp_r_try_rc), R_DESCRIPTION(_tmp_r_try_rc));     \
            return;                                                             \
        }                                                                       \
    } 

    #define TRY_AND_RETURN(res_expr, message)                                   \
    {                                                                           \
        const auto _tmp_r_try_rc = (res_expr);                                  \
        if (R_FAILED(_tmp_r_try_rc)) {                                          \
            logger::logError("%s. Error %i:%i\n", message, R_MODULE(_tmp_r_try_rc), R_DESCRIPTION(_tmp_r_try_rc));     \
            return _tmp_r_try_rc;                                               \
        }                                                                       \
        return 0;                                                               \
    }

    using UserUid = std::string;
    using UserNickname = std::string;
    constexpr u16 USERID_MAXSIZE = 129; //64*2+1    

    namespace structs {

        struct UserData {
            AccountUid uid;
            UserNickname nickname;

            bool isValid() const {
                return accountUidIsValid(&uid) && nickname.substr(0, 4) != "ERR#";
            }

            bool operator==(const UserData& other) const {
                return uid.uid[0] == other.uid.uid[0] && uid.uid[1] == other.uid.uid[1];
            }

            void clear() {
                uid.uid[0] = 0;
                uid.uid[1] = 0;
                nickname.clear();
            }
            
        };

    }

    namespace helpers {        
        std::string titleIdToString(u64 titleId);
        UserUid accountUidToString(AccountUid uid);
        AccountUid accountUidFromString(const UserUid& uid);

        structs::UserData getCurrentUser();
        structs::UserData getUserFromAccountUid(AccountUid uid);
        u64 getRunningApplicationPid();
        u64 getRunningApplicationTitleId(u64 process_id);
        std::string getApplicationName(u64 title_id);
        u16 getUserUsageTimeForToday(const AccountUid& uid);

        std::string today();

        //bool shutdown();
        bool rebootToPayload();

        bool terminateCurrentApplication();

        // Blacklist management
        bool isCurrentTitleBlacklisted();
        std::vector<u64> getBlacklistedTitlesForUser(const std::string& userId);
        void addToBlacklist(const std::string& userId, u64 titleId);
        void removeFromBlacklist(const std::string& userId, u64 titleId);

        // Limits management
        u16 getDailyLimitForUser(const std::string& userId);
        void setDailyLimitForUser(const std::string& userId, u16 limit_in_minutes);

        // Authentication management
        std::string encodePassword(const std::vector<u64>&);
        std::vector<u64> decodePassword(const std::string&);
    }
}   
