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

#ifdef __SWITCH__
#include <switch.h>
#endif

namespace alefbet::pctrl::i18n {

namespace {

Language g_language = Language::English;

const StringMap* tableFor(Language language) {
    switch(language) {
        case Language::French:  return &french();
        case Language::English: return nullptr;  // source language
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
 * function exists so the translation tables can be exercised without a console.
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

/*! @brief Translates an English source string into the active language.
 *
 * The source string is looked up in the active language's table; when no
 * translation exists (or for the English source language) the source string
 * itself is returned.
 */
const char* tr(const char* sourceText) {
    const StringMap* table = tableFor(g_language);
    if(table != nullptr) {
        const auto it = table->find(sourceText);
        if(it != table->end()) {
            return it->second;
        }
    }

    return sourceText;
}

/*! @brief Translates an English template and replaces its "%s" placeholder with the value. */
std::string format(const char* sourceTemplate, const std::string& value) {
    std::string result = tr(sourceTemplate);
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

    return std::to_string(hours) + tr("h") + " " + std::to_string(minutes) + tr("mn");
}

}
