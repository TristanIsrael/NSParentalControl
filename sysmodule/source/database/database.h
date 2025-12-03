#pragma once

#include <map>
#include <switch.h>
#include "settings.h"
#include "history.h"
#include "helpers.h"

using namespace alefbet::pctrl::structs;

namespace alefbet::pctrl::database {

    /* Files management */
    /*bool dataDirectoryExists();
    bool dataFileExists();
    bool settingsFileExists();*/
    bool createDataDirectory();

    /* Data management */
    std::vector<HistoryEntry> getHistory(AccountUid uid, std::string date);
    HistoryEntry addToHistory(AccountUid uid, u64 titleId, u16 duration_in_minutes);

    /* Settings management */
    Settings& loadSettings();
    void saveSettings();
    void saveSetting(Setting setting);

    bool upgradeNeeded();
    bool isTampered();

    /* Authentication management */
    using Password = std::string;
    using Passwords = std::map<UserUid, Password>;

    Passwords& loadPasswords();
    void savePassword(UserUid account, Password password);
}