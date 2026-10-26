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
#include <array>

#ifdef __SWITCH__
#include <switch.h>
#endif

namespace alefbet::pctrl::i18n {

namespace {

struct Entry {
    Key key;
    const char* english;
    const char* french;
};

constexpr std::array<Entry, 5> kTable = {{
    {Key::NotificationTitle, "Parental Control", "Contrôle parental"},
    {Key::MonitoringStarted, "Parental control enabled", "Contrôle parental activé"},
    {Key::TimeLeft, "%s left", "Il reste %s"},
    {Key::UnitHours, "h", "h"},
    {Key::UnitMinutes, "mn", " min"}
}};

Language g_language = Language::English;

const Entry* findEntry(Key key) {
    for(const auto& entry : kTable) {
        if(entry.key == key) {
            return &entry;
        }
    }

    return nullptr;
}

}

/*! @brief Returns the currently active language. */
Language current() {
    return g_language;
}

/*! @brief Sets the active language.
 *
 * The language is normally set once by detectSystemLanguage at startup; this
 * function exists so the translation table can be exercised without a console.
 */
void setLanguage(Language language) {
    g_language = language;
}

/*! @brief Reads the console language (SET service) and activates the matching language.
 *
 * French is selected when the console language is French (including Canadian
 * French); any other console language falls back to English. The SET service
 * must be initialized (setInitialize) before calling this function.
 */
void detectSystemLanguage() {
#ifdef __SWITCH__
    u64 languageCode = 0;
    SetLanguage consoleLanguage = SetLanguage_ENUS;

    if(R_SUCCEEDED(setGetSystemLanguage(&languageCode)) &&
       R_SUCCEEDED(setMakeLanguage(languageCode, &consoleLanguage)) &&
       (consoleLanguage == SetLanguage_FR || consoleLanguage == SetLanguage_FRCA)) {
        g_language = Language::French;
    } else {
        g_language = Language::English;
    }
#else
    g_language = Language::English;
#endif
}

/*! @brief Returns the translated string for the key in the active language.
 *
 * @return The translated string, or an empty string if the key is unknown.
 */
const char* tr(Key key) {
    const Entry* entry = findEntry(key);
    if(entry == nullptr) {
        return "";
    }

    return g_language == Language::French ? entry->french : entry->english;
}

/*! @brief Returns the translated string for the key with its "%s" placeholder replaced by the value. */
std::string format(Key key, const std::string& value) {
    std::string result = tr(key);
    const size_t placeholder = result.find("%s");
    if(placeholder != std::string::npos) {
        result.replace(placeholder, 2, value);
    }

    return result;
}

/*! @brief Formats a duration in minutes in the active language.
 *
 * @return e.g. "2h 30mn" (English) or "2h 30 min" (French).
 */
std::string formatDuration(int durationInMinutes) {
    const int hours = durationInMinutes / 60;
    const int minutes = durationInMinutes % 60;

    return std::to_string(hours) + tr(Key::UnitHours) + " " + std::to_string(minutes) + tr(Key::UnitMinutes);
}

}
