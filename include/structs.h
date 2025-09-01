#include <switch.h>
#include <string>
#include <map>

struct GameSession {
    u64 game_id;
    std::string game_title;
    uint32_t daily_seconds;
    uint32_t daily_limit; // limite individuelle par jeu
};

struct UserSession {
    u64 account_id;
    int last_day;
    std::map<std::string, GameSession> games;
    uint32_t total_daily_seconds; // temps total joué aujourd'hui
    uint32_t total_daily_limit;   // limite globale quotidienne
};