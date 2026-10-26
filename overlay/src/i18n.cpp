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

constexpr std::array<Entry, 60> kTable = {{
    // Common frame titles / subtitles
    {Key::AppTitle, "Parental Control", "Contrôle parental"},
    {Key::Administration, "Administration", "Administration"},
    {Key::DebugOptions, "Debug options", "Options de débogage"},
    {Key::UsageHistory, "Usage history", "Historique d'utilisation"},
    {Key::SetLimits, "Set limits", "Définir les limites"},
    {Key::SetupPin, "Setup PIN", "Définir le PIN"},
    {Key::VerifyPin, "Verify PIN", "Vérifier le PIN"},
    {Key::SetLimitsForPrefix, "Set limits for ", "Définir les limites pour "},
    // Main menu
    {Key::PcEnabled, "Parental Control is Enabled", "Contrôle parental activé"},
    {Key::PcDisabled, "Parental Control is Disabled", "Contrôle parental désactivé"},
    {Key::PcNotInstalled, "Parental Control is not installed", "Contrôle parental non installé"},
    {Key::DbTampered, "Database tampered!", "Base de données altérée !"},
    {Key::DbUpgrade, "Database needs upgrade!", "Mise à jour de la base de données requise !"},
    {Key::UserPrefix, "User: ", "Utilisateur : "},
    {Key::PlayingPrefix, "Playing: ", "Jeu : "},
    {Key::UsagePrefix, "Usage: ", "Utilisation : "},
    {Key::BedtimePrefix, "Bedtime: ", "Heure du coucher : "},
    {Key::PlayFromPrefix, "Play from: ", "Jeu autorisé à partir de : "},
    {Key::RemainingPrefix, "Remaining: ", "Restant : "},
    {Key::NoUserApp, "No user / app started", "Aucun utilisateur ou jeu actif"},
    {Key::Debug, "Debug", "Débogage"},
    {Key::Admin, "Admin", "Admin"},
    // Admin menu
    {Key::State, "State", "État"},
    {Key::Enabled, "Enabled", "Activé"},
    {Key::Disabled, "Disabled", "Désactivé"},
    {Key::WorkingMode, "Working mode", "Mode de fonctionnement"},
    {Key::Blocking, "Blocking", "Blocage"},
    {Key::NotifyRemainingTime, "Notify remaining time", "Notifier le temps restant"},
    {Key::LogLevel, "Log level", "Niveau de journalisation"},
    {Key::DebugLevel, "DEBUG", "DEBUG"},
    {Key::InfoLevel, "INFO", "INFO"},
    {Key::SetPin, "Set PIN", "Définir le PIN"},
    {Key::Versions, "Versions", "Versions"},
    {Key::OverlayVersion, "Overlay", "Overlay"},
    {Key::SysmoduleVersion, "Sysmodule", "Sysmodule"},
    // Debug menu
    {Key::RunTest, "Run test", "Lancer le test"},
    {Key::CurrentUserPrefix, "Current user: ", "Utilisateur actuel : "},
    {Key::CurrentTitlePrefix, "Current title: ", "Jeu actuel : "},
    // Usage history
    {Key::ChooseUser, "Choose a user", "Choisir un utilisateur"},
    {Key::Today, "Today", "Aujourd'hui"},
    {Key::Usage, "Usage", "Utilisation"},
    {Key::Remaining, "Remaining", "Restant"},
    // Setup limits
    {Key::HintLeftRight, "Please use \u2190 and \u2192 to move.", "Utilisez \u2190 et \u2192 pour vous déplacer."},
    {Key::HintUpDown, "Modify the limits with \u2191 and \u2193.", "Modifiez les limites avec \u2191 et \u2193."},
    {Key::PlayLimits, "Play limits", "Limites de jeu"},
    {Key::Daily, "Daily", "Quotidien"},
    {Key::UnitMinutes, "mn", "min"},
    {Key::UnitHours, "h", "h"},
    {Key::PlaytimeWindow, "Playtime window", "Fenêtre de jeu"},
    {Key::SetTimeZeroHint, "Set time to 0h 0mn to disable.", "Mettre 0h 0 min pour désactiver."},
    {Key::AllowGamesFrom, "Allow games from", "Jeu autorisé à partir de"},
    {Key::StopGamesAfter, "Stop games after", "Arrêter les jeux après"},
    // PIN
    {Key::EnterNewPin, "Please enter the new PIN", "Veuillez saisir le nouveau PIN"},
    {Key::NewPin, "New PIN", "Nouveau PIN"},
    {Key::PinsMismatch, "PINs do not match", "Les PIN ne correspondent pas"},
    {Key::UnknownError, "unknown error", "erreur inconnue"},
    {Key::PressARetry, "Press A to retry, B to cancel", "Appuyez sur A pour réessayer, B pour annuler"},
    {Key::EnterAdminPin, "Please enter Admin PIN", "Veuillez saisir le PIN administrateur"},
    {Key::AdminPin, "Admin PIN", "PIN administrateur"},
    {Key::PinIncorrect, "PIN is incorrect", "Le PIN est incorrect"}
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

}
