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

#include "panel_setuppin.h"
#include "logger.h"
#include <switch.h>
#include <mutex>
#include "Command.hpp"
#include "AppContext.h"
#include "helpers/ipc_helpers.h"
#include "panel_admin_menu.h"
#include "i18n.h"

using namespace alefbet::pctrl;
using namespace alefbet::pctrl::i18n;

static std::mutex updateMutex_;
constexpr tsl::Color ColorReady = tsl::Color(0xffff);
constexpr tsl::Color ColorNotReady = tsl::Color(RGBA4(0x7, 0x7, 0x7, 0xf));

/*! @brief Constructs a SetupPinPanel instance. */
SetupPinPanel::SetupPinPanel() {}

/*! @brief Destructs a SetupPinPanel instance and releases resources. */
SetupPinPanel::~SetupPinPanel() {  
    closeAndClean();
}

/*! @brief Closes the panel and cleans up associated resources. */
void SetupPinPanel::closeAndClean() {
}

/*! @brief Creates and returns the root UI element for the PIN setup panel.
 * @return A pointer to the root UI element (OverlayFrame). */
tsl::elm::Element* SetupPinPanel::createUI() {
    rootFrame_ = new tsl::elm::OverlayFrame(tr(Key::AppTitle), tr(Key::SetupPin));
    rootList_ = new tsl::elm::List();
    
    rebuildUI();

    return rootFrame_;
}

/*! @brief Rebuilds the PIN entry UI with circle indicators and error messages. */
void SetupPinPanel::rebuildUI() {    
    rootList_->addItem(new tsl::elm::CustomDrawer([this](tsl::gfx::Renderer* renderer, s32 x, s32 y, s32 w, s32 h) {
        renderer->drawString(tr(Key::EnterNewPin), false, x + 20, y + 20, 20, renderer->a(0xFFFF));  
        //renderer->drawString("Use all pad keys except B", false, 0, 0, 20, renderer->a(0xff0f));
    }), 30);

    /*rootList_->addItem(new tsl::elm::CategoryHeader("Current PIN"));    
    rootList_->addItem(new tsl::elm::CustomDrawer([this](tsl::gfx::Renderer* renderer, s32 x, s32 y, s32 w, s32 h) {
        u16 spacing = w/5;
        bool active = false;        

        for(size_t i = 1 ; i < 5 ; i++) {
            active = currentPin_.size() >= i;
            renderer->drawCircle(x + i*spacing, y + 15, 15, active, ColorReady);
        }
    }), 30);*/

    rootList_->addItem(new tsl::elm::CategoryHeader(tr(Key::NewPin)));
    rootList_->addItem(new tsl::elm::CustomDrawer([this](tsl::gfx::Renderer* renderer, s32 x, s32 y, s32 w, s32 h) {
        u16 spacing = w/5;
        bool active = false;

        for(size_t i = 1 ; i < 5 ; i++) {
            active = newPin_.size() >= i;
            renderer->drawCircle(x + i*spacing, y + 15, 15, active, ColorReady);
        }
    }), 30);

    rootList_->addItem(new tsl::elm::CategoryHeader(tr(Key::VerifyPin)));
    rootList_->addItem(new tsl::elm::CustomDrawer([this](tsl::gfx::Renderer* renderer, s32 x, s32 y, s32 w, s32 h) {
        u16 spacing = w/5;
        bool active = false;

        for(size_t i = 1 ; i < 5 ; i++) {
            active = verifyPin_.size() >= i;
            renderer->drawCircle(x + i*spacing, y + 15, 15, active, newPin_.size() < 4 ? ColorNotReady : ColorReady);
        }
    }), 30);    

    rootList_->addItem(new tsl::elm::CustomDrawer([this](tsl::gfx::Renderer* renderer, s32 x, s32 y, s32 w, s32 h) {
        if(error_ != NO_ERROR) {
            const char* lblError = error_ == VERIFY_ERROR ? tr(Key::PinsMismatch) : tr(Key::UnknownError);

            renderer->drawString(lblError, false, x + 90, y + 30, 20, renderer->a(RGBA4(0xf, 0x0, 0x0, 0xf)));
            renderer->drawString(tr(Key::PressARetry), false, x + 50, y + 55, 20, renderer->a(RGBA4(0xf, 0xf, 0xf, 0xf)));
        }
    }), 60);
    
    rootFrame_->setContent(rootList_);
}

/*! @brief Updates the panel state each frame. */
void SetupPinPanel::update() {    
    
}

/*! @brief Handles user input not captured by other UI elements for PIN entry.
 * @param keysDown Keys pressed this frame.
 * @param keysHeld Keys held this frame.
 * @param touchPos Current touch screen position.
 * @param joyStickPosLeft Left analog stick position.
 * @param joyStickPosRight Right analog stick position.
 * @return true if the input was handled, false otherwise. */
bool SetupPinPanel::handleInput(u64 keysDown, u64 keysHeld, const HidTouchState &touchPos, HidAnalogStickState joyStickPosLeft, HidAnalogStickState joyStickPosRight) {
    if (keysDown == 0) return false;    

    if (error_) {
        // A to retry
        if(keysDown & HidNpadButton_A) {
            verifyPin_.clear();
            newPin_.clear();
            error_ = NO_ERROR;
            return true;
        } 

        // B to cancel
        if(keysDown & HidNpadButton_B) {
            tsl::goBack();
            return true;
        } 

        return false;
    }

    if(newPin_.size() < (size_t)4) {
        newPin_.push_back(keysDown);
    } else if(verifyPin_.size() < (size_t)4) {
        verifyPin_.push_back(keysDown);            

        if(/*currentPin_.size() == 4 &&*/ newPin_.size() == 4 && verifyPin_.size() == 4) {
            // Step 1: verify current PIN -> REMOVED
            // Step 2: verify whether new PINs match
            // Step 3: send new PIN to the sysmodule
            // Step 4: handle sysmodule error

            bool verified = newPin_ == verifyPin_;
            if(!verified) {
                error_ = VERIFY_ERROR;
            } else {
                bool res = ipc::setupPin(newPin_);
                if(!res) {
                    error_ = SERVICE_ERROR;
                } else {
                    tsl::goBack();
                }                
            }
        }
                    
    }

    return true; // We capture all input
}