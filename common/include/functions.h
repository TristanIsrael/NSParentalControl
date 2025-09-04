#pragma once

#include <sstream>
#include <string>
#include <switch.h>

std::string accountUidToString(AccountUid& uid);
AccountUid accountUidFromString(std::string& uid);
