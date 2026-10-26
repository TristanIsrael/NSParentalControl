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
#include <unordered_map>

namespace alefbet::pctrl::i18n {

    /*! @brief Languages supported by the overlay interface. */
    enum class Language {
        English,
        French,
        ChineseSimplified
    };

    /*! @brief Translation table type: English source text -> translated text. */
    using StringMap = std::unordered_map<std::string, const char*>;

    /*! @brief French table, defined in i18n_fr.cpp. */
    const StringMap& french();

    /*! @brief Simplified Chinese table, defined in i18n_zh_hans.cpp. */
    const StringMap& chineseSimplified();

    /*! @brief Returns the currently active language. */
    Language current();

    /*! @brief Sets the active language. */
    void setLanguage(Language language);

    /*! @brief Reads the console language and activates the matching interface language. */
    void detectSystemLanguage();

    /*! @brief Translates an English source string into the active language.
     *
     * @return The translated string, or the source string when no translation exists.
     */
    const char* tr(const char* sourceText);

    /*! @brief Translates an English template and replaces its "%s" placeholder with the value. */
    std::string trf(const char* sourceTemplate, const std::string& value);

}
