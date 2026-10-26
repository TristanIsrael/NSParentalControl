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

    /*! @brief Languages supported by the overlay interface. */
    enum class Language {
        English,
        French
    };

    /*! @brief Identifiers of the translatable on-screen strings. */
    enum class Key {
        // Common frame titles / subtitles
        AppTitle,
        Administration,
        DebugOptions,
        UsageHistory,
        SetLimits,
        SetupPin,
        VerifyPin,
        SetLimitsForPrefix,
        // Main menu
        PcEnabled,
        PcDisabled,
        PcNotInstalled,
        DbTampered,
        DbUpgrade,
        UserPrefix,
        PlayingPrefix,
        UsagePrefix,
        BedtimePrefix,
        PlayFromPrefix,
        RemainingPrefix,
        NoUserApp,
        Debug,
        Admin,
        // Admin menu
        State,
        Enabled,
        Disabled,
        WorkingMode,
        Blocking,
        NotifyRemainingTime,
        LogLevel,
        DebugLevel,
        InfoLevel,
        SetPin,
        Versions,
        OverlayVersion,
        SysmoduleVersion,
        // Debug menu
        RunTest,
        CurrentUserPrefix,
        CurrentTitlePrefix,
        // Usage history
        ChooseUser,
        Today,
        Usage,
        Remaining,
        // Setup limits
        HintLeftRight,
        HintUpDown,
        PlayLimits,
        Daily,
        UnitMinutes,
        UnitHours,
        PlaytimeWindow,
        SetTimeZeroHint,
        AllowGamesFrom,
        StopGamesAfter,
        // PIN
        EnterNewPin,
        NewPin,
        PinsMismatch,
        UnknownError,
        PressARetry,
        EnterAdminPin,
        AdminPin,
        PinIncorrect
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

}
