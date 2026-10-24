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

#include "panel_admin_menu.h"
#include "logger.h"
#include <switch.h>
#include "Command.hpp"
#include "AppContext.h"
#include "logger.h"
#include "helpers/ipc_helpers.h"
#include "panel_setuppin.h"
#include "panel_main_menu.h"
#include "panel_setup_limits_main.h"

using namespace alefbet::pctrl;
using namespace alefbet::pctrl::logger;

/*! @brief Constructs an AdminMenuPanel instance. */
AdminMenuPanel::AdminMenuPanel() {    
}

/*! @brief Destructs an AdminMenuPanel instance and releases resources. */
AdminMenuPanel::~AdminMenuPanel() {      
}

/*! @brief Closes the panel and cleans up associated resources. */
void AdminMenuPanel::closeAndClean() {
}

/*! @brief Creates and returns the root UI element for the admin menu panel.
 * @return A pointer to the root UI element (OverlayFrame). */
tsl::elm::Element* AdminMenuPanel::createUI() {        
    rootFrame_ = new tsl::elm::OverlayFrame("Parental Control", "Administration");
    rootList_ = new tsl::elm::List();
    
    rebuildUI();

    return rootFrame_;
}


/*! @brief Rebuilds the admin menu list items with their configurations and click handlers. */
void AdminMenuPanel::rebuildUI() {

    // Enable parental control
    auto entryEnabled = new tsl::elm::ToggleListItem("State", getAppContext().is_enabled, "Enabled", "Disabled");
    rootList_->addItem(entryEnabled);        
    entryEnabled->setStateChangedListener([entryEnabled] (bool enabled) -> bool {
        bool res = ipc::setEnabled(enabled);
        if(!res) {
            entryEnabled->setState(!entryEnabled->getState());
        }

        return true;
    });    

    auto entrySetWorkingMode = new tsl::elm::ListItem("Working mode", "Blocking");
    rootList_->addItem(entrySetWorkingMode);

    // Show notifications
    const auto& remainingTimeVisibility = ipc::getShowRemainingTime();
    auto entryShowRemainingTime = new tsl::elm::ToggleListItem("Notify remaining time", remainingTimeVisibility);
    rootList_->addItem(entryShowRemainingTime);
    entryShowRemainingTime->setStateChangedListener([entryShowRemainingTime](bool enabled) -> bool {
        bool res = ipc::setShowRemainingTime(enabled);
        if(!res) {
            entryShowRemainingTime->setState(!entryShowRemainingTime->getState());
        }
        return true;        
    });

    // Log level
    const auto& debugLogEnabled = ipc::isDebugLogEnabled();
    auto logLevelEntry = new tsl::elm::ToggleListItem("Log level", debugLogEnabled, "DEBUG", "INFO");
    rootList_->addItem(logLevelEntry);
    logLevelEntry->setStateChangedListener([logLevelEntry](bool enabled) -> bool {
        logDebug("Set log level");
        bool res = ipc::enableDebugLog(enabled);
        if(!res) {
            logLevelEntry->setState(!logLevelEntry->getState());
        } else {
            //getAppContext().is_debug = enabled;
            setLogLevel(enabled ? DEBUG : INFO);
        }

        return true;
    });

    // Setup PIN
    auto entrySetPin = new tsl::elm::ListItem("Set PIN");
    rootList_->addItem(entrySetPin);
    entrySetPin->setClickListener([](u64 keys) -> bool {
        if(keys & HidNpadButton_A) {
            tsl::changeTo<SetupPinPanel>();
            return true;
        }

        return false;
    });

    // Setup limits
    auto entryLimits = new tsl::elm::ListItem("Set limits");
    rootList_->addItem(entryLimits);
    entryLimits->setClickListener([](u64 keys) -> bool {
        if(keys & HidNpadButton_A) {
            tsl::changeTo<SetupLimitsPanel>();
            return true;
        }
        
        return false;
    });

    rootList_->addItem(new tsl::elm::CategoryHeader("Versions"));
    rootList_->addItem(new tsl::elm::ListItem("Overlay", std::string(VERSION)));
    rootList_->addItem(new tsl::elm::ListItem("Sysmodule", ipc::getVersion()));        

    rootFrame_->setContent(rootList_);
}

/*! @brief Updates the panel state each frame. */
void AdminMenuPanel::update() {
    
}

/*! @brief Handles user input not captured by other UI elements.
 * @param keysDown Keys pressed this frame.
 * @param keysHeld Keys held this frame.
 * @param touchPos Current touch screen position.
 * @param joyStickPosLeft Left analog stick position.
 * @param joyStickPosRight Right analog stick position.
 * @return true if the input was handled, false otherwise. */
bool AdminMenuPanel::handleInput(u64 keysDown, u64 keysHeld, const HidTouchState &touchPos, HidAnalogStickState joyStickPosLeft, HidAnalogStickState joyStickPosRight) {
    if (keysDown & HidNpadButton_B) {
        tsl::changeTo<MainMenuPanel>();
        return true;
    }

    return false;
}