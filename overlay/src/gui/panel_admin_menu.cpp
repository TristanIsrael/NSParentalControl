#include "panel_admin_menu.h"
#include "logger.h"
#include <switch.h>
#include "Command.hpp"
#include "AppContext.h"
#include "version.h"
#include "logger.h"
#include "helpers/ipc_helpers.h"
#include "panel_setuppin.h"
#include "panel_main_menu.h"
#include "panel_setup_limits_main.h"
#include "panel_blacklist_main.h"

using namespace alefbet::pctrl;
using namespace alefbet::pctrl::logger;

AdminMenuPanel::AdminMenuPanel() {    
}

AdminMenuPanel::~AdminMenuPanel() {      
}

void AdminMenuPanel::closeAndClean() {
}

tsl::elm::Element* AdminMenuPanel::createUI() {        
    rootFrame_ = new tsl::elm::OverlayFrame("Parental Control", "Administration");
    rootList_ = new tsl::elm::List();
    
    rebuildUI();

    return rootFrame_;
}


void AdminMenuPanel::rebuildUI() {

    // Enable parental control
    auto entryEnabled = new tsl::elm::ToggleListItem("Enabled", getAppContext().is_enabled, "Enabled", "Disabled");
    rootList_->addItem(entryEnabled);        
    entryEnabled->setStateChangedListener([entryEnabled] (bool enabled) -> bool {
        bool res = ipc::setEnabled(enabled);
        if(!res) {
            entryEnabled->setState(!entryEnabled->getState());
        }

        return true;
    });    

    // Set working mode
    /*const auto& workingMode = ipc::getWorkingMode();

    auto entrySetWorkingMode = new tsl::elm::ToggleListItem("Working mode", workingMode, "Blocking", "Information");
    rootList_->addItem(entrySetWorkingMode);
    entrySetWorkingMode->setClickListener([entrySetWorkingMode](u64 keys) -> bool {
        if(keys & HidNpadButton_A) {            
            bool res = ipc::setWorkingMode(entrySetWorkingMode->getState() ? ipc::WorkingModeBlocking : ipc::WorkingModeInfo);
            if(!res) {
                entrySetWorkingMode->setState(!entrySetWorkingMode->getState());
            }
            return true;
        }

        return false;
    });*/
    auto entrySetWorkingMode = new tsl::elm::ListItem("WorkingMode", "Blocking");
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
    
    // Blacklist
    auto entryBlacklist = new tsl::elm::ListItem("Blacklist");
    rootList_->addItem(entryBlacklist);
    entryBlacklist->setClickListener([](u64 keys) -> bool {
        if(keys & HidNpadButton_A) {
            tsl::changeTo<SetupBlacklistPanel>();
            return true;
        }

        return false;
    });

    rootList_->addItem(new tsl::elm::CategoryHeader("Versions"));
    rootList_->addItem(new tsl::elm::ListItem("Overlay", std::string(VERSION)));
    rootList_->addItem(new tsl::elm::ListItem("Sysmodule", ipc::getVersion()));        

    rootFrame_->setContent(rootList_);
}

void AdminMenuPanel::update() {
    
}

// Called once every frame to handle inputs not handled by other UI elements
bool AdminMenuPanel::handleInput(u64 keysDown, u64 keysHeld, const HidTouchState &touchPos, HidAnalogStickState joyStickPosLeft, HidAnalogStickState joyStickPosRight) {
    if (keysDown & HidNpadButton_B) {
        tsl::changeTo<MainMenuPanel>();
        return true;
    }

    return false;
}