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

#pragma once

#include "switch_helpers.h"
#include <tesla.hpp>
#include <switch.h>

typedef enum {
    NoItem = 0,
    DailyLimitHours = 1,
    DailyLimitMinutes = 2,
    BedtimeHours = 3,
    BedtimeMinutes = 4,
    GameStartHours = 5,
    GameStartMinutes = 6
} SelectedItem;

using namespace alefbet::pctrl::helpers;

class SetupLimitsUserPanel : public tsl::Gui {
public:
    SetupLimitsUserPanel(const UserData& user);
    ~SetupLimitsUserPanel();

    tsl::elm::Element* createUI() override;
    void rebuildUI();
    void update() override;
    void closeAndClean();

    // Called once every frame to handle inputs not handled by other UI elements
    virtual bool handleInput(u64 keysDown, u64 keysHeld, const HidTouchState &touchPos, HidAnalogStickState joyStickPosLeft, HidAnalogStickState joyStickPosRight) override;

private:
    void selectNextItem();
    void selectBeforeItem();
    void decreaseValue();
    void increaseValue();
    u16 valueRanged(int value, int diff, int min, int max, bool cycle = true);

private:    
    UserData user_;
    tsl::elm::OverlayFrame* rootFrame_ = nullptr;
    tsl::elm::List* rootList_ = nullptr;
    bool error_ = false;
    SelectedItem selectedItem_ = DailyLimitHours;
    u16 dailyLimitHours_ = 0;
    u16 dailyLimitMinutes_ = 0;
    u16 bedtimeHours_ = 0;
    u16 bedtimeMinutes_ = 0;
    u16 gameStartHours_ = 0;
    u16 gameStartMinutes_ = 0;
};