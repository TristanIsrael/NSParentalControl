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

#include "i18n.h"

namespace alefbet::pctrl::i18n {

/*! @brief French notification strings (keys are the English source strings). */
const StringMap& french() {
    static const StringMap table = {
        {"Parental Control", "Contrôle parental"},
        {"Parental control enabled", "Contrôle parental activé"},
        {"%s left", "Il reste %s"},
        {"h", "h"},
        {"mn", " min"}
    };

    return table;
}

}
