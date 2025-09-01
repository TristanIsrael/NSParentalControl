#pragma once

#include <switch.h>
#include <string>
#include <map>

struct GameSession {
    u64 game_id;
    std::string game_title;
    uint32_t daily_seconds;
};

struct UserSession {
    std::string account_id;
    int last_day;
    std::map<u64, GameSession> games;
    uint32_t total_daily_seconds; // temps total joué aujourd'hui
};

typedef enum {
    INTEGER = 0,
    DOUBLE = 1,
    STRING = 2
} SettingType;

struct Setting {
    SettingKey key;
    SettingType type = INTEGER;
    int int_value = 0;
    double double_value = 0.0;
    std::string string_value;
};

constexpr std::string_view DAILY_LIMIT_GAME = "daily_limit_game";
constexpr std::string_view DAILY_LIMIT_GLOBAL = "daily_limit_global";

using SettingKey = std::string_view;
using Settings = std::map<SettingKey, Setting>;
using UserSessions = std::map<std::string, UserSession>;