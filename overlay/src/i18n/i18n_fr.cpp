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

/*! @brief French on-screen strings (keys are the English source strings). */
const StringMap& french() {
    static const StringMap table = {
        // Common frame titles / subtitles
        {"Parental Control", "Contrôle parental"},
        {"Administration", "Administration"},
        {"Debug options", "Options de débogage"},
        {"Usage history", "Historique d'utilisation"},
        {"Set limits", "Définir les limites"},
        {"Setup PIN", "Définir le PIN"},
        {"Verify PIN", "Vérifier le PIN"},
        {"Set limits for ", "Définir les limites pour "},
        // Main menu
        {"Parental Control is Enabled", "Contrôle parental activé"},
        {"Parental Control is Disabled", "Contrôle parental désactivé"},
        {"Parental Control is not installed", "Contrôle parental non installé"},
        {"Database tampered!", "Base de données altérée !"},
        {"Database needs upgrade!", "Mise à jour de la base de données requise !"},
        {"User: ", "Utilisateur : "},
        {"Playing: ", "Jeu : "},
        {"Usage: ", "Utilisation : "},
        {"Bedtime: ", "Heure du coucher : "},
        {"Play from: ", "Jeu autorisé à partir de : "},
        {"Remaining: ", "Restant : "},
        {"No user / app started", "Aucun utilisateur ou jeu actif"},
        {"Debug", "Débogage"},
        {"Admin", "Admin"},
        // Admin menu
        {"State", "État"},
        {"Enabled", "Activé"},
        {"Disabled", "Désactivé"},
        {"Working mode", "Mode de fonctionnement"},
        {"Blocking", "Blocage"},
        {"Notify remaining time", "Notifier le temps restant"},
        {"Log level", "Niveau de journalisation"},
        {"DEBUG", "DEBUG"},
        {"INFO", "INFO"},
        {"Set PIN", "Définir le PIN"},
        {"Versions", "Versions"},
        {"Overlay", "Overlay"},
        {"Sysmodule", "Sysmodule"},
        // Debug menu
        {"Run test", "Lancer le test"},
        {"Current user: ", "Utilisateur actuel : "},
        {"Current title: ", "Jeu actuel : "},
        // Usage history
        {"Choose a user", "Choisir un utilisateur"},
        {"Today", "Aujourd'hui"},
        {"Usage", "Utilisation"},
        {"Remaining", "Restant"},
        // Setup limits
        {"Please use \u2190 and \u2192 to move.", "Utilisez \u2190 et \u2192 pour vous déplacer."},
        {"Modify the limits with \u2191 and \u2193.", "Modifiez les limites avec \u2191 et \u2193."},
        {"Play limits", "Limites de jeu"},
        {"Daily", "Quotidien"},
        {"mn", "min"},
        {"h", "h"},
        {"Playtime window", "Fenêtre de jeu"},
        {"Set time to 0:00 to disable.", "Définir 0:00 pour désactiver."},
        {"Allow games from", "Jeu autorisé à partir de"},
        {"Stop games after", "Arrêter les jeux après"},
        // PIN
        {"Please enter the new PIN", "Veuillez saisir le nouveau PIN"},
        {"New PIN", "Nouveau PIN"},
        {"PINs do not match", "Les PIN ne correspondent pas"},
        {"unknown error", "erreur inconnue"},
        {"Press A to retry, B to cancel", "A pour réessayer, B pour annuler"},
        {"Please enter Admin PIN", "Veuillez saisir le PIN admin"},
        {"Admin PIN", "PIN admin"},
        {"PIN is incorrect", "Le PIN est incorrect"}
    };

    return table;
}

}
