#pragma once

#include <map>
#include <switch.h>
#include "structs.h"
#include "json.hpp"

void ensureDataDirectory();
void loadDatabase(UserSessions& sessions);
void saveDatabase(UserSessions& sessions);
Settings loadSettings();
void saveSettings(Settings& settings, Setting& setting);
