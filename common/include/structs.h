#pragma once

#include <string>
#include <map>
#include <mutex>
#ifndef __NO_SWITCH_H
#include <switch.h>
#include <tesla.hpp>
#else
using u16 = std::uint16_t;
using u32 = std::uint32_t;
using u64 = std::uint64_t;
namespace tsl {
struct Color {
    union {
        struct {
            u16 r: 4, g: 4, b: 4, a: 4;
        } PACKED;
        u16 rgba;
    };
};
}
#endif

typedef enum {
    INTEGER = 0,
    DOUBLE = 1,
    STRING = 2
} SettingType;

typedef enum {
    ALERT_NO_ALERT = 0,
    ALERT_LIMIT_ALMOST_REACHED = 1,
    ALERT_LIMIT_REACHED = 2
} AlertType;

const std::string DATA_DIR = "sdmc:/switch/parental_control";
const std::string DB_FILENAME = DATA_DIR + "/sessions.json";
const std::string SETTINGS_FILENAME = DATA_DIR + "/settings.json";

constexpr std::string_view SETTING_DAILY_LIMIT_GAME = "daily_limit_game";
constexpr std::string_view SETTING_DAILY_LIMIT_GLOBAL = "daily_limit_global";
constexpr std::string_view SETTING_SHMEM_HANDLE = "shmem_handle";
using UserId = std::string;
using UserNickName = std::string;

constexpr tsl::Color ColorError = { 0xF, 0x0, 0x0, 0xF };

struct GameSession {
    u64 game_id;
    std::string game_title;
    u64 daily_seconds;
};

struct UserSession {
    std::string account_id;
    std::string profile_name;
    int last_day;
    std::map<u64, GameSession> games;
    u64 total_daily_seconds; // temps total joué aujourd'hui
};

using SettingKey = std::string_view;
struct Setting {
    SettingKey key;
    SettingType type = INTEGER;
    u64 int_value = 0;
    double double_value = 0.0;
    std::string string_value;
};

typedef struct {
    bool active;   // System state 
    AlertType alert_type;    // Alert type
    bool acknowledged;     

    u64 today_time_remaining_in_secs;
    u64 today_global_usage_in_secs;    
} ParentalControlState;

//inline std::atomic<ParentalControlState> pcState;
inline std::mutex pcMutex;
inline ParentalControlState pcState;

inline void initParentalControlState() {
    std::lock_guard<std::mutex> lock(pcMutex);
    pcState.active = false;
    pcState.alert_type = ALERT_NO_ALERT;
    pcState.acknowledged = false;
    pcState.today_global_usage_in_secs = 0;
    pcState.today_time_remaining_in_secs = 0;    
}

inline ParentalControlState getParentalControlState() {
    std::lock_guard<std::mutex> lock(pcMutex);
    return pcState;
}

inline void setParentalControlState(ParentalControlState& state) {
    std::lock_guard<std::mutex> lock(pcMutex);
    pcState = state;
}

using Settings = std::map<SettingKey, Setting>;
using UserSessions = std::map<std::string, UserSession>;

inline void initState(ParentalControlState* state) {
    std::memset(state, 0, sizeof(ParentalControlState));

    state->active = false;
    state->alert_type = ALERT_NO_ALERT;
    state->acknowledged = false;
    state->today_global_usage_in_secs = 0;
    state->today_time_remaining_in_secs = 0;    
}

constexpr int WARNING_DELAY = 5*60;

typedef enum {
    TimeLimitOk = 0,
    TimeLimitWarning = 1,
    TimeLimitReached = 2
} TimeLimitState;