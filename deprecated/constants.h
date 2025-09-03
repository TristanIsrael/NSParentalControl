#pragma once

#include <switch.h>
#include "tesla.hpp"
#include <string>
#include <map>

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
