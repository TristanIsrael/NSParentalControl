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

#include "literals.h"

namespace ams {
    /* This comes from Atmosphère */

    namespace os {
        constexpr inline size_t MemoryPageSize = 4_KB;
        constexpr inline size_t MemoryHeapUnitSize  = 2_MB;
    }

    namespace util {
        template<typename T>
        constexpr inline T AlignUp(T value, size_t alignment) {
            using U = typename std::make_unsigned<T>::type;
            const U invmask = static_cast<U>(alignment - 1);
            return static_cast<T>((value + invmask) & ~invmask);
        }
    }
}
