#include "main_overlay.h"
#include "switch.h"
#include "console.h"
#include "database.h"
#include <tesla.hpp>
#include "panel_main_menu.h"

MainOverlay::MainOverlay() {    
}

std::unique_ptr<tsl::Gui> MainOverlay::loadInitialGui() {
    return initially<MainMenu>(); 
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