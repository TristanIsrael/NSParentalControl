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

// Host test for the sysmodule translation module (i18n).
//
// The module is compiled without __SWITCH__ on the host, so the SET service
// is not involved: detectSystemLanguage() falls back to English and the tests
// switch the language explicitly with setLanguage().

#include "test_framework.h"
#include "../sysmodule/source/i18n.h"
#include <string>

using namespace alefbet::pctrl::i18n;

TEST(tr_english_strings) {
    setLanguage(Language::English);
    CHECK_EQ(tr(Key::NotificationTitle), std::string("Parental Control"));
    CHECK_EQ(tr(Key::MonitoringStarted), std::string("Parental control enabled"));
    CHECK_EQ(tr(Key::TimeLeft), std::string("%s left"));
    CHECK_EQ(tr(Key::UnitHours), std::string("h"));
    CHECK_EQ(tr(Key::UnitMinutes), std::string("mn"));
}

TEST(tr_french_strings) {
    setLanguage(Language::French);
    CHECK_EQ(tr(Key::NotificationTitle), std::string("Contrôle parental"));
    CHECK_EQ(tr(Key::MonitoringStarted), std::string("Contrôle parental activé"));
    CHECK_EQ(tr(Key::TimeLeft), std::string("Il reste %s"));
    CHECK_EQ(tr(Key::UnitHours), std::string("h"));
    CHECK_EQ(tr(Key::UnitMinutes), std::string(" min"));
}

TEST(format_replaces_placeholder_english) {
    setLanguage(Language::English);
    CHECK_EQ(format(Key::TimeLeft, "2h 30mn"), std::string("2h 30mn left"));
    CHECK_EQ(format(Key::TimeLeft, "60s"), std::string("60s left"));
}

TEST(format_replaces_placeholder_french) {
    setLanguage(Language::French);
    CHECK_EQ(format(Key::TimeLeft, "2h 30 min"), std::string("Il reste 2h 30 min"));
    CHECK_EQ(format(Key::TimeLeft, "60s"), std::string("Il reste 60s"));
}

TEST(format_without_placeholder_is_unchanged) {
    setLanguage(Language::English);
    CHECK_EQ(format(Key::NotificationTitle, "x"), std::string("Parental Control"));
}

TEST(format_duration_english) {
    setLanguage(Language::English);
    CHECK_EQ(formatDuration(150), std::string("2h 30mn"));
    CHECK_EQ(formatDuration(30), std::string("0h 30mn"));
    CHECK_EQ(formatDuration(0), std::string("0h 0mn"));
}

TEST(format_duration_french) {
    setLanguage(Language::French);
    CHECK_EQ(formatDuration(150), std::string("2h 30 min"));
    CHECK_EQ(formatDuration(30), std::string("0h 30 min"));
    CHECK_EQ(formatDuration(0), std::string("0h 0 min"));
}

TEST(detect_system_language_falls_back_to_english_on_host) {
    // On the host there is no SET service, so detection must keep English.
    setLanguage(Language::French);
    detectSystemLanguage();
    CHECK_EQ(static_cast<int>(current()), static_cast<int>(Language::English));

    // Leave the global state in a deterministic state for the other tests.
    setLanguage(Language::English);
}

int main() {
    return tf::runAll();
}
