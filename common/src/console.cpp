#include <switch.h>
#include "console.h"
#include "structs.h"
#include "functions.h"

std::map<UserId, UserNickName> getUsersList()
{
    std::map<UserId, UserNickName> users;

    if (R_SUCCEEDED(accountInitialize(AccountServiceType_Application))) {
        AccountUid uids[ACC_USER_LIST_SIZE];
        s32 count = 0;

        if (R_SUCCEEDED(accountListAllUsers(uids, ACC_USER_LIST_SIZE, &count))) {
            for (s32 i = 0; i < count; i++) {
                AccountProfile profile;
                AccountUid uid = uids[i];

                if (R_SUCCEEDED(accountGetProfile(&profile, uid))) {
                    AccountProfileBase base;
                    AccountUserData user_data;
                    accountProfileGet(&profile, &user_data, &base);                    

                    UserId user_id = accountUidToString(uid);
                    users[user_id] = std::string(base.nickname);

                    accountProfileClose(&profile);
                }
            }
        } else {
            users["ERR#0002"] = "ERR#0002";
        }

        accountExit();
    } else {
        users["ERR#0001"] = "ERR#0001";
    }
    
    return users;
}

UserSession getCurrentUser() {
    AccountUid account_id;
    Result rc;
    UserSession user;

    rc = accountInitialize(AccountServiceType_Application);
    if(R_FAILED(rc)) {
        user.profile_name = std::format("ERR#0001 (0x{:x})", rc);
        return user;
    }

    rc = accountGetPreselectedUser(&account_id);
    if(R_FAILED(rc)) {
        user.profile_name = std::format("ERR#0002 (0x{:x})", rc);
        return user;
    }
    
    std::string uid = accountUidToString(account_id);
    user.account_id = uid;

    // Get profile info
    AccountProfile profile;

    rc = accountGetProfile(&profile, account_id);
    if(R_FAILED(rc)) {
        user.profile_name = std::format("ERR#0003 (0x{:x})", rc);
        return user;
    }
    AccountProfileBase base;
    AccountUserData user_data;
    accountProfileGet(&profile, &user_data, &base);
    user.profile_name = std::string(base.nickname);
    accountProfileClose(&profile);    

    /*if (sessions.find(uid) == sessions.end()) {
        s.account_id = uid;
        s.profile_name = profile_name;
        s.last_day = 0;
        s.total_daily_seconds = 0;
        sessions[uid] = s;
    }

    return sessions[uid];*/
    accountExit();

    return user;
}

GameSession getCurrentGame(UserSession& user) {
    u64 appId = appletGetAppletResourceUserId();
    
    GameSession g;
    printf("Current App ID: %016llX\n", appId);

    if(appId == 0) {
        printf("No game loaded.");
        return g;
    }

    NsApplicationControlData appdata;
    u64 data_size = 0;
    Result rc = nsGetApplicationControlData(::NsApplicationControlSource_Storage, appId, &appdata, sizeof(appdata), &data_size);

    if (R_SUCCEEDED(rc)) {        
        NacpStruct nacp = appdata.nacp;
        g.game_title = std::string(nacp.lang[0].name);
        printf("Title name: %s\n", nacp.lang[0].name); // Langue 0 = anglais
    }
    
    g.game_id = appId;
    g.daily_seconds = 0;
    user.games[g.game_id] = g;

    return g;
}

/*GameSession& getCurrentGame(UserSession &user, const std::string &game_id) {
    if (user.games.find(game_id) == user.games.end()) {
        GameSession g;
        g.game_id = game_id;
        g.daily_seconds = 0;
        g.daily_limit = 3600; // 1h par défaut
        user.games[g.game_id] = g;
    }

    return user.games[game_id];
}*/