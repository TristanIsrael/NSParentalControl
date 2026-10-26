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

// Host test for the IPC command contract.
//
// The sysmodule Command.hpp only depends on <string>, so it is host-compilable.
// These tests guard the IPC contract: every enumerator must be covered by
// commandToString (a missing case falls through to "Unknown") and the ordinals
// of the first/last commands are pinned so that reordering is caught.
//
// NOTE: keep kAllCommands in sync with the Command enum in
// sysmodule/source/ipc/Command.hpp (and, by the project rule, the identical
// enum in overlay/src/Command.hpp).

#include "test_framework.h"
#include "../sysmodule/source/ipc/Command.hpp"
#include <string>
#include <vector>

using namespace Ipc;

static const std::vector<Command> kAllCommands = {
    Command::Version,
    Command::Test,
    Command::Initialize,
    Command::GetUserUsageTime,
    Command::GetUserRemainingTime,
    Command::GetCurrentUserUid,
    Command::GetCurrentUserNickname,
    Command::GetRunningApplication,
    Command::SetUserDailyLimit,
    Command::SetAdminPin,
    Command::VerifyAdminPin,
    Command::SetWorkingMode,
    Command::GetWorkingMode,
    Command::SetShowRemainingTime,
    Command::GetShowRemainingTime,
    Command::IsEnabled,
    Command::SetEnabled,
    Command::SetLogLevel,
    Command::GetLogLevel,
    Command::IsTampered,
    Command::MustUpgradeDatabase,
    Command::GetBlacklistedTitlesCount,
    Command::GetBlacklistedTitle,
    Command::AddTitleToBlacklist,
    Command::RemoveTitleFromBlacklist,
    Command::GetUserDailyLimit,
    Command::SetAuthenticationActive,
    Command::IsAuthenticationActive,
    Command::CancelScreenTimeout,
    Command::SetBedtime,
    Command::GetBedtime,
    Command::SetGameStartTime,
    Command::GetGameStartTime,
    Command::GetAdjustedCurrentTime
};

TEST(command_count) {
    CHECK_EQ(kAllCommands.size(), 34u);
}

TEST(command_first_and_last_ordinals) {
    CHECK_EQ(static_cast<int>(Command::Version), 0);
    CHECK_EQ(static_cast<int>(Command::GetAdjustedCurrentTime), 33);
}

TEST(command_to_string_covers_all_enumerators) {
    // A value missing from the commandToString switch returns "Unknown".
    for(const Command c : kAllCommands) {
        CHECK_TRUE(commandToString(c) != "Unknown");
    }
}

TEST(command_to_string_exact_names) {
    CHECK_EQ(commandToString(Command::Version), std::string("Version"));
    CHECK_EQ(commandToString(Command::SetBedtime), std::string("SetBedtime"));
    CHECK_EQ(commandToString(Command::GetGameStartTime), std::string("GetGameStartTime"));
    CHECK_EQ(commandToString(Command::GetAdjustedCurrentTime), std::string("GetAdjustedCurrentTime"));
}

int main() {
    return tf::runAll();
}
