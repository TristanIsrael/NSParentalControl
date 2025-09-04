#include <switch.h>
#include "console.h"
#include "structs.h"
#include "functions.h"

UserSession getCurrentUser(UserSessions& sessions) {
    AccountUid account_id;
    accountInitialize(AccountServiceType_Application);
    accountGetPreselectedUser(&account_id);
    accountExit();

    std::string uid = accountUidToString(account_id);

    if (sessions.find(uid) == sessions.end()) {
        UserSession s;
        s.account_id = uid;
        s.last_day = 0;
        s.total_daily_seconds = 0;
        sessions[uid] = s;
    }

    return sessions[uid];
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