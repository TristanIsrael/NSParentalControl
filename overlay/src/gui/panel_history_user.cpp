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

#include "panel_history_user.h"
#include <switch.h>
#include <chrono>
#include <string>
#include "logger.h"
#include "Command.hpp"
#include "AppContext.h"
#include "helpers/ipc_helpers.h"
#include "i18n/i18n.h"

using namespace alefbet::pctrl;
using namespace alefbet::pctrl::logger;
using namespace alefbet::pctrl::i18n;
using namespace std::chrono;

/*! @brief Constructs a HistoryUserPanel for the given user.
 * @param user The user data for whom history is being displayed. */
HistoryUserPanel::HistoryUserPanel(const UserData& user) {    
    user_ = user;
}

/*! @brief Destructs a HistoryUserPanel instance and releases resources. */
HistoryUserPanel::~HistoryUserPanel() {      
}

/*! @brief Creates and returns the root UI element for the user history panel.
 * @return A pointer to the root UI element (OverlayFrame). */
tsl::elm::Element* HistoryUserPanel::createUI() {
    rootFrame_ = new tsl::elm::OverlayFrame(tr("Parental Control"), tr("Usage history"));
    rootList_ = new tsl::elm::List();
    
    rebuildUI();        

    return rootFrame_;
}

void HistoryUserPanel::rebuildUI() {
    rootList_->addItem(new tsl::elm::CategoryHeader(tr("Today")));    
    
    duration today_usage = ipc::getUserUsageTime(user_.uid);
    hours today_usage_h = duration_cast<hours>(today_usage);
    duration today_remaining = ipc::getUserRemainingTime(user_.uid);
    hours today_remaining_h = duration_cast<hours>(today_remaining);

    logDebug("Usage: %i, remaining:%i\n", today_usage.count(), today_remaining.count());

    std::string strUsage = std::to_string(today_usage_h.count()) + tr("h") + " " +std::to_string( (today_usage - today_usage_h).count() ) + " " + tr("mn");
    std::string strRemaining = std::to_string(today_remaining_h.count()) + tr("h") + " " +std::to_string( (today_remaining - today_remaining_h).count() ) + " " + tr("mn");

    rootList_->addItem(new tsl::elm::ListItem(tr("Usage"), strUsage));
    rootList_->addItem(new tsl::elm::ListItem(tr("Remaining"), strRemaining));

    rootFrame_->setContent(rootList_);
}

/*! @brief Updates the panel state each frame. */
void HistoryUserPanel::update() {    
}

/*! @brief Handles user input not captured by other UI elements.
 * @param keysDown Keys pressed this frame.
 * @param keysHeld Keys held this frame.
 * @param touchPos Current touch screen position.
 * @param joyStickPosLeft Left analog stick position.
 * @param joyStickPosRight Right analog stick position.
 * @return true if the input was handled, false otherwise. */
bool HistoryUserPanel::handleInput(u64 keysDown, u64 keysHeld, const HidTouchState &touchPos, HidAnalogStickState joyStickPosLeft, HidAnalogStickState joyStickPosRight) {
    if (keysDown & HidNpadButton_B) {
        tsl::goBack();
        return true;
    }

    return false;
}