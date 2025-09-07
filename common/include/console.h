#pragma once

#include <list>
#include <switch.h>
#include "structs.h"

std::map<UserId, UserNickName> getUsersList();
UserSession getCurrentUser();
GameSession getCurrentGame(UserSession& user);
