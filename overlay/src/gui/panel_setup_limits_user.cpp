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

#include "panel_setup_limits_user.h"
#include <switch.h>
#include <mutex>
#include <chrono>
#include <algorithm>
#include "logger.h"
#include "Command.hpp"
#include "AppContext.h"
#include "helpers/ipc_helpers.h"
#include "panel_admin_menu.h"

using namespace alefbet::pctrl;
using namespace alefbet::pctrl::logger;
using namespace std::chrono;

constexpr tsl::Color ColorSelected = tsl::style::color::ColorHighlight;
constexpr tsl::Color ColorWhite = tsl::Color(0xffff);

/*! @brief Constructs a SetupLimitsUserPanel for the given user.
 * @param user The user data for whom limits are being set. */
SetupLimitsUserPanel::SetupLimitsUserPanel(const UserData& user) {
    user_ = user;
}

/*! @brief Destructs a SetupLimitsUserPanel instance and releases resources. */
SetupLimitsUserPanel::~SetupLimitsUserPanel() {  
    closeAndClean();
}

/*! @brief Closes the panel and cleans up associated resources. */
void SetupLimitsUserPanel::closeAndClean() {
}

/*! @brief Creates and returns the root UI element for the user limits panel.
 * @return A pointer to the root UI element (OverlayFrame). */
tsl::elm::Element* SetupLimitsUserPanel::createUI() {
    rootFrame_ = new tsl::elm::OverlayFrame("Parental Control", "Set limits for " +user_.nickname);
    rootList_ = new tsl::elm::List();
    
    // Load settings
    const u16 dailyLimitInMinutes = ipc::getDailyLimit(user_);
    const auto duration = minutes{dailyLimitInMinutes};
    auto hoursPart = duration_cast<hours>(duration);
    auto minutesPart = duration_cast<minutes>(duration - hoursPart);
    dailyLimitHours_ = std::min(hoursPart.count(), 12ULL);
    dailyLimitMinutes_ = std::min(minutesPart.count(), 59ULL);

    rebuildUI();

    return rootFrame_;
}

/*! @brief Rebuilds the UI displaying play limit controls for the user. */
void SetupLimitsUserPanel::rebuildUI() {    
    rootList_->addItem(new tsl::elm::CustomDrawer([this](tsl::gfx::Renderer* renderer, s32 x, s32 y, s32 w, s32 h) {
        renderer->drawString("Please use \u2190 and \u2192 to move.", false, x + 50, y + 30, 16, renderer->a(ColorWhite));
        renderer->drawString("Modify the limits with \u2191 and \u2193.", false, x + 45, y + 55, 16, renderer->a(ColorWhite));
    }), 60);

    rootList_->addItem(new tsl::elm::CategoryHeader("Play limits"));
    
    // Daily Limit
    rootList_->addItem(new tsl::elm::CustomDrawer([this](tsl::gfx::Renderer* renderer, s32 x, s32 y, s32 w, s32 h) {
        s32 yPos = y;

        yPos += 35;
        renderer->drawString("Daily", false, x, yPos, 20, renderer->a(ColorWhite));
        
        // Minutes
        renderer->drawString("mn", false, w-40, yPos, 20, renderer->a(selectedItem_ == DailyLimitMinutes ? ColorSelected : ColorWhite));
        std::string str = std::to_string(dailyLimitMinutes_);
        renderer->drawString(str.c_str(), false, w-60 -(dailyLimitMinutes_ > 9 ? 10 : 0), yPos, 20, renderer->a(selectedItem_ == DailyLimitMinutes ? ColorSelected : ColorWhite));
                
        // Hours
        renderer->drawString("h", false, w-90, yPos, 20, renderer->a(selectedItem_ == DailyLimitHours ? ColorSelected : ColorWhite));
        str = std::to_string(dailyLimitHours_);
        renderer->drawString(str.c_str(), false, w-110 -(dailyLimitHours_ > 9 ? 10 : 0), yPos, 20, renderer->a(selectedItem_ == DailyLimitHours ? ColorSelected : ColorWhite));        
        
    }), 60);

    
    rootFrame_->setContent(rootList_);
}

/*! @brief Updates the panel state each frame. */
void SetupLimitsUserPanel::update() {    
    
}

/*! @brief Handles user input for navigating and adjusting play limits.
 * @param keysDown Keys pressed this frame.
 * @param keysHeld Keys held this frame.
 * @param touchPos Current touch screen position.
 * @param joyStickPosLeft Left analog stick position.
 * @param joyStickPosRight Right analog stick position.
 * @return true if the input was handled, false otherwise. */
bool SetupLimitsUserPanel::handleInput(u64 keysDown, u64 keysHeld, const HidTouchState &touchPos, HidAnalogStickState joyStickPosLeft, HidAnalogStickState joyStickPosRight) {
    if (keysDown == 0) return false;    

    // B to cancel
    if(keysDown & HidNpadButton_B) {
        tsl::goBack();

        // Update the setting only when exitting
        u16 limitInMinutes = (u16)dailyLimitHours_*60 + (u16)dailyLimitMinutes_;
        logDebug("limitInMinutes=%i, hours=%i, minutes=%i\n", limitInMinutes, dailyLimitHours_, dailyLimitMinutes_);
        ipc::setDailyLimit(user_, limitInMinutes);

        return true;
    } else if(keysDown & HidNpadButton_AnyDown) {
        decreaseValue();
    } else if(keysDown & HidNpadButton_AnyUp) {
        increaseValue();
    } else if(keysDown & HidNpadButton_AnyLeft) {
        selectBeforeItem();
    } else if(keysDown & HidNpadButton_AnyRight) {
        selectNextItem();
    }
    
    return false; 
}

/*! @brief Selects the next item in the limit fields (hours <-> minutes). */
void SetupLimitsUserPanel::selectNextItem() {
    switch(selectedItem_) {
        case DailyLimitHours: selectedItem_ = DailyLimitMinutes; break;
        case DailyLimitMinutes: selectedItem_ = DailyLimitHours; break;
        default: break;
    }
}

/*! @brief Selects the previous item in the limit fields (minutes <-> hours). */
void SetupLimitsUserPanel::selectBeforeItem() {
    switch(selectedItem_) {
        case DailyLimitHours: selectedItem_ = DailyLimitMinutes; break;
        case DailyLimitMinutes: selectedItem_ = DailyLimitHours; break;
        default: break;
    }
}

/*! @brief Decreases the currently selected limit value by one unit. */
void SetupLimitsUserPanel::decreaseValue() {
    switch(selectedItem_) {
        case DailyLimitHours: dailyLimitHours_ = valueRanged(dailyLimitHours_, -1, 0, 12); break;
        case DailyLimitMinutes: dailyLimitMinutes_ = valueRanged(dailyLimitMinutes_, -1, 0, 59); break;
        default: break;
    }
}

/*! @brief Increases the currently selected limit value by one unit. */
void SetupLimitsUserPanel::increaseValue() {
    switch(selectedItem_) {
        case DailyLimitHours: dailyLimitHours_ = valueRanged(dailyLimitHours_, +1, 0, 12); break;
        case DailyLimitMinutes: dailyLimitMinutes_ = valueRanged(dailyLimitMinutes_, +1, 0, 59); break;
        default: break;
    }
}

/*! @brief Clamps a value within a specified range, optionally wrapping around.
 * @param value The current value.
 * @param diff The difference to add to the value.
 * @param min The minimum allowed value.
 * @param max The maximum allowed value.
 * @param cycle Whether to wrap around at boundaries (default false).
 * @return The clamped (or wrapped) value as an unsigned 16-bit integer. */
u16 SetupLimitsUserPanel::valueRanged(int value, int diff, int min, int max, bool cycle) {
    if(value + diff < min) {
        return (u16)(cycle ? max : min);
    } else if(value + diff > max) {
        return (u16)(cycle ? min : max);
    } else {
        return (u16)(value + diff);
    }
}