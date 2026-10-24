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

#include "switch.h"
#include <tesla.hpp>

typedef enum {
    NO_ERROR = 0,
    VERIFY_ERROR = 1,
    SERVICE_ERROR = 2
} PinError;

class SetupPinPanel : public tsl::Gui {
public:
    SetupPinPanel();
    ~SetupPinPanel();

    tsl::elm::Element* createUI() override;
    void rebuildUI();
    void update() override;
    void closeAndClean();

    // Called once every frame to handle inputs not handled by other UI elements
    virtual bool handleInput(u64 keysDown, u64 keysHeld, const HidTouchState &touchPos, HidAnalogStickState joyStickPosLeft, HidAnalogStickState joyStickPosRight) override;

private:    
    tsl::elm::OverlayFrame* rootFrame_ = nullptr;
    tsl::elm::List* rootList_ = nullptr;
    //std::vector<u64> currentPin_;
    std::vector<u64> newPin_;
    std::vector<u64> verifyPin_;
    PinError error_ = NO_ERROR;
};