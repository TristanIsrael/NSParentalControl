#pragma once
#include <string>
#include <switch.h>

namespace alefbet::pctrl {

    using UserUid = std::string;
    using UserNickname = std::string;

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

        bool isCurrentTitleBlacklisted();
    }
}   
