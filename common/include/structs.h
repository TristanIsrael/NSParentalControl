#pragma once

#include <string>
#include <map>
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

constexpr std::string_view SETTING_DAILY_LIMIT_GAME = "daily_limit_game";
constexpr std::string_view SETTING_DAILY_LIMIT_GLOBAL = "daily_limit_global";
constexpr std::string_view SETTING_SHMEM_HANDLE = "shmem_handle";

constexpr tsl::Color ColorError = { 0xF, 0x0, 0x0, 0xF };

struct GameSession {
    u64 game_id;
    std::string game_title;
    uint32_t daily_seconds;
};

struct UserSession {
    std::string account_id;
    std::string profile_name;
    int last_day;
    std::map<u64, GameSession> games;
    uint32_t total_daily_seconds; // temps total joué aujourd'hui
};

using SettingKey = std::string_view;
struct Setting {
    SettingKey key;
    SettingType type = INTEGER;
    int int_value = 0;
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

inline std::atomic<ParentalControlState> pcState;

inline void initParentalControlState() {
    ParentalControlState state = pcState.load();
    state.active = false;
    state.alert_type = ALERT_NO_ALERT;
    state.acknowledged = false;
    state.today_global_usage_in_secs = 0;
    state.today_time_remaining_in_secs = 0;
    pcState.store(state);
}

inline ParentalControlState getParentalControlState() {
    return pcState.load();
}

inline void setParentalControlState(ParentalControlState& state) {
    return pcState.store(state);
}

using Settings = std::map<SettingKey, Setting>;
using UserSessions = std::map<std::string, UserSession>;

void initState(ParentalControlState* state) {
    std::memset(state, 0, sizeof(ParentalControlState));

    state->active = false;
    state->alert_type = ALERT_NO_ALERT;
    state->acknowledged = false;
    state->today_global_usage_in_secs = 0;
    state->today_time_remaining_in_secs = 0;    
}

