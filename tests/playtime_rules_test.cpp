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

#include "test_framework.h"
#include "playtime_rules.h"
#include <map>
#include <string>

using namespace alefbet::pctrl::core;

// ---------------------------------------------------------------------------
// formatTimeInMinutes
// ---------------------------------------------------------------------------

TEST(format_midnight) {
    CHECK_EQ(formatTimeInMinutes(0), std::string("00:00"));
}

TEST(format_midnight_minus_one) {
    CHECK_EQ(formatTimeInMinutes(1439), std::string("23:59"));
}

TEST(format_noon) {
    CHECK_EQ(formatTimeInMinutes(720), std::string("12:00"));
}

TEST(format_single_digits_padded) {
    CHECK_EQ(formatTimeInMinutes(5), std::string("00:05"));
    CHECK_EQ(formatTimeInMinutes(595), std::string("09:55"));
}

// ---------------------------------------------------------------------------
// computeAdjustedTime (tamper protection)
// ---------------------------------------------------------------------------

TEST(adjusted_first_iteration_no_jump) {
    s32 offset = 0;
    auto r = computeAdjustedTime(540, 0, offset);
    CHECK_EQ(r.time, 540);
    CHECK_EQ(r.jump, 0);
    CHECK_EQ(offset, 0);
}

TEST(adjusted_normal_step) {
    s32 offset = 0;
    auto r = computeAdjustedTime(541, 540, offset);
    CHECK_EQ(r.time, 541);
    CHECK_EQ(r.jump, 0);
    CHECK_EQ(offset, 0);
}

TEST(adjusted_step_within_threshold_ignored) {
    s32 offset = 0;
    auto r = computeAdjustedTime(545, 540, offset);
    CHECK_EQ(r.time, 545);
    CHECK_EQ(r.jump, 0);
    CHECK_EQ(offset, 0);
}

TEST(adjusted_forward_jump_compensated) {
    // 09:00 -> 12:00: the clock was pushed forward by 3h, the adjusted time must stay at 09:00
    s32 offset = 0;
    auto r = computeAdjustedTime(720, 540, offset);
    CHECK_EQ(r.jump, 180);
    CHECK_EQ(offset, -180);
    CHECK_EQ(r.time, 540);
}

TEST(adjusted_backward_jump_compensated) {
    // 19:00 -> 10:00: the clock was pushed back by 9h, the adjusted time must stay at 19:00
    s32 offset = 0;
    auto r = computeAdjustedTime(600, 1140, offset);
    CHECK_EQ(r.jump, -540);
    CHECK_EQ(offset, 540);
    CHECK_EQ(r.time, 1140);
}

TEST(adjusted_jumps_accumulate_in_offset) {
    s32 offset = 0;
    u16 last = 540;

    auto r1 = computeAdjustedTime(720, last, offset); // +180
    CHECK_EQ(r1.jump, 180);

    last = 720;
    auto r2 = computeAdjustedTime(800, last, offset); // +80
    CHECK_EQ(r2.jump, 80);
    CHECK_EQ(offset, -260);
    CHECK_EQ(r2.time, 540); // 09:00 still
}

TEST(adjusted_wraps_below_zero) {
    s32 offset = -20;
    auto r = computeAdjustedTime(10, 5, offset);
    CHECK_EQ(r.time, 1430);
}

TEST(adjusted_wraps_above_end_of_day) {
    s32 offset = 20;
    auto r = computeAdjustedTime(1430, 1425, offset);
    CHECK_EQ(r.time, 10);
}

TEST(adjusted_reset_state) {
    u16 last = 100;
    s32 offset = 42;
    resetAdjustedTimeState(last, offset);
    CHECK_EQ(last, 0u);
    CHECK_EQ(offset, 0);
}

// ---------------------------------------------------------------------------
// isInsidePlayWindow
// ---------------------------------------------------------------------------

TEST(window_unrestricted_when_both_bounds_disabled) {
    CHECK_TRUE(isInsidePlayWindow(0, 0, 0));
    CHECK_TRUE(isInsidePlayWindow(720, 0, 0));
    CHECK_TRUE(isInsidePlayWindow(1439, 0, 0));
}

TEST(window_same_day) {
    // 08:00 to 20:00
    CHECK_TRUE(!isInsidePlayWindow(420, 480, 1200));   // 07:00 -> before
    CHECK_TRUE(isInsidePlayWindow(480, 480, 1200));    // 08:00 -> start inclusive
    CHECK_TRUE(isInsidePlayWindow(720, 480, 1200));    // 12:00 -> inside
    CHECK_TRUE(isInsidePlayWindow(1199, 480, 1200));   // 19:59 -> inside
    CHECK_TRUE(!isInsidePlayWindow(1200, 480, 1200));  // 20:00 -> end exclusive
    CHECK_TRUE(!isInsidePlayWindow(0, 480, 1200));     // 00:00 -> before
    CHECK_TRUE(!isInsidePlayWindow(1439, 480, 1200));  // 23:59 -> after
}

TEST(window_crossing_midnight) {
    // 20:00 to 08:00
    CHECK_TRUE(isInsidePlayWindow(1200, 1200, 480));   // 20:00 -> start inclusive
    CHECK_TRUE(isInsidePlayWindow(1320, 1200, 480));   // 22:00 -> inside (the core case of test plan C2)
    CHECK_TRUE(isInsidePlayWindow(0, 1200, 480));      // 00:00 -> inside
    CHECK_TRUE(isInsidePlayWindow(30, 1200, 480));     // 00:30 -> inside
    CHECK_TRUE(isInsidePlayWindow(479, 1200, 480));    // 07:59 -> inside
    CHECK_TRUE(!isInsidePlayWindow(480, 1200, 480));   // 08:00 -> end exclusive
    CHECK_TRUE(!isInsidePlayWindow(720, 1200, 480));   // 12:00 -> outside
    CHECK_TRUE(!isInsidePlayWindow(1199, 1200, 480));  // 19:59 -> outside
}

TEST(window_empty_when_start_equals_end) {
    for(u16 t = 0; t < 1440; t += 7) {
        CHECK_TRUE(!isInsidePlayWindow(t, 480, 480));
    }
}

TEST(window_start_only) {
    CHECK_TRUE(!isInsidePlayWindow(0, 720, 0));
    CHECK_TRUE(!isInsidePlayWindow(719, 720, 0));
    CHECK_TRUE(isInsidePlayWindow(720, 720, 0));
    CHECK_TRUE(isInsidePlayWindow(1439, 720, 0));
}

TEST(window_end_only) {
    CHECK_TRUE(isInsidePlayWindow(0, 0, 720));
    CHECK_TRUE(isInsidePlayWindow(719, 0, 720));
    CHECK_TRUE(!isInsidePlayWindow(720, 0, 720));
    CHECK_TRUE(!isInsidePlayWindow(1439, 0, 720));
}

TEST(window_fifteen_minutes_across_midnight) {
    // 23:59 to 00:01
    CHECK_TRUE(isInsidePlayWindow(1439, 1439, 1));
    CHECK_TRUE(isInsidePlayWindow(0, 1439, 1));
    CHECK_TRUE(!isInsidePlayWindow(1, 1439, 1));
    CHECK_TRUE(!isInsidePlayWindow(720, 1439, 1));
}

// ---------------------------------------------------------------------------
// Per-user values serialization
// ---------------------------------------------------------------------------

TEST(serialize_empty) {
    CHECK_EQ(serializeUserValues({}), std::string(""));
}

TEST(serialize_single_entry) {
    std::map<std::string, u16> m{{"uid1", 480}};
    CHECK_EQ(serializeUserValues(m), std::string("uid1=480"));
}

TEST(serialize_multiple_entries_sorted_by_key) {
    std::map<std::string, u16> m;
    m["uidB"] = 10;
    m["uidA"] = 20;
    m["uidC"] = 0;
    CHECK_EQ(serializeUserValues(m), std::string("uidA=20;uidB=10;uidC=0"));
}

TEST(serialize_zero_value_preserved) {
    std::map<std::string, u16> m{{"uid1", 0}};
    CHECK_EQ(serializeUserValues(m), std::string("uid1=0"));
}

TEST(deserialize_empty) {
    CHECK_EQ(deserializeUserValues("").size(), 0u);
}

TEST(deserialize_multiple_entries) {
    auto m = deserializeUserValues("uidA=480;uidB=1200;uidC=0");
    CHECK_EQ(m.size(), 3u);
    CHECK_EQ(m.at("uidA"), 480);
    CHECK_EQ(m.at("uidB"), 1200);
    CHECK_EQ(m.at("uidC"), 0);
}

TEST(deserialize_skips_malformed_segments) {
    auto m = deserializeUserValues("uidA=480;;garbage;uidB=10;");
    CHECK_EQ(m.size(), 2u);
    CHECK_EQ(m.at("uidA"), 480);
    CHECK_EQ(m.at("uidB"), 10);
}

TEST(serialize_deserialize_roundtrip) {
    std::map<std::string, u16> m;
    m["00112233445566778899aabbccddeeff00112233445566778899aabbccddeeff"] = 1439;
    m["0000000000000000000000000000000000000000000000000000000000000001"] = 1;

    auto back = deserializeUserValues(serializeUserValues(m));
    CHECK_TRUE(back == m);
}

// ---------------------------------------------------------------------------
// shouldSendNotification (notification cadence)
// ---------------------------------------------------------------------------

TEST(notify_every_15_minutes) {
    CHECK_TRUE(shouldSendNotification(15));
    CHECK_TRUE(shouldSendNotification(30));
    CHECK_TRUE(shouldSendNotification(45));
    CHECK_TRUE(shouldSendNotification(75));
    CHECK_TRUE(!shouldSendNotification(16));
    CHECK_TRUE(!shouldSendNotification(31));
}

TEST(notify_every_minute_in_last_5) {
    CHECK_TRUE(shouldSendNotification(5));
    CHECK_TRUE(shouldSendNotification(4));
    CHECK_TRUE(shouldSendNotification(3));
    CHECK_TRUE(shouldSendNotification(2));
    CHECK_TRUE(shouldSendNotification(1));
    CHECK_TRUE(!shouldSendNotification(6));
    CHECK_TRUE(!shouldSendNotification(7));
}

// ---------------------------------------------------------------------------
// evaluatePlayRules (enforcement decision)
// ---------------------------------------------------------------------------

TEST(eval_within_limit_inside_window_no_action) {
    // limit 120, used 31 -> remaining 89 (not a multiple of 15, > 5), 12:00 inside 08:00-20:00
    // -> keep playing, no notification
    auto r = evaluatePlayRules(120, 31, 720, 480, 1200, true);
    CHECK_EQ(r.remainingTimeInMinutes, 89);
    CHECK_TRUE(!r.outsidePlayWindow);
    CHECK_TRUE(!r.shouldBlock);
    CHECK_TRUE(!r.shouldNotify);
}

TEST(eval_exhausted_limit_blocks) {
    // limit 60, used 60 -> remaining 0 -> block
    auto r = evaluatePlayRules(60, 60, 720, 0, 0, true);
    CHECK_EQ(r.remainingTimeInMinutes, 0);
    CHECK_TRUE(r.shouldBlock);
    CHECK_TRUE(!r.shouldNotify);
}

TEST(eval_zero_limit_yields_zero_remaining) {
    // Mirrors the monitor arithmetic: daily_limit 0 -> remaining 0 (0 > used is false).
    // This is the existing product behavior for a user with no daily limit set.
    auto r = evaluatePlayRules(0, 30, 720, 0, 0, true);
    CHECK_EQ(r.remainingTimeInMinutes, 0);
    CHECK_TRUE(r.shouldBlock);
}

TEST(eval_outside_window_blocks_even_with_time_left) {
    // plenty of time left (limit 600, used 0) but 23:00 is after bed 22:00 -> block
    auto r = evaluatePlayRules(600, 0, 1380, 0, 1320, true);
    CHECK_EQ(r.remainingTimeInMinutes, 600);
    CHECK_TRUE(r.outsidePlayWindow);
    CHECK_TRUE(r.shouldBlock);
    CHECK_TRUE(!r.shouldNotify);
}

TEST(eval_before_game_start_blocks) {
    // 07:00 is before game start 08:00 -> block
    auto r = evaluatePlayRules(600, 0, 420, 480, 0, true);
    CHECK_TRUE(r.outsidePlayWindow);
    CHECK_TRUE(r.shouldBlock);
}

TEST(eval_crossing_midnight_window_allows_night) {
    // window 20:00-08:00, time 22:00, time left -> not blocked
    auto r = evaluatePlayRules(600, 0, 1320, 1200, 480, true);
    CHECK_TRUE(!r.outsidePlayWindow);
    CHECK_TRUE(!r.shouldBlock);
}

TEST(eval_notifies_when_cadence_reached_and_panel_on) {
    // remaining 30 (a multiple of 15), inside window, panel on -> notify
    auto r = evaluatePlayRules(120, 90, 720, 480, 1200, true);
    CHECK_EQ(r.remainingTimeInMinutes, 30);
    CHECK_TRUE(!r.shouldBlock);
    CHECK_TRUE(r.shouldNotify);
}

TEST(eval_no_notification_when_panel_off) {
    // remaining 30 but panel disabled -> no notify
    auto r = evaluatePlayRules(120, 90, 720, 480, 1200, false);
    CHECK_EQ(r.remainingTimeInMinutes, 30);
    CHECK_TRUE(!r.shouldNotify);
}

TEST(eval_no_notification_when_cadence_not_reached) {
    // remaining 90? 90%15==0 -> would notify; use remaining 91 (120-29) -> no notify
    auto r = evaluatePlayRules(120, 29, 720, 480, 1200, true);
    CHECK_EQ(r.remainingTimeInMinutes, 91);
    CHECK_TRUE(!r.shouldBlock);
    CHECK_TRUE(!r.shouldNotify);
}

TEST(eval_blocked_never_notifies) {
    // exhausted + outside window: blocked, and must not notify
    auto r = evaluatePlayRules(60, 60, 1380, 0, 1320, true);
    CHECK_TRUE(r.shouldBlock);
    CHECK_TRUE(!r.shouldNotify);
}

int main() {
    return tf::runAll();
}
