#pragma once
#include <string>
#include <vector>
#include <switch.h>

namespace alefbet::pctrl {

    using UserUid = std::string;
    using UserNickname = std::string;
    constexpr u16 USERID_MAXSIZE = 129; //64*2+1    

    namespace structs {

        typedef struct {
            AccountUid uid;
            UserNickname nickname;

            bool isValid() const {
                return accountUidIsValid(&uid) && nickname.substr(0, 4) != "ERR#";
            }
        } UserData;

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
    }
}   
