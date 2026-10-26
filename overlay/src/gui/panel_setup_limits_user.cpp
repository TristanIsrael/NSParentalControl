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
    auto duration = minutes{dailyLimitInMinutes};
    auto hoursPart = duration_cast<hours>(duration);
    auto minutesPart = duration_cast<minutes>(duration - hoursPart);
    dailyLimitHours_ = std::min(hoursPart.count(), 12L);
    dailyLimitMinutes_ = std::min(minutesPart.count(), 59L);

    const u16 bedTimeInMinutes = ipc::getBedtime(user_);
    duration = minutes{bedTimeInMinutes};
    hoursPart = duration_cast<hours>(duration);
    minutesPart = duration_cast<minutes>(duration - hoursPart);
    bedtimeHours_ = std::min(hoursPart.count(), 24L);
    bedtimeMinutes_ = std::min(minutesPart.count(), 59L);

    const u16 gameStartInMinutes = ipc::getGameStartTime(user_);
    duration = minutes{gameStartInMinutes};
    hoursPart = duration_cast<hours>(duration);
    minutesPart = duration_cast<minutes>(duration - hoursPart);
    gameStartHours_ = std::min(hoursPart.count(), 24L);
    gameStartMinutes_ = std::min(minutesPart.count(), 59L);

    rebuildUI();

    return rootFrame_;
}

/*! @brief Rebuilds the UI displaying play limit controls for the user. */
void SetupLimitsUserPanel::rebuildUI() {    
    rootList_->addItem(new tsl::elm::CustomDrawer([this](tsl::gfx::Renderer* renderer, s32 x, s32 y, s32 w, s32 h) {
        renderer->drawString("Please use \u2190 and \u2192 to move.", false, x + 50, y + 30, 16, renderer->a(ColorWhite));
        renderer->drawString("Modify the limits with \u2191 and \u2193.", false, x + 45, y + 55, 16, renderer->a(ColorWhite));
    }), 60);

    // Daily Limit
    rootList_->addItem(new tsl::elm::CategoryHeader("Play limits"));    
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

    // Bed time
    rootList_->addItem(new tsl::elm::CategoryHeader("Bed time"));
    rootList_->addItem(new tsl::elm::CustomDrawer([this](tsl::gfx::Renderer* renderer, s32 x, s32 y, s32 w, s32 h) {
        renderer->drawString("Set time to 0h 0mn to disable.", false, x, y + 30, 16, renderer->a(ColorWhite));
    }), 30);
    rootList_->addItem(new tsl::elm::CustomDrawer([this](tsl::gfx::Renderer* renderer, s32 x, s32 y, s32 w, s32 h) {
        s32 yPos = y;

        yPos += 35;
        renderer->drawString("Stop games after", false, x, yPos, 20, renderer->a(ColorWhite));
        
        // Minutes
        //renderer->drawString("mn", false, w-40, yPos, 20, renderer->a(selectedItem_ == BedtimeMinutes ? ColorSelected : ColorWhite));
        std::string str = std::to_string(bedtimeMinutes_);
        renderer->drawString(str.c_str(), false, w-60 -(bedtimeMinutes_ > 9 ? 10 : 0), yPos, 20, renderer->a(selectedItem_ == BedtimeMinutes ? ColorSelected : ColorWhite));
                
        // Hours
        renderer->drawString(":", false, w-90, yPos, 20, renderer->a(selectedItem_ == BedtimeHours ? ColorSelected : ColorWhite));
        str = std::to_string(bedtimeHours_);
        renderer->drawString(str.c_str(), false, w-110 -(bedtimeHours_ > 9 ? 10 : 0), yPos, 20, renderer->a(selectedItem_ == BedtimeHours ? ColorSelected : ColorWhite));            

    }), 60);

    // Game start time
    rootList_->addItem(new tsl::elm::CategoryHeader("Play time"));
    rootList_->addItem(new tsl::elm::CustomDrawer([this](tsl::gfx::Renderer* renderer, s32 x, s32 y, s32 w, s32 h) {
        renderer->drawString("Set time to 0h 0mn to disable.", false, x, y + 30, 16, renderer->a(ColorWhite));
    }), 30);
    rootList_->addItem(new tsl::elm::CustomDrawer([this](tsl::gfx::Renderer* renderer, s32 x, s32 y, s32 w, s32 h) {
        s32 yPos = y;

        yPos += 35;
        renderer->drawString("Allow games from", false, x, yPos, 20, renderer->a(ColorWhite));
        
        // Minutes
        std::string str = std::to_string(gameStartMinutes_);
        renderer->drawString(str.c_str(), false, w-60 -(gameStartMinutes_ > 9 ? 10 : 0), yPos, 20, renderer->a(selectedItem_ == GameStartMinutes ? ColorSelected : ColorWhite));
                
        // Hours
        renderer->drawString(":", false, w-90, yPos, 20, renderer->a(selectedItem_ == GameStartHours ? ColorSelected : ColorWhite));
        str = std::to_string(gameStartHours_);
        renderer->drawString(str.c_str(), false, w-110 -(gameStartHours_ > 9 ? 10 : 0), yPos, 20, renderer->a(selectedItem_ == GameStartHours ? ColorSelected : ColorWhite));            

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
        // Update the setting only when exitting
        u32 limitInMinutes = dailyLimitHours_*60 + dailyLimitMinutes_;        
        ipc::setDailyLimit(user_, limitInMinutes);
        logDebug("limitInMinutes=%i, hours=%i, minutes=%i\n", limitInMinutes, dailyLimitHours_, dailyLimitMinutes_);

        u32 bedtime = bedtimeHours_*60 + bedtimeMinutes_;
        ipc::setBedtime(user_, bedtime);
        logDebug("bedTime=%i, hours=%i, minutes=%i\n", bedtime, bedtimeHours_, bedtimeMinutes_);

        u32 gameStart = gameStartHours_*60 + gameStartMinutes_;
        ipc::setGameStartTime(user_, gameStart);
        logDebug("gameStart=%i, hours=%i, minutes=%i\n", gameStart, gameStartHours_, gameStartMinutes_);

        tsl::goBack();
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
        case DailyLimitMinutes: selectedItem_ = BedtimeHours; break;
        case BedtimeHours: selectedItem_ = BedtimeMinutes; break;
        case BedtimeMinutes: selectedItem_ = GameStartHours; break;
        case GameStartHours: selectedItem_ = GameStartMinutes; break;
        case GameStartMinutes: selectedItem_ = DailyLimitHours; break;
        default: break;
    }
}

/*! @brief Selects the previous item in the limit fields (minutes <-> hours). */
void SetupLimitsUserPanel::selectBeforeItem() {
    switch(selectedItem_) {
        case GameStartMinutes: selectedItem_ = GameStartHours; break;
        case GameStartHours: selectedItem_ = BedtimeMinutes; break;
        case BedtimeMinutes: selectedItem_ = BedtimeHours; break;
        case BedtimeHours: selectedItem_ = DailyLimitMinutes; break;
        case DailyLimitMinutes: selectedItem_ = DailyLimitHours; break;
        case DailyLimitHours: selectedItem_ = GameStartMinutes; break;
        default: break;
    }
}

/*! @brief Decreases the currently selected limit value by one unit. */
void SetupLimitsUserPanel::decreaseValue() {
    switch(selectedItem_) {
        case DailyLimitHours: dailyLimitHours_ = valueRanged(dailyLimitHours_, -1, 0, 12); break;
        case DailyLimitMinutes: dailyLimitMinutes_ = valueRanged(dailyLimitMinutes_, -1, 0, 59); break;
        case BedtimeHours: bedtimeHours_ = valueRanged(bedtimeHours_, -1, 0, 23); break;
        case BedtimeMinutes: bedtimeMinutes_ = valueRanged(bedtimeMinutes_, -1, 0, 59); break;
        case GameStartHours: gameStartHours_ = valueRanged(gameStartHours_, -1, 0, 23); break;
        case GameStartMinutes: gameStartMinutes_ = valueRanged(gameStartMinutes_, -1, 0, 59); break;
        default: break;
    }
}

/*! @brief Increases the currently selected limit value by one unit. */
void SetupLimitsUserPanel::increaseValue() {
    switch(selectedItem_) {
        case DailyLimitHours: dailyLimitHours_ = valueRanged(dailyLimitHours_, +1, 0, 12); break;
        case DailyLimitMinutes: dailyLimitMinutes_ = valueRanged(dailyLimitMinutes_, +1, 0, 59); break;
        case BedtimeHours: bedtimeHours_ = valueRanged(bedtimeHours_, +1, 0, 23); break;
        case BedtimeMinutes: bedtimeMinutes_ = valueRanged(bedtimeMinutes_, +1, 0, 59); break;
        case GameStartHours: gameStartHours_ = valueRanged(gameStartHours_, +1, 0, 23); break;
        case GameStartMinutes: gameStartMinutes_ = valueRanged(gameStartMinutes_, +1, 0, 59); break;
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
    int newVal = value + diff;

    if(newVal < min) {
        return static_cast<u16>(cycle ? max : min);
    } else if(newVal > max) {
        return static_cast<u16>(cycle ? min : max);
    } else {
        return static_cast<u16>(newVal);
    }
}