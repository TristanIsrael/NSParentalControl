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
#include <cstdint>
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
    int32_t offset = 0;
    auto r = computeAdjustedTime(540, 0, offset);
    CHECK_EQ(r.time, 540);
    CHECK_EQ(r.jump, 0);
    CHECK_EQ(offset, 0);
}

TEST(adjusted_normal_step) {
    int32_t offset = 0;
    auto r = computeAdjustedTime(541, 540, offset);
    CHECK_EQ(r.time, 541);
    CHECK_EQ(r.jump, 0);
    CHECK_EQ(offset, 0);
}

TEST(adjusted_step_within_threshold_ignored) {
    int32_t offset = 0;
    auto r = computeAdjustedTime(545, 540, offset);
    CHECK_EQ(r.time, 545);
    CHECK_EQ(r.jump, 0);
    CHECK_EQ(offset, 0);
}

TEST(adjusted_forward_jump_compensated) {
    // 09:00 -> 12:00: the clock was pushed forward by 3h, the adjusted time must stay at 09:00
    int32_t offset = 0;
    auto r = computeAdjustedTime(720, 540, offset);
    CHECK_EQ(r.jump, 180);
    CHECK_EQ(offset, -180);
    CHECK_EQ(r.time, 540);
}

TEST(adjusted_backward_jump_compensated) {
    // 19:00 -> 10:00: the clock was pushed back by 9h, the adjusted time must stay at 19:00
    int32_t offset = 0;
    auto r = computeAdjustedTime(600, 1140, offset);
    CHECK_EQ(r.jump, -540);
    CHECK_EQ(offset, 540);
    CHECK_EQ(r.time, 1140);
}

TEST(adjusted_jumps_accumulate_in_offset) {
    int32_t offset = 0;
    uint16_t last = 540;

    auto r1 = computeAdjustedTime(720, last, offset); // +180
    CHECK_EQ(r1.jump, 180);

    last = 720;
    auto r2 = computeAdjustedTime(800, last, offset); // +80
    CHECK_EQ(r2.jump, 80);
    CHECK_EQ(offset, -260);
    CHECK_EQ(r2.time, 540); // 09:00 still
}

TEST(adjusted_wraps_below_zero) {
    int32_t offset = -20;
    auto r = computeAdjustedTime(10, 5, offset);
    CHECK_EQ(r.time, 1430);
}

TEST(adjusted_wraps_above_end_of_day) {
    int32_t offset = 20;
    auto r = computeAdjustedTime(1430, 1425, offset);
    CHECK_EQ(r.time, 10);
}

TEST(adjusted_reset_state) {
    uint16_t last = 100;
    int32_t offset = 42;
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
    for(uint16_t t = 0; t < 1440; t += 7) {
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
    std::map<std::string, uint16_t> m{{"uid1", 480}};
    CHECK_EQ(serializeUserValues(m), std::string("uid1=480"));
}

TEST(serialize_multiple_entries_sorted_by_key) {
    std::map<std::string, uint16_t> m;
    m["uidB"] = 10;
    m["uidA"] = 20;
    m["uidC"] = 0;
    CHECK_EQ(serializeUserValues(m), std::string("uidA=20;uidB=10;uidC=0"));
}

TEST(serialize_zero_value_preserved) {
    std::map<std::string, uint16_t> m{{"uid1", 0}};
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
    std::map<std::string, uint16_t> m;
    m["00112233445566778899aabbccddeeff00112233445566778899aabbccddeeff"] = 1439;
    m["0000000000000000000000000000000000000000000000000000000000000001"] = 1;

    auto back = deserializeUserValues(serializeUserValues(m));
    CHECK_TRUE(back == m);
}

int main() {
    return tf::runAll();
}
