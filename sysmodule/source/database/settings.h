#pragma once

#include <map>
#include <string>
#include <string_view>
#include "types.h"

using SettingKey = std::string;

typedef enum {
    INTEGER = 0,
    DOUBLE = 1,
    STRING = 2
} SettingType;

struct Setting {
    SettingKey key;
    SettingType type = INTEGER;
    u64 int_value = 0;
    double double_value = 0.0;
    std::string string_value;
    bool encrypted = false;
};

typedef enum {
    WorkingModeInfo = 0,
    WorkingModeBlocking = 1
} WorkingMode;

using Settings = std::map<SettingKey, Setting>;

constexpr const char* SETTING_DAILY_LIMIT_USERS = "daily_limit_users";
constexpr const char* SETTING_ADMIN_PIN = "admin_pin";
constexpr const char* SETTING_WORKING_MODE = "working_mode";
constexpr const char* SETTING_SHOW_REMAINING_TIME = "show_remaining_time";
constexpr const char* SETTING_ENABLED = "enabled";
constexpr const char* SETTING_LOGLEVEL = "log_level";
constexpr const char* SETTING_BLACKLIST = "blacklist";
constexpr const char* SETTING_AUTHENTICATION = "authentication";
