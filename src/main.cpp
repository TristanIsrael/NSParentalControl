#include <switch.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <vector>
#include <map>
#include <ctime>
#include <cstring>

#include "aes.c"
#include "sha256.c"
#include "structs.h"
#include "console.h"
#include "database.h"
#include "gui.h"

const bool CIPHER_DATABASE = false;
const char* KEY_FILE = "save:/console_key.bin";
std::string parentalPIN = "1234"; // PIN par défaut
UserSessions sessions;
Settings settings;

int main() {
    consoleInit(NULL);
    ensureDataDirectory();
    
    srand(time(nullptr));

    /*const size_t KEY_LEN = 32;
    uint8_t key[KEY_LEN];

    std::ifstream infile(KEY_FILE, std::ios::binary);
    if (infile.good()) {
        infile.read(reinterpret_cast<char*>(key), KEY_LEN);
        infile.close();
        printf("Loaded existing key.\n");
    } else {
        generateKey(key, KEY_LEN);
        std::ofstream outfile(KEY_FILE, std::ios::binary);
        outfile.write(reinterpret_cast<char*>(key), KEY_LEN);
        outfile.close();
        printf("Generated new console key.\n");
    }

    decryptFile("/switch/NSParentalControl/sessions.txt", key, CIPHER_DATABASE);*/

    UserSession user = getCurrentUser(sessions);
    loadDatabase(sessions); 

    GameSession game = getCurrentGame(user);

    u64 last_tick = armGetSystemTick();

    while (appletMainLoop()) {
        u64 now = armGetSystemTick();
        u64 freq = armGetSystemTickFreq();
        u64 us = (now * 1000000) / freq;
        u64 elapsed_us = now - last_tick;
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

        saveDatabase(sessions);

        if (game.daily_seconds >= settings[DAILY_LIMIT_GAME].int_value || user.total_daily_seconds >= settings[DAILY_LIMIT_GLOBAL].int_value) {
            consoleClear();
            printf("Temps de jeu épuisé pour aujourd'hui.\n");
            consoleUpdate(NULL);
            svcSleepThread(5000000000ULL);
            break;
        }

        consoleClear();
        printf("Utilisateur: %lu\n", user.account_id);
        printf("Jeu: %s\n", game.game_title.c_str());
        printf("Temps jeu individuel: %u / %u sec\n", game.daily_seconds, settings[DAILY_LIMIT_GAME].int_value);
        printf("Temps total joué: %u / %u sec\n", user.total_daily_seconds, settings[DAILY_LIMIT_GLOBAL].int_value);
        printf("\nAppuyez sur PLUS pour configurer la limite.\n");
        consoleUpdate(NULL);

        //u64 k_down = hidKeysDown(CONTROLLER_P1_AUTO);
        //if (k_down & KEY_PLUS) configureLimits(user, game);

        svcSleepThread(50000000ULL);
    }
}