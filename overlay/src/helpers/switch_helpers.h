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