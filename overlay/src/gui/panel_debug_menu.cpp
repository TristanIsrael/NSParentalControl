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

#include "panel_debug_menu.h"
#include "logger.h"
#include <switch.h>
#include "Command.hpp"
#include "AppContext.h"
#include "helpers/ipc_helpers.h"

using namespace alefbet::pctrl;

DebugMenu::DebugMenu() {}

DebugMenu::~DebugMenu() {  
    closeAndClean();
}

void DebugMenu::closeAndClean() {
}


tsl::elm::Element* DebugMenu::createUI() {
    std::string subTitle = "Debug options";

    rootFrame_ = new tsl::elm::OverlayFrame("Parental Control", subTitle);
    rootList_ = new tsl::elm::List();
    
    rebuildUI();

    return rootFrame_;
}


void DebugMenu::rebuildUI() {
   
    // Run Test
    auto entryTest = new tsl::elm::ListItem("Run test");
    rootList_->addItem(entryTest);
    entryTest->setClickListener([this](u64 keys) {
        if(keys & HidNpadButton_A) {
            ipc::startTest();
            return true;
        }

        return false;
    });

    // Current User
    auto currentUser = ipc::getCurrentUserNickname();
    auto entryUser = new tsl::elm::ListItem("Current user: " +currentUser);
    rootList_->addItem(entryUser);
    
    // Current game
    auto currentTitle = ipc::getCurrentTitle();
    auto entryTitle = new tsl::elm::ListItem("Current title: " +currentTitle);
    rootList_->addItem(entryTitle);    
    
    rootFrame_->setContent(rootList_);
}

void DebugMenu::update() {
    
}

// Called once every frame to handle inputs not handled by other UI elements
bool DebugMenu::handleInput(u64 keysDown, u64 keysHeld, const HidTouchState &touchPos, HidAnalogStickState joyStickPosLeft, HidAnalogStickState joyStickPosRight) {
    if (keysDown & HidNpadButton_B) {
        tsl::goBack();
        return true;
    }

    return false;
}