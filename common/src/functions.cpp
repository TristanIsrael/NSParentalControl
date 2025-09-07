#include <sstream>
#include <string>
#include <switch.h>
#include "functions.h"
#include "database.h"

std::string accountUidToString(AccountUid& uid) {
    std::stringstream st;

    st << uid.uid[0]<< " " << uid.uid[1];

    return st.str();
}

AccountUid accountUidFromString(std::string& uid) {
    AccountUid u;
    std::stringstream st(uid);

    st >> u.uid[0] >> u.uid[1];

    return u;
}

bool isParentalControlInitialized()
{
    return dataDirectoryExists() && dataFileExists();
}

bool isParentalControlActive()
{
    return false;
}

void initializeParentalControl()
{
    createDataDirectory();

    UserSessions sessions;
    saveDatabase(sessions);

    Settings settings;
    Setting version;
    version.key = "app_version";
    version.type = STRING;
    version.string_value = "1.0.2";
    saveSettings(settings, version);
}