#include <switch.h>
#include <ctime>
#include "algo.h"
#include "structs.h"
#include "database.h"

u64 last_tick(0);

/*! 
\brief Verify whether the time limit is reached for the user provided in arguments.
\return true when the time limit is reached.
*/
TimeLimitState verifyTimeLimit(UserSession& user, GameSession& game, UserSessions& sessions, Settings& settings) {
    //Verify time limit
    u64 now = armGetSystemTick();
    u64 freq = armGetSystemTickFreq();
    u64 us = (now * 1000000) / freq;
    u64 elapsed_us = us - last_tick;
    last_tick = us;

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

    if (game.daily_seconds >= settings[SETTING_DAILY_LIMIT_GAME].int_value || user.total_daily_seconds >= settings[SETTING_DAILY_LIMIT_GLOBAL].int_value) {
        printf("Time exhausted.\n");                
        return TimeLimitReached;
    } else {
        // After verifying the limit, the user struct has been updated
        // We verify whether the limit is approaching
        if(user.total_daily_seconds >= (settings[SETTING_DAILY_LIMIT_GLOBAL].int_value - WARNING_DELAY)) {
            printf("Time limit is approaching\n");
            return TimeLimitWarning;
        } else {
            return TimeLimitOk;
        }
    }

    return TimeLimitReached;
}