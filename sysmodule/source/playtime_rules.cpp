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

#include "playtime_rules.h"
#include <cstdio>
#include <cstdlib>
#include <sstream>

namespace alefbet::pctrl::core {

    std::string formatTimeInMinutes(uint16_t minutesSinceMidnight) {
        const uint16_t hours = minutesSinceMidnight / 60;
        const uint16_t minutes = minutesSinceMidnight % 60;

        char buffer[12] = {0};
        std::snprintf(buffer, sizeof(buffer), "%02d:%02d", hours, minutes);

        return std::string(buffer);
    }

    AdjustedTimeResult computeAdjustedTime(uint16_t currentTime, uint16_t lastTime, int32_t& offset) {
        int32_t jump = 0;

        if(lastTime != 0) {
            const int32_t apparentDiff = static_cast<int32_t>(currentTime) - static_cast<int32_t>(lastTime);

            if(std::abs(apparentDiff) > kTimeJumpThreshold) {
                jump = apparentDiff;
                offset -= apparentDiff;
            }
        }

        const int32_t adjusted = static_cast<int32_t>(currentTime) + offset;

        uint16_t result;
        if(adjusted < 0) {
            result = static_cast<uint16_t>(1440 + adjusted);
        } else if(adjusted >= 1440) {
            result = static_cast<uint16_t>(adjusted - 1440);
        } else {
            result = static_cast<uint16_t>(adjusted);
        }

        return {result, jump};
    }

    void resetAdjustedTimeState(uint16_t& lastTime, int32_t& offset) {
        lastTime = 0;
        offset = 0;
    }

    bool isInsidePlayWindow(uint16_t time, uint16_t start, uint16_t end) {
        if(start == 0 && end == 0) {
            return true;
        }

        if(start <= end) {
            const bool beforeStart = start > 0 && time < start;
            const bool afterEnd = end > 0 && time >= end;
            return !beforeStart && !afterEnd;
        }

        // Window crossing midnight: [start, 24:00) ∪ [00:00, end)
        return time >= start || time < end;
    }

    std::string serializeUserValues(const std::map<std::string, uint16_t>& values) {
        std::ostringstream oss;
        bool first = true;
        for(const auto& [userId, value] : values) {
            if(!first) oss << ';';
            first = false;
            oss << userId << '=' << value;
        }
        return oss.str();
    }

    std::map<std::string, uint16_t> deserializeUserValues(const std::string& data) {
        std::map<std::string, uint16_t> result;
        std::istringstream stream(data);
        std::string line;
        while(std::getline(stream, line, ';')) {
            if(line.empty()) continue;
            auto eqPos = line.find('=');
            if(eqPos == std::string::npos) continue;
            std::string userId = line.substr(0, eqPos);
            uint16_t value = static_cast<uint16_t>(std::stoul(line.substr(eqPos + 1)));
            result[userId] = value;
        }
        return result;
    }

}
