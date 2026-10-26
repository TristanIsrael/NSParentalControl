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

#include <string>

namespace alefbet::pctrl::i18n {

    /*! @brief Languages supported by the sysmodule notifications. */
    enum class Language {
        English,
        French
    };

    /*! @brief Identifiers of the translatable notification strings. */
    enum class Key {
        NotificationTitle,   ///< Attention notification title shown before the message.
        MonitoringStarted,   ///< "monitoring started" notification.
        TimeLeft,            ///< Remaining time template, one "%s" placeholder.
        UnitHours,           ///< Hours unit of a formatted duration.
        UnitMinutes          ///< Minutes unit of a formatted duration.
    };

    /*! @brief Returns the currently active language. */
    Language current();

    /*! @brief Sets the active language. */
    void setLanguage(Language language);

    /*! @brief Reads the console language and activates the matching interface language. */
    void detectSystemLanguage();

    /*! @brief Returns the translated string for the key in the active language. */
    const char* tr(Key key);

    /*! @brief Returns the translated string for the key with its "%s" placeholder replaced by the value. */
    std::string format(Key key, const std::string& value);

    /*! @brief Formats a duration in minutes in the active language (e.g. "2h 30mn" or "2h 30 min"). */
    std::string formatDuration(int durationInMinutes);

}
