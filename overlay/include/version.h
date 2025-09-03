#pragma once

#include <string>

constexpr const char* VERSION = "1.0\0";

std::string app_version() {
    return std::string("version ") +std::string(VERSION);
}