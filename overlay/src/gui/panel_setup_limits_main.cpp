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

#include "panel_setup_limits_main.h"
#include <switch.h>
#include <chrono>
#include "logger.h"
#include "Command.hpp"
#include "AppContext.h"
#include "helpers/ipc_helpers.h"
#include "helpers/switch_helpers.h"
#include "panel_setup_limits_user.h"
#include "i18n.h"

using namespace alefbet::pctrl;
using namespace alefbet::pctrl::helpers;
using namespace alefbet::pctrl::i18n;

/*! @brief Constructs a SetupLimitsPanel instance. */
SetupLimitsPanel::SetupLimitsPanel() {    
}

/*! @brief Destructs a SetupLimitsPanel instance and releases resources. */
SetupLimitsPanel::~SetupLimitsPanel() {      
}

/*! @brief Creates and returns the root UI element for the setup limits main panel.
 * @return A pointer to the root UI element (OverlayFrame). */
tsl::elm::Element* SetupLimitsPanel::createUI() {
    rootFrame_ = new tsl::elm::OverlayFrame(tr(Key::AppTitle), tr(Key::SetLimits));
    rootList_ = new tsl::elm::List();
    
    rebuildUI();        

    return rootFrame_;
}


/*! @brief Rebuilds the user selection list for setting limits. */
void SetupLimitsPanel::rebuildUI() {
    rootList_->addItem(new tsl::elm::CategoryHeader(tr(Key::ChooseUser)));

    const auto& users = helpers::getUsersList();    

    for(const auto& user: users) {        
        auto entryUser = new tsl::elm::ListItem(user.nickname);
        rootList_->addItem(entryUser);        
        entryUser->setClickListener([user](u64 keys) {
            if(keys & HidNpadButton_A) {
                tsl::changeTo<SetupLimitsUserPanel>(user);
                return true;
            }

            return false;
        });
    }   

    rootFrame_->setContent(rootList_);
}

/*! @brief Updates the panel state each frame. */
void SetupLimitsPanel::update() {    
}

/*! @brief Handles user input not captured by other UI elements.
 * @param keysDown Keys pressed this frame.
 * @param keysHeld Keys held this frame.
 * @param touchPos Current touch screen position.
 * @param joyStickPosLeft Left analog stick position.
 * @param joyStickPosRight Right analog stick position.
 * @return true if the input was handled, false otherwise. */
bool SetupLimitsPanel::handleInput(u64 keysDown, u64 keysHeld, const HidTouchState &touchPos, HidAnalogStickState joyStickPosLeft, HidAnalogStickState joyStickPosRight) {
    if (keysDown & HidNpadButton_B) {
        tsl::goBack();
        return true;
    }

    return false;
}