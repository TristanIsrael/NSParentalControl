#include "panel_main_menu.h"
//#include <tesla.hpp>
#include <switch.h>
#include <chrono>
#include "version.h"
#include "logger.h"
#include "Command.hpp"
#include "panel_debug_menu.h"
#include "panel_admin_menu.h"
#include "panel_verifypin.h"
#include "panel_setup_limits_main.h"
#include "panel_history_main.h"
#include "AppContext.h"
#include "helpers/ipc_helpers.h"

using namespace alefbet::pctrl;
using namespace alefbet::pctrl::logger;
using namespace std::chrono;

MainMenuPanel::MainMenuPanel() {    
}

MainMenuPanel::~MainMenuPanel() {      
}

void MainMenuPanel::closeAndClean() {
}

bool MainMenuPanel::isParentalControlEnabled() {
    getAppContext().is_enabled = ipc::isEnabled();
    return getAppContext().is_enabled;
}

bool MainMenuPanel::isParentalControlInstalled() {
    return getAppContext().is_available;
}

std::list<std::string> MainMenuPanel::getUsersList() {
    std::list<std::string> users;

    return users;
}

tsl::elm::Element* MainMenuPanel::createUI() {
    std::string subTitle = isParentalControlInstalled() ? isParentalControlEnabled() ? "Parental Control is Enabled" : "Parental Control is Disabled" : "Parental Control is not installed";

    rootFrame_ = new tsl::elm::OverlayFrame("Parental Control", subTitle);
    rootList_ = new tsl::elm::List();
    
    rebuildUI();        

    return rootFrame_;
}


void MainMenuPanel::rebuildUI() {
    // Special place for special messages (database tampered / needs upgrade)
    bool specialMessages = false;
    const auto& dbTampered = ipc::isDatabaseTampered();
    const auto& mustUpgrade = ipc::isDatabaseNeedsUpgrade();
    specialMessages = dbTampered || mustUpgrade;
    if(specialMessages) {
        //rootList_->addItem(new tsl::elm::CategoryHeader("Warnings"));
        rootList_->addItem(new tsl::elm::CustomDrawer([](tsl::gfx::Renderer *r, s32 x, s32 y, s32 width, s32 height) {
                r->drawString(" ", false, x, y, 22, tsl::Color(0xffff));
            }), 30);
        
        if(dbTampered) {
            rootList_->addItem(new tsl::elm::CustomDrawer([](tsl::gfx::Renderer *r, s32 x, s32 y, s32 width, s32 height) {
                std::vector<std::string> symbols;
                symbols.push_back("\ue150");
                r->drawStringWithColoredSections("\ue150 Database tampered!", false, symbols, x, y, 22, tsl::Color(0xffff), tsl::Color(0xf00f));                    
            }), 30);
        }

        if(mustUpgrade) {
            rootList_->addItem(new tsl::elm::CustomDrawer([](tsl::gfx::Renderer *r, s32 x, s32 y, s32 width, s32 height) {
                std::vector<std::string> symbols;
                symbols.push_back("\ue150");
                r->drawStringWithColoredSections("\ue150 Database needs upgrade!", false, symbols, x, y, 22, tsl::Color(0xffff), tsl::Color(0xf00f));                    
            }), 30);
        }
    }


    // Current User
    auto currentTitle = ipc::getCurrentTitle();
    logDebug("title=%s\n", currentTitle.c_str());

    if(!currentTitle.empty() && !currentTitle.starts_with("Err#")) {
        auto currentUserUid = ipc::getCurrentUserUid();
        logDebug("user id=%s\n", currentUserUid.c_str());

        auto currentUserNickname = ipc::getCurrentUserNickname();        
        logDebug("user name=%s\n", currentUserNickname.c_str());        

        auto entryUser = new tsl::elm::ListItem("User: " +currentUserNickname);
        rootList_->addItem(entryUser);        

        // Current game        
        auto entryTitle = new tsl::elm::ListItem("Playing: " +currentTitle);
        rootList_->addItem(entryTitle);

        // Usage time        
        auto usageTimeInMinutes = ipc::getUserUsageTime();
        auto durationInMinutes = minutes{usageTimeInMinutes};
        auto hoursPart = duration_cast<hours>(durationInMinutes);
        auto minutesPart = duration_cast<minutes>(durationInMinutes - hoursPart);
        auto entryUsageTime = new tsl::elm::ListItem("Usage: " +(hoursPart.count() > 0 ? std::to_string(hoursPart.count()) + "h " : "") +std::to_string(minutesPart.count()) +" mn");
        rootList_->addItem(entryUsageTime);

        // Remaining time
        auto remainingTime = ipc::getUserRemainingTime();
        durationInMinutes = minutes{remainingTime};
        hoursPart = duration_cast<hours>(durationInMinutes);
        minutesPart = duration_cast<minutes>(durationInMinutes - hoursPart);
        auto entryRemainingTime = new tsl::elm::ListItem("Remaining: " +(hoursPart.count() > 0 ? std::to_string(hoursPart.count()) + "h " : "") +std::to_string(minutesPart.count()) +" mn");
        rootList_->addItem(entryRemainingTime);        
    } else {
        rootList_->addItem(new tsl::elm::ListItem("No user / app started"));
    }        

    if(false) { // Disabled
        const auto& remainingTimeVisibility = ipc::getShowRemainingTime();
        auto entryShowRemainingTime = new tsl::elm::ToggleListItem("Remaining time visible", remainingTimeVisibility);
        rootList_->addItem(entryShowRemainingTime);
        entryShowRemainingTime->setClickListener([this, entryShowRemainingTime](u64 keys) -> bool {
            if(keys & HidNpadButton_A) {            
                bool res = ipc::setShowRemainingTime(entryShowRemainingTime->getState());
                if(!res) {
                    entryShowRemainingTime->setState(!entryShowRemainingTime->getState());
                } else {
                    dirty_ = true;
                }
                
                return true;
            }

            return false;
        }); 
    }

    // History
    auto entryShowHistory = new tsl::elm::ListItem("Usage history");
    rootList_->addItem(entryShowHistory);
    entryShowHistory->setClickListener([this, entryShowHistory](u64 keys) -> bool {
        if(keys & HidNpadButton_A) {            
            tsl::changeTo<HistoryMainPanel>();
            return true;
        }

        return false;
    }); 
    
    // Debug menu
    if(getAppContext().is_debug) {
        auto entryDebugMenu = new tsl::elm::ListItem("Debug");
        rootList_->addItem(entryDebugMenu);        
        entryDebugMenu->setClickListener([this](u64 keys) {
            if(keys & HidNpadButton_A) {
                tsl::changeTo<DebugMenu>();
                return true;
            }

            return false;
        });
    }

    // Admin menu
    auto entrySetupMenu = new tsl::elm::ListItem("Admin");
    rootList_->addItem(entrySetupMenu);        
    entrySetupMenu->setClickListener([this](u64 keys) {
        if(keys & HidNpadButton_A) {
            tsl::changeTo<VerifyPinPanel, NextPanel>(PanelSetupMenu);
            //tsl::changeTo<SetupLimitsPanel>();
            return true;
        }

        return false;
    });

    rootFrame_->setContent(rootList_);
}

void MainMenuPanel::update() {    
}

// Called once every frame to handle inputs not handled by other UI elements
bool MainMenuPanel::handleInput(u64 keysDown, u64 keysHeld, const HidTouchState &touchPos, HidAnalogStickState joyStickPosLeft, HidAnalogStickState joyStickPosRight) {
    if (keysDown & HidNpadButton_B) {
        tsl::Overlay::get()->close();
        return true;
    }

    return false;
}