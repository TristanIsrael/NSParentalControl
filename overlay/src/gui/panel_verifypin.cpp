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

#include "panel_verifypin.h"
#include "logger.h"
#include <switch.h>
#include <mutex>
#include "Command.hpp"
#include "AppContext.h"
#include "helpers/ipc_helpers.h"
#include "panel_setuppin.h"
#include "panel_admin_menu.h"
#include "panel_main_menu.h"

using namespace alefbet::pctrl;

VerifyPinPanel::VerifyPinPanel(const NextPanel& nextPanel) 
: nextPanel_(nextPanel) {    
}

VerifyPinPanel::~VerifyPinPanel() {  
    closeAndClean();
}

void VerifyPinPanel::closeAndClean() {
}

tsl::elm::Element* VerifyPinPanel::createUI() {
    rootFrame_ = new tsl::elm::OverlayFrame("Parental Control", "Verify PIN");
    rootList_ = new tsl::elm::List();
    
    rebuildUI();

    return rootFrame_;
}

void VerifyPinPanel::rebuildUI() {     
    rootList_->addItem(new tsl::elm::CustomDrawer([](tsl::gfx::Renderer *renderer, s32 x, s32 y, s32 w, s32 h) {
        renderer->drawString("Please enter Admin PIN", false, x + 20, y + 20, 20, renderer->a(0xFFFF));            
        //renderer->drawString("Use all pad keys except B", false, x + 20, y + 45, 20, renderer->a(0xFFFF));            
    }), 50);

    rootList_->addItem(new tsl::elm::CategoryHeader("Admin PIN"));
    rootList_->addItem(new tsl::elm::CustomDrawer([this](tsl::gfx::Renderer* renderer, s32 x, s32 y, s32 w, s32 h) {
        u16 spacing = w/5;
        bool active = false;

        for(size_t i = 1 ; i < 5 ; i++) {
            active = verifyPin_.size() >= i;
            renderer->drawCircle(x + i*spacing, y + 15, 15, active, tsl::Color(0xffff));
        }
    }), 30);        
    
    rootList_->addItem(new tsl::elm::CustomDrawer([this](tsl::gfx::Renderer *renderer, s32 x, s32 y, s32 w, s32 h) {
        if(error_) {
            renderer->drawString("PIN is incorrect", false, x + 100, y + 30, 20, renderer->a(RGBA4(0xf, 0x0, 0x0, 0xf)));
            renderer->drawString("Press A to retry, B to cancel", false, x + 30, y + 55, 20, renderer->a(RGBA4(0xf, 0xf, 0xf, 0xf)));
        }
    }), 60);

    rootFrame_->setContent(rootList_);
}

void VerifyPinPanel::update() {    
}

// Called once every frame to handle inputs not handled by other UI elements
bool VerifyPinPanel::handleInput(u64 keysDown, u64 keysHeld, const HidTouchState &touchPos, HidAnalogStickState joyStickPosLeft, HidAnalogStickState joyStickPosRight) {    
    if (keysDown == 0) return false;    

    if (error_) {
        // A to retry
        if(keysDown & HidNpadButton_A) {
            verifyPin_.clear();
            error_ = false;
            return true;
        } 

        // B to cancel
        if(keysDown & HidNpadButton_B) {
            tsl::changeTo<MainMenuPanel>();
            return true;
        } 

        return false;
    }

    if(verifyPin_.size() < 4) {  
        verifyPin_.push_back(keysDown);   
    } 
    
    if(verifyPin_.size() == 4) {
        bool verified = ipc::verifyPin(verifyPin_);

        if(!verified) {
            error_ = true;
        } else {
            goToNextPanel();
        }

        return true;
    }         

    return true; // We capture all input
}

void VerifyPinPanel::goToNextPanel() const {
    switch(nextPanel_) {
        case PanelSetPin: tsl::changeTo<SetupPinPanel>(); break;
        case PanelSetupMenu: tsl::changeTo<AdminMenuPanel>(); break;
    }
}