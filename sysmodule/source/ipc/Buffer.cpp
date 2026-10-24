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

#include <cstring>
#include "Buffer.hpp"

namespace Utils::Buffer {
    bool appendString(std::vector<uint8_t> & buf, const std::string & str) {
        // Iterate over string
        for (size_t i = 0; i < str.size(); i++) {
            buf.push_back(str[i]);
        }
        buf.push_back('\0');
        return true;
    }

    bool readString(const std::vector<uint8_t> & buf, size_t & pos, std::string & str) {
        if (pos >= buf.size()) {
            return false;
        }

        str = std::string(reinterpret_cast<const char *>(&buf[pos]));
        pos += str.length() + 1;
        return true;
    }
};