#pragma once
#include <switch.h>
#include <list>
#include <string>

namespace alefbet {
    namespace pctrl {
        namespace helpers {

            using UserUid = std::string;
            using UserNickname = std::string;

            typedef struct {
                AccountUid uid;
                UserNickname nickname;

                bool isValid() const {
                    //return uid.uid[0] > 0 && uid.uid[1] > 0 && !nickname.empty() && nickname.substr(0, 4) != "ERR#";
                    return accountUidIsValid(&uid) && nickname.substr(0, 4) != "ERR#";
                }
            } UserData;

            std::list<UserData> getUsersList();

            UserUid accountUidToString(AccountUid uid);
            AccountUid accountUidFromString(const UserUid& uid);

            std::string getTitleName(u64 titleId);
        }
    }
}