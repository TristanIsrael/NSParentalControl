#include <switch.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <vector>
#include <map>
#include <ctime>
#include <cstring>

#include "aes.c"
#include "sha256.c"
#include "cipher.cpp"
#include "structs.h"
#include "console.cpp"
#include "database.cpp"
#include "gui.cpp"

std::map<u64, UserSession> sessions;
std::string parentalPIN = "1234"; // PIN par défaut

int main() {
    consoleInit(NULL);
    ensureDataDirectory();

    uint8_t key[32];
    generateConsoleKey(key);

    decryptFile("/switch/NSParentalControl/sessions.txt", key);
    loadDatabase();

    UserSession &user = getCurrentUser();
    GameSession &game = getCurrentGame(user);

    u64 last_tick = armGetSystemTick();

    while (appletMainLoop()) {
        u64 now = armGetSystemTick();
        u64 elapsed_us = armTicksToUs(now - last_tick);
        last_tick = now;

        time_t t_now = time(NULL);
        struct tm t;
        localtime_r(&t_now, &t);
        
        if (t.tm_mday != user.last_day) {
            user.total_daily_seconds = 0;
            for (auto &pair : user.games)
                pair.second.daily_seconds = 0;
            user.last_day = t.tm_mday;
        }

        uint32_t elapsed_seconds = elapsed_us / 1000000;
        game.daily_seconds += elapsed_seconds;
        user.total_daily_seconds += elapsed_seconds;

        if (game.daily_seconds >= game.daily_limit || user.total_daily_seconds >= user.total_daily_limit) {
            consoleClear();
            printf("Temps de jeu épuisé pour aujourd'hui.\n");
            consoleUpdate(NULL);
            svcSleepThread(5000000000ULL);
            break;
        }

        consoleClear();
        printf("Utilisateur: %lu\n", user.account_id);
        printf("Jeu: %s\n", game.game_id.c_str());
        printf("Temps jeu individuel: %u / %u sec\n", game.daily_seconds, game.daily_limit);
        printf("Temps total joué: %u / %u sec\n", user.total_daily_seconds, user.total_daily_limit);
        printf("\nAppuyez sur PLUS pour configurer la limite.\n");
        consoleUpdate(NULL);

        u64 k_down = hidKeysDown(CONTROLLER_P1_AUTO);
        if (k_down & KEY_PLUS) configureLimits(user, game);

        svcSleepThread(50000000ULL);
    }
}