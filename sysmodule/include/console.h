#pragma once

#include "structs.h"

UserSession getCurrentUser(UserSessions& sessions);
GameSession getCurrentGame(UserSession& user);
