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

#include <cstdint>
#include <map>
#include <string>

namespace alefbet::pctrl::core {

    /*! @brief Jump (in minutes) above which a system time change is considered a tampering attempt. */
    constexpr int32_t kTimeJumpThreshold = 5;

    struct AdjustedTimeResult {
        uint16_t time;    ///< Adjusted time in minutes since midnight (0-1439).
        int32_t jump = 0; ///< The detected jump in minutes, 0 if no jump was detected.
    };

    /*! @brief Formats minutes since midnight as an HH:mm string.
      *
      * @param minutesSinceMidnight Minutes since midnight (0-1439).
      * @return Formatted time string (e.g. "21:30").
      */
    std::string formatTimeInMinutes(uint16_t minutesSinceMidnight);

    /*! @brief Computes the tamper-protected time.
      *
      * When the difference between the current time and the last observed time is
      * larger than kTimeJumpThreshold, a system time jump is assumed and the offset
      * is updated so that the returned (adjusted) time keeps progressing normally.
      *
      * @param currentTime The current time in minutes since midnight (0-1439).
      * @param lastTime The time observed at the previous iteration (0 on the first iteration).
      * @param offset Cumulative compensation offset, updated in place.
      * @return The adjusted time and the detected jump (0 if none).
      */
    AdjustedTimeResult computeAdjustedTime(uint16_t currentTime, uint16_t lastTime, int32_t& offset);

    /*! @brief Resets the adjusted-time state (e.g. when the sysmodule starts).
      *
      * @param lastTime Last observed time, reset to 0.
      * @param offset Cumulative compensation offset, reset to 0.
      */
    void resetAdjustedTimeState(uint16_t& lastTime, int32_t& offset);

    /*! @brief Checks whether a time is inside the playtime window.
      *
      * The window is defined by its start and its end, both in minutes since
      * midnight. A bound of 0 means the bound is disabled (unlimited on that side).
      * When both bounds are 0, the window is unrestricted.
      *
      * | start | end   | allowed period                          |
      * |-------|-------|-----------------------------------------|
      * | 0     | 0     | all day                                 |
      * | 0     | B     | [00:00, B)                              |
      * | S     | 0     | [S, 24:00)                              |
      * | S < B |       | [S, B)                                  |
      * | S = B |       | never (empty window)                    |
      * | S > B |       | [S, 24:00) ∪ [00:00, B) (midnight)      |
      *
      * The start bound is inclusive, the end bound is exclusive.
      *
      * @param time Time to check, in minutes since midnight (0-1439).
      * @param start Window start in minutes since midnight (0 = disabled).
      * @param end Window end in minutes since midnight (0 = disabled).
      * @return true if the time is inside the window, false otherwise.
      */
    bool isInsidePlayWindow(uint16_t time, uint16_t start, uint16_t end);

    /*! @brief Serializes per-user values as a "uid=value;uid=value" string.
      *
      * @param values Map of user ID to value.
      * @return The serialized string (empty if the map is empty).
      */
    std::string serializeUserValues(const std::map<std::string, uint16_t>& values);

    /*! @brief Deserializes a "uid=value;uid=value" string into a map of user values.
      *
      * Empty segments and segments without an '=' separator are ignored.
      *
      * @param data The serialized string.
      * @return The deserialized map.
      */
    std::map<std::string, uint16_t> deserializeUserValues(const std::string& data);

}
