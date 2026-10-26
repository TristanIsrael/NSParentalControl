/*
 * NS Parental Control
 * Copyright (C) 2025 Tristan Israël
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#pragma once

#include <map>
#include <string>
#include <string_view>
#include <switch.h>

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
constexpr const char* SETTING_BEDTIME_USERS = "bedtime_users";
constexpr const char* SETTING_GAME_START_USERS = "game_start_users";
