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
#include <switch.h>
#include "settings.h"
#include "history.h"

namespace alefbet::pctrl::database {

    /* Files management */
    bool createDataDirectory();

    /* Data management */
    std::vector<HistoryEntry> getHistory(AccountUid uid, std::string date);
    HistoryEntry addToHistory(AccountUid uid, u64 titleId, u16 duration_in_minutes);

    bool upgradeNeeded();
    bool isTampered();

}
