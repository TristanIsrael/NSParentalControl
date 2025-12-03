#pragma once

#include <string>

namespace Ipc {
    enum class Command {
        Version,                    /*! Queries the server version. Result: the version as a string. */
        Test,                       /*! Triggers a test of the service. Result: none. */
        Initialize,                 /*! Triggers the initialization of the system. */
        GetUsersList,               /*! DEPRECATED - Queries the current users list. Returns a list of strings containing the users names registered on the system. */
        GetUserUsageTime,           /*! Queries a user's usage data. Returns a data structure with the information of the user. */
        GetUserRemainingTime,       /*! Queries a user's remaining time. Returns the remaining time in minutes as an integer. */
        GetCurrentUserUid,          /*! Returns the current profile Uid as a string. */
        GetCurrentUserNickname,     /*! Returns the current profile label as a string. */
        GetRunningApplication,      /*! Returns the currently running application title as an integer. */
        SetUserLimits,              /*! Sets a users limits. */
        SetAdminPin,                /*! Sets the administrator PIN. */
        VerifyAdminPin,             /*! Verifies the PIN entered by the admin. */  
        SetWorkingMode,             /*! Sets the working mode (information, blocking) */   
        GetWorkingMode,             /*! Returns the current working mode as an integer. */
        SetShowRemainingTime,       /*! Activate the remaining time panel in the upper right corner */
        GetShowRemainingTime,       /*! Returns the current state of the remaining time panel setting. Returns 1 if the panel is set to be visible. */
        IsEnabled,                  /*! Returns the current state of the parental control service. Returns 1 if the service is enabled. */
        SetEnabled,                 /*! Sets the state of the parental control service. 1 to enable the service. 0 to disable. */
        GetDailyLimit,              /*! Returns the current setting of the daily limit in minutes. */
        SetDailyLimit,              /*! Sets the daily limit in minutes. */
        SetLogLevel,                /*! Sets the log level */
        GetLogLevel,                /*! Returns the current log level */
        IsTampered,                 /*! Returns true if the database or the settings have been modified externally */
        MustUpgradeDatabase,        /*! Returns true if the database must be upgraded */
        GetBlacklistedTitlesCount,  /*! Returns the number of blacklisted titles for a user */
        GetBlacklistedTitle,        /*! Returns the blacklisted title ID at index (starting at 0) for a user */
        AddTitleToBlacklist,        /*! Adds a title to a users's blacklist */
        RemoveTitleFromBlacklist    /*! Removes a title from a users's blacklist */
    };


    std::string commandToString(const Command& cmd) {
        switch(cmd) {
            case Command::Version: return "Version";
            case Command::Test: return "Test";
            case Command::Initialize: return "Initialize";
            case Command::GetUsersList: return "GetUsersList";
            case Command::GetUserUsageTime: return "GetUserUsageTime";
            case Command::GetUserRemainingTime: return "GetUserRemainingTime";
            case Command::GetCurrentUserUid: return "GetCurrentUserUid";
            case Command::GetCurrentUserNickname: return "GetCurrentUserNickname";
            case Command::GetRunningApplication: return "GetRunningApplication";
            case Command::SetUserLimits: return "SetUserLimits";
            case Command::SetAdminPin: return "SetAdminPin";
            case Command::VerifyAdminPin: return "VerifyAdminPin";
            case Command::SetWorkingMode: return "SetWorkingMode";
            case Command::GetWorkingMode: return "GetWorkingMode";
            case Command::SetShowRemainingTime: return "SetShowRemainingTime";
            case Command::GetShowRemainingTime: return "GetShowRemainingTime";
            case Command::IsEnabled: return "IsEnabled";
            case Command::SetEnabled: return "SetEnabled";
            case Command::GetDailyLimit: return "GetDailyLimit";
            case Command::SetDailyLimit: return "SetDailyLimit";
            case Command::SetLogLevel: return "SetLogLevel";
            case Command::GetLogLevel: return "GetLogLevel";
            case Command::IsTampered: return "IsTampered";
            case Command::MustUpgradeDatabase: return "MustUpgradeDatabase";
            case Command::GetBlacklistedTitlesCount: return "GetBlacklistedTitlesCount";
            case Command::GetBlacklistedTitle: return "GetBlacklistedTitle";
            case Command::AddTitleToBlacklist: return "AddTitleToBlacklist";
            case Command::RemoveTitleFromBlacklist: return "RemoveTitleFromBlacklist";
            default: return "Unknown";
        }
    };
};
