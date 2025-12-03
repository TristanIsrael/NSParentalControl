#include "panel_setup_limits_main.h"
#include <switch.h>
#include <chrono>
#include "version.h"
#include "logger.h"
#include "Command.hpp"
#include "AppContext.h"
#include "helpers/ipc_helpers.h"
#include "helpers/switch_helpers.h"
#include "panel_setup_limits_user.h"

using namespace alefbet::pctrl;
using namespace alefbet::pctrl::helpers;

SetupLimitsPanel::SetupLimitsPanel() {    
}

SetupLimitsPanel::~SetupLimitsPanel() {      
}

tsl::elm::Element* SetupLimitsPanel::createUI() {
    rootFrame_ = new tsl::elm::OverlayFrame("Parental Control", "Set limits");
    rootList_ = new tsl::elm::List();
    
    rebuildUI();        

    return rootFrame_;
}


void SetupLimitsPanel::rebuildUI() {
    rootList_->addItem(new tsl::elm::CategoryHeader("Choose a user"));

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

void SetupLimitsPanel::update() {    
}

// Called once every frame to handle inputs not handled by other UI elements
bool SetupLimitsPanel::handleInput(u64 keysDown, u64 keysHeld, const HidTouchState &touchPos, HidAnalogStickState joyStickPosLeft, HidAnalogStickState joyStickPosRight) {
    if (keysDown & HidNpadButton_B) {
        tsl::goBack();
        return true;
    }

    return false;
}