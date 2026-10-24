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

#include "main_overlay.h"
#include <switch.h>
#include "panel_main_menu.h"
#include "logger.h"

using namespace alefbet::pctrl::logger;

/*! @brief Constructs a MainOverlay instance. */
MainOverlay::MainOverlay() {    
}

extern "C" u64 __nx_vi_layer_id;

/*! @brief Loads the initial GUI when the overlay starts.
 * @return A unique pointer to the Gui to display initially. */
std::unique_ptr<tsl::Gui> MainOverlay::loadInitialGui() {
    logDebug("loadInitialGui. __nx_vi_layer_id=%i\n", __nx_vi_layer_id);
    logDebug("aruid=%i\n", appletGetAppletResourceUserId());
    return initially<MainMenuPanel>(); 
}

/*void BlockerOverlay::onShow() { 
    if (rootFrame != nullptr && active) {
        tsl::Overlay::get()->getCurrentGui()->removeFocus();
        rebuildUI();
        rootFrame->invalidate();
        tsl::Overlay::get()->getCurrentGui()->requestFocus(rootFrame, tsl::FocusDirection::None);
    }
}*/

/*bool askPIN() {
    consoleClear();
    printf("Entrez le PIN parental: ");
    consoleUpdate(NULL);

    char input[16] = {0};
    int pos = 0;
    while (true) {
        u64 k_down = hidKeysDown(CONTROLLER_P1_AUTO);
        if (k_down & KEY_PLUS) break;

        for (char c = '0'; c <= '9'; c++) {
            if (k_down & (1 << (c - '0'))) {
                if (pos < 15) {
                    input[pos++] = c;
                    input[pos] = 0;
                }
            }
        }
        consoleClear();
        printf("Entrez le PIN parental: %s\n", input);
        consoleUpdate(NULL);
        svcSleepThread(50000000ULL);
    }
    return std::string(input) == parentalPIN;
}

void configureLimits(UserSession &user, GameSession &game) {
    if (!askPIN()) return;

    int new_limit = game.daily_limit / 60; // minutes
    consoleClear();
    printf("Limite actuelle pour %s: %d minutes\n", game.game_id.c_str(), new_limit);
    printf("Entrez nouvelle limite en minutes: ");
    consoleUpdate(NULL);

    char input[16] = {0};
    int pos = 0;
    while (true) {
        u64 k_down = hidKeysDown(CONTROLLER_P1_AUTO);
        if (k_down & KEY_PLUS) break;

        for (char c = '0'; c <= '9'; c++) {
            if (k_down & (1 << (c - '0'))) {
                if (pos < 15) {
                    input[pos++] = c;
                    input[pos] = 0;
                }
            }
        }
        consoleClear();
        printf("Entrez nouvelle limite en minutes: %s\n", input);
        consoleUpdate(NULL);
        svcSleepThread(50000000ULL);
    }

    int val = atoi(input);
    if (val > 0) game.daily_limit = val * 60;
}*/