#include "switch.h"
#include "sha256.c"
#include "structs.h"

void generateConsoleKey(uint8_t key[32]) {
    u64 console_id;
    accountInitialize(AccountServiceType_Application);
    accountGetUniqueId(&console_id);
    accountExit();

    sha256((uint8_t*)&console_id, sizeof(console_id), key);
}

UserSession& getCurrentUser() {
    u64 account_id;
    accountInitialize(AccountServiceType_Application);
    accountGetPreselectedUser(&account_id);
    accountExit();

    if (sessions.find(account_id) == sessions.end()) {
        UserSession s;
        s.account_id = account_id;
        s.last_day = 0;
        s.total_daily_seconds = 0;
        s.total_daily_limit = 7200; // 2h par défaut
        sessions[account_id] = s;
    }

    return sessions[account_id];
}

GameSession& getCurrentTitle(UserSession& user) {
    u64 titleID = 0;
    Result rc = appletGetAppletResourceUserId(&titleID);
    
    GameSession g;

    if (R_SUCCEEDED(rc)) {
        printf("Current Title ID: %016llX\n", titleID);

        NsApplicationControlData appdata;
        rc = nsGetApplicationControlData(0, &appdata, sizeof(appdata));
        if (R_SUCCEEDED(rc)) {
            NacpStruct nacp;
            rc = nsGetApplicationControlDataNacp(&appdata, &nacp);
            if (R_SUCCEEDED(rc)) {
                g.game_title = std::string(nacp.lang[0].name);
                printf("Title name: %s\n", nacp.lang[0].name); // Langue 0 = anglais
            }
        }
        
        g.game_id = titleID;
        g.daily_seconds = 0;
        g.daily_limit = 3600; // 1h par défaut
        user.games[g.game_id] = g;
    } else {
        printf("Failed to get Title ID: 0x%x\n", rc);
    }

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