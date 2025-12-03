#pragma once

#include <string>

namespace Ipc {
    enum class Command {
        Version,                    /*! Queries the server version. Result: the version as a string. */
        Test,                       /*! Triggers a test of the service. Result: none. */
        Initialize,                 /*! Triggers the initialization of the system. */
        GetUserUsageTime,           /*! Queries a user's usage data. Returns a data structure with the information of the user. */
        GetUserRemainingTime,       /*! Queries a user's remaining time. Returns the remaining time in minutes as an integer. */
        GetCurrentUserUid,          /*! Returns the current profile Uid as a string. */
        GetCurrentUserNickname,     /*! Returns the current profile label as a string. */
        GetRunningApplication,      /*! Returns the currently running application title as an integer. */
        SetUserDailyLimit,          /*! Sets a users limits. */
        SetAdminPin,                /*! Sets the administrator PIN. */
        VerifyAdminPin,             /*! Verifies the PIN entered by the admin. */  
        SetWorkingMode,             /*! Sets the working mode (information, blocking) */   
        GetWorkingMode,             /*! Returns the current working mode as an integer. */
        SetShowRemainingTime,       /*! Activate the remaining time panel in the upper right corner */
        GetShowRemainingTime,       /*! Returns the current state of the remaining time panel setting. Returns 1 if the panel is set to be visible. */
        IsEnabled,                  /*! Returns the current state of the parental control service. Returns 1 if the service is enabled. */
        SetEnabled,                 /*! Sets the state of the parental control service. 1 to enable the service. 0 to disable. */
        SetLogLevel,                /*! Sets the log level */
        GetLogLevel,                /*! Returns the current log level */
        IsTampered,                 /*! Returns true if the database or the settings have been modified externally */
        MustUpgradeDatabase,        /*! Returns true if the database must be upgraded */
        GetBlacklistedTitlesCount,  /*! Returns the number of blacklisted titles for a user */
        GetBlacklistedTitle,        /*! Returns the blacklisted title ID at index (starting at 0) for a user */
        AddTitleToBlacklist,        /*! Adds a title to a users's blacklist */
        RemoveTitleFromBlacklist,   /*! Removes a title from a users's blacklist */
        GetUserDailyLimit,          /*! Gets a users limits. */
        SetAuthenticationActive,    /*! Sets the authentication active or not */
        IsAuthenticationActive      /*! Returns true if authentication is enabled */
    };

};
