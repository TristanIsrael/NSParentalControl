#pragma once

#include <map>
#include <switch.h>
#include "structs.h"
#include "json.hpp"

bool dataDirectoryExists();
bool dataFileExists();
bool settingsFileExists();

bool createDataDirectory();
void loadDatabase(UserSessions& sessions);
void saveDatabase(UserSessions& sessions);
Settings loadSettings();
void saveSettings(Settings& settings, Setting& setting);
