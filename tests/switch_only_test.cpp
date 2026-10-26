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

// Toolchain test: NOT built by the default host target.
//
// This test requires the devkitPro devkitA64 toolchain (build it with
// `make -C tests switch`). It verifies that the pure playtime logic and the
// IPC command contract also build and behave correctly under the real AArch64
// toolchain, and that the libnx base types (u8/u16/u32/u64/s32/s64) have the
// expected sizes on the target. The host build cannot run it, so it is kept
// separate from the always-green `make -C tests` target.
//
// On-device integration tests (IPC round-trip against the running service,
// the Monitor loop with real time/account, and actual game blocking) require
// a console and are tracked in docs/test-plan.md, section "On-device tests".

#include "test_framework.h"
#include "playtime_rules.h"
#include "../sysmodule/source/ipc/Command.hpp"
#include <string>

using namespace alefbet::pctrl::core;

// The libnx base types must match the target ABI.
static_assert(sizeof(u8) == 1, "u8 must be 1 byte");
static_assert(sizeof(u16) == 2, "u16 must be 2 bytes");
static_assert(sizeof(u32) == 4, "u32 must be 4 bytes");
static_assert(sizeof(u64) == 8, "u64 must be 8 bytes");
static_assert(sizeof(s32) == 4, "s32 must be 4 bytes");
static_assert(sizeof(s64) == 8, "s64 must be 8 bytes");

TEST(toolchain_window_crossing_midnight) {
    // 22:00 inside a 20:00-08:00 window; 12:00 outside it
    CHECK_TRUE(isInsidePlayWindow(1320, 1200, 480));
    CHECK_TRUE(!isInsidePlayWindow(720, 1200, 480));
}

TEST(toolchain_eval_blocks_outside_window) {
    // Time left, but 23:00 is after bed 22:00 -> block
    auto r = evaluatePlayRules(600, 0, 1380, 0, 1320, true);
    CHECK_TRUE(r.outsidePlayWindow);
    CHECK_TRUE(r.shouldBlock);
}

TEST(toolchain_eval_exhausted_limit_blocks) {
    auto r = evaluatePlayRules(60, 60, 720, 0, 0, true);
    CHECK_EQ(r.remainingTimeInMinutes, 0);
    CHECK_TRUE(r.shouldBlock);
}

TEST(toolchain_command_contract) {
    CHECK_EQ(static_cast<int>(Ipc::Command::Version), 0);
    CHECK_EQ(static_cast<int>(Ipc::Command::GetAdjustedCurrentTime), 33);
    CHECK_EQ(Ipc::commandToString(Ipc::Command::GetAdjustedCurrentTime), std::string("GetAdjustedCurrentTime"));
}

int main() {
    return tf::runAll();
}
