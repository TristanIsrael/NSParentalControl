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
#include <charconv>

namespace alefbet::pctrl::core {

    std::string formatTimeInMinutes(u16 minutesSinceMidnight) {
        const u16 hours = minutesSinceMidnight / 60;
        const u16 minutes = minutesSinceMidnight % 60;

        char buffer[12] = {0};
        std::snprintf(buffer, sizeof(buffer), "%02d:%02d", hours, minutes);

        return std::string(buffer);
    }

    AdjustedTimeResult computeAdjustedTime(u16 currentTime, u16 lastTime, s64 elapsedMinutes, s32& offset) {
        constexpr s64 MinutesPerDay = 1440;
        s32 jump = 0;

        if(elapsedMinutes != kElapsedUnknown) {
            const s64 expectedTime = (static_cast<s64>(lastTime) + elapsedMinutes) % MinutesPerDay;

            // Difference between the current and the expected time, in (-720, 720]
            s64 diff = (static_cast<s64>(currentTime) - expectedTime) % MinutesPerDay;
            if(diff > MinutesPerDay / 2) diff -= MinutesPerDay;
            if(diff <= -MinutesPerDay / 2) diff += MinutesPerDay;

            if(std::abs(diff) > kTimeJumpThreshold) {
                jump = static_cast<s32>(diff);
                offset -= jump;
            }
        }

        const s32 adjusted = static_cast<s32>(currentTime) + offset;

        // Wrap in [0, 1440) whatever the accumulated offset
        s32 result = adjusted % static_cast<s32>(MinutesPerDay);
        if(result < 0) result += static_cast<s32>(MinutesPerDay);

        return {static_cast<u16>(result), jump};
    }

    void resetAdjustedTimeState(u16& lastTime, s32& offset) {
        lastTime = 0;
        offset = 0;
    }

    bool isInsidePlayWindow(u16 time, u16 start, u16 end) {
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

    std::string serializeUserValues(const std::map<std::string, u16>& values) {
        std::ostringstream oss;
        bool first = true;
        for(const auto& [userId, value] : values) {
            if(!first) oss << ';';
            first = false;
            oss << userId << '=' << value;
        }
        return oss.str();
    }

    std::map<std::string, u16> deserializeUserValues(const std::string& data) {
        std::map<std::string, u16> result;
        std::istringstream stream(data);
        std::string line;
        while(std::getline(stream, line, ';')) {
            if(line.empty()) continue;
            auto eqPos = line.find('=');
            if(eqPos == std::string::npos) continue;
            std::string userId = line.substr(0, eqPos);
            u16 value = static_cast<u16>(std::stoul(line.substr(eqPos + 1)));
            result[userId] = value;
        }
        return result;
    }

    std::string serializeUserTitles(const std::map<std::string, std::vector<u64>>& titles) {
        std::ostringstream oss;
        bool firstUser = true;
        for(const auto& [userId, titleIds] : titles) {
            if(titleIds.empty()) continue;
            if(!firstUser) oss << ';';
            firstUser = false;
            oss << userId << '=';
            bool firstTitle = true;
            for(u64 titleId : titleIds) {
                if(!firstTitle) oss << ',';
                firstTitle = false;
                oss << titleId;
            }
        }
        return oss.str();
    }

    std::map<std::string, std::vector<u64>> deserializeUserTitles(const std::string& data) {
        std::map<std::string, std::vector<u64>> result;
        std::istringstream stream(data);
        std::string line;
        while(std::getline(stream, line, ';')) {
            if(line.empty()) continue;
            auto eqPos = line.find('=');
            if(eqPos == std::string::npos) continue;
            std::string userId = line.substr(0, eqPos);
            std::istringstream titleStream(line.substr(eqPos + 1));
            std::string titlePart;
            while(std::getline(titleStream, titlePart, ',')) {
                // Exceptions are disabled on the Switch: parse without std::stoull
                u64 titleId = 0;
                const auto [end, ec] = std::from_chars(titlePart.data(), titlePart.data() + titlePart.size(), titleId);
                if(ec == std::errc() && end == titlePart.data() + titlePart.size() && titleId != 0) {
                    result[userId].push_back(titleId);
                }
            }
        }
        return result;
    }

    bool shouldSendNotification(u16 remainingTimeInMinutes) {
        // Remaining time notifications are sent every 15 minutes and every minute during the last 5 minutes
        return (remainingTimeInMinutes % 15) == 0 || remainingTimeInMinutes <= 5;
    }

    PlayRuleEvaluation evaluatePlayRules(u16 dailyLimit,
                                         u16 totalDurationToday,
                                         u16 adjustedTimeMinutes,
                                         u16 gameStartTime,
                                         u16 bedtime,
                                         bool showRemainingTime,
                                         bool titleBlocked) {
        PlayRuleEvaluation result{};

        result.remainingTimeInMinutes = (dailyLimit > totalDurationToday) ? (dailyLimit - totalDurationToday) : 0;

        result.outsidePlayWindow = (gameStartTime > 0 || bedtime > 0) &&
                                   !isInsidePlayWindow(adjustedTimeMinutes, gameStartTime, bedtime);

        result.titleBlocked = titleBlocked;

        result.shouldBlock = titleBlocked || !(result.remainingTimeInMinutes > 0 && !result.outsidePlayWindow);

        result.shouldNotify = !result.shouldBlock &&
                              showRemainingTime &&
                              shouldSendNotification(result.remainingTimeInMinutes);

        return result;
    }

}
