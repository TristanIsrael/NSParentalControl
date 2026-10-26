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
#include <switch/types.h>

namespace alefbet::pctrl::core {

    /*! @brief Jump (in minutes) above which a system time change is considered a tampering attempt. */
    constexpr s32 kTimeJumpThreshold = 5;

    struct AdjustedTimeResult {
        u16 time;    ///< Adjusted time in minutes since midnight (0-1439).
        s32 jump = 0; ///< The detected jump in minutes, 0 if no jump was detected.
    };

    /*! @brief Formats minutes since midnight as an HH:mm string.
      *
      * @param minutesSinceMidnight Minutes since midnight (0-1439).
      * @return Formatted time string (e.g. "21:30").
      */
    std::string formatTimeInMinutes(u16 minutesSinceMidnight);

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
    AdjustedTimeResult computeAdjustedTime(u16 currentTime, u16 lastTime, s32& offset);

    /*! @brief Resets the adjusted-time state (e.g. when the sysmodule starts).
      *
      * @param lastTime Last observed time, reset to 0.
      * @param offset Cumulative compensation offset, reset to 0.
      */
    void resetAdjustedTimeState(u16& lastTime, s32& offset);

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
    bool isInsidePlayWindow(u16 time, u16 start, u16 end);

    /*! @brief Serializes per-user values as a "uid=value;uid=value" string.
      *
      * @param values Map of user ID to value.
      * @return The serialized string (empty if the map is empty).
      */
    std::string serializeUserValues(const std::map<std::string, u16>& values);

    /*! @brief Deserializes a "uid=value;uid=value" string into a map of user values.
      *
      * Empty segments and segments without an '=' separator are ignored.
      *
      * @param data The serialized string.
      * @return The deserialized map.
      */
    std::map<std::string, u16> deserializeUserValues(const std::string& data);

    /*! @brief Determines whether a remaining-time notification should be sent.
      *
      * Notifications are sent every 15 minutes and every minute during the
      * last 5 minutes.
      *
      * @param remainingTimeInMinutes The number of minutes remaining until the daily limit.
      * @return true if a notification should be sent, false otherwise.
      */
    bool shouldSendNotification(u16 remainingTimeInMinutes);

    struct PlayRuleEvaluation {
        u16 remainingTimeInMinutes = 0; ///< Minutes left before the daily limit is reached (0 = exhausted).
        bool outsidePlayWindow = false; ///< true if the adjusted time is outside the playtime window.
        bool shouldBlock = false;       ///< true if the running game must be stopped.
        bool shouldNotify = false;      ///< true if a remaining-time notification must be sent.
    };

    /*! @brief Evaluates the play rules and decides whether to block and/or notify.
      *
      * This is the pure, tamper-protected enforcement decision used by the
      * monitoring loop. A game is blocked when the daily limit is exhausted
      * (remaining minutes == 0) or when the adjusted time is outside the
      * playtime window. A remaining-time notification is sent only when the
      * game is not blocked, the remaining-time panel is enabled, and the
      * notification cadence (see shouldSendNotification) is reached.
      *
      * @param dailyLimit The user's daily limit in minutes (0 = no limit).
      * @param totalDurationToday Minutes already used today.
      * @param adjustedTimeMinutes The tamper-protected current time (minutes since midnight).
      * @param gameStartTime Earliest allowed play time in minutes (0 = disabled).
      * @param bedtime Bed time in minutes (0 = disabled).
      * @param showRemainingTime Whether the remaining-time panel is enabled.
      * @return The evaluation result (remaining time, window state, block/notify decisions).
      */
    PlayRuleEvaluation evaluatePlayRules(u16 dailyLimit,
                                         u16 totalDurationToday,
                                         u16 adjustedTimeMinutes,
                                         u16 gameStartTime,
                                         u16 bedtime,
                                         bool showRemainingTime);

}
