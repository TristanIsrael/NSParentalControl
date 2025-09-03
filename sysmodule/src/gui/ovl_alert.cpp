#include <string>
#include "tesla.hpp"
#include "ovl_alert.h"
#include "structs.h"

tsl::elm::Element* PanelLimitReached::createUI() {
    rootFrame_ = new tsl::elm::OverlayFrame("Parental Control", "Time limit");    

    rebuildUI();

    return rootFrame_;
}

void PanelLimitReached::rebuildUI() {    
    ParentalControlState state = getParentalControlState();
    if(!state.acknowledged || !state.active) return; //Show the GUI only when it is active and not acknowledged

    std::string alert_text = "";
    switch(state.alert_type) {
        case ALERT_LIMIT_ALMOST_REACHED: alert_text = std::string("The playing limit is almost reached. Please quit the game."); break;
        case ALERT_LIMIT_REACHED: alert_text = std::string("The playing limit is reached. The game will be quit."); break;
        default: alert_text = std::string("ERR#1");
    }

    // Texte centré
    auto content = new tsl::elm::CustomDrawer([alert_text, state](tsl::gfx::Renderer *renderer, s32 x, s32 y, s32 w, s32 h) {
        renderer->drawString(alert_text.c_str(),
            false,
            w / 2,
            h / 2 - 20,
            30,
            ColorError,
            w);

        if(state.alert_type == ALERT_LIMIT_ALMOST_REACHED) {            
            std::string remaining_text = std::string("Remaining : " +std::to_string(state.today_time_remaining_in_secs) +std::string(" seconds"));
            renderer->drawString(remaining_text.c_str(),
                false,
                w / 2,
                h / 2 + 20,
                30,
                tsl::style::color::ColorText,
                w);
        }
    });    
    
    rootFrame_->setContent(content);
}

void PanelLimitReached::update() {    
}

// Called once every frame to handle inputs not handled by other UI elements
bool PanelLimitReached::handleInput(u64 keysDown, u64 keysHeld, const HidTouchState &touchPos, HidAnalogStickState joyStickPosLeft, HidAnalogStickState joyStickPosRight) {
    if(keysDown & HidNpadButton_B) {
        ParentalControlState state = getParentalControlState();
        state.acknowledged = false;
        setParentalControlState(state);
        
        rootFrame_->invalidate();

        return true;
    }

    return false;
}


OverlayAlert::OverlayAlert() {
}

void OverlayAlert::initServices() {
    hidInitialize();
    appletInitialize();
}

void OverlayAlert::exitServices() {
    appletExit();
    hidExit();
}    

std::unique_ptr<tsl::Gui> OverlayAlert::loadInitialGui() {
    return initially<PanelLimitReached>(); 
}