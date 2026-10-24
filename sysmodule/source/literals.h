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
#include <switch.h>

constexpr inline u64 operator ""_KB(unsigned long long n) {
    return static_cast<u64>(n) * UINT64_C(1024);
}

constexpr inline u64 operator ""_MB(unsigned long long n) {
    return operator ""_KB(n) * UINT64_C(1024);
}

constexpr inline u64 operator ""_GB(unsigned long long n) {
    return operator ""_MB(n) * UINT64_C(1024);
}