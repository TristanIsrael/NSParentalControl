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

#include "panel_main_menu.h"
//#include <tesla.hpp>
#include <switch.h>
#include <chrono>
#include <limits>
#include "logger.h"
#include "Command.hpp"
#include "panel_debug_menu.h"
#include "panel_admin_menu.h"
#include "panel_verifypin.h"
#include "panel_setup_limits_main.h"
#include "panel_history_main.h"
#include "AppContext.h"
#include "helpers/ipc_helpers.h"
#include "helpers/switch_helpers.h"

using namespace alefbet::pctrl;
using namespace alefbet::pctrl::logger;
using namespace std::chrono;

/*! @brief Constructs a MainMenuPanel instance. */
MainMenuPanel::MainMenuPanel() {    
}

/*! @brief Destructs a MainMenuPanel instance and releases resources. */
MainMenuPanel::~MainMenuPanel() {      
}

/*! @brief Closes the panel and cleans up associated resources. */
void MainMenuPanel::closeAndClean() {
}

/*! @brief Checks whether parental control is currently enabled.
 * @return true if parental control is enabled, false otherwise. */
bool MainMenuPanel::isParentalControlEnabled() {
    getAppContext().is_enabled = ipc::isEnabled();
    return getAppContext().is_enabled;
}

/*! @brief Checks whether parental control is installed on the system.
 * @return true if parental control is installed, false otherwise. */
bool MainMenuPanel::isParentalControlInstalled() {
    return getAppContext().is_available;
}

/*! @brief Retrieves the list of user accounts.
 * @return A list of user account names. */
std::list<std::string> MainMenuPanel::getUsersList() {
    std::list<std::string> users;

    return users;
}

/*! @brief Creates and returns the root UI element for the main menu panel.
 * @return A pointer to the root UI element (OverlayFrame). */
tsl::elm::Element* MainMenuPanel::createUI() {
    std::string subTitle = isParentalControlInstalled() ? isParentalControlEnabled() ? "Parental Control is Enabled" : "Parental Control is Disabled" : "Parental Control is not installed";

    rootFrame_ = new tsl::elm::OverlayFrame("Parental Control", subTitle);
    rootList_ = new tsl::elm::List();
    
    rebuildUI();        

    return rootFrame_;
}


/*! @brief Rebuilds the main menu list items and their click handlers. */
void MainMenuPanel::rebuildUI() {
    // Special place for special messages (database tampered / needs upgrade)
    bool specialMessages = false;
    const auto& dbTampered = ipc::isDatabaseTampered();
    const auto& mustUpgrade = ipc::isDatabaseNeedsUpgrade();
    specialMessages = dbTampered || mustUpgrade;

    if(specialMessages) {
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

    auto currentUserUid = ipc::getCurrentUserUid();
    logDebug("userId=%s\n", currentUserUid.c_str());

    if(!currentTitle.empty() && !currentTitle.starts_with("Err#") && !currentUserUid.empty()) {
        
        logDebug("userId=%s\n", currentUserUid.c_str());

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

        // Bedtime
        AccountUid currentAccountUid = helpers::accountUidFromString(currentUserUid);
        UserData currentUserData;
        currentUserData.uid = currentAccountUid;
        currentUserData.nickname = currentUserNickname;
        auto bedtime = ipc::getBedtime(currentUserData);
        if(bedtime > 0) {
            u16 bedtimeHours = bedtime / 60;
            u16 bedtimeMinutes = bedtime % 60;
            char bedtimeStr[6] = {0};
            std::snprintf(bedtimeStr, sizeof(bedtimeStr), "%02d:%02d", bedtimeHours, bedtimeMinutes);
            auto entryBedtime = new tsl::elm::ListItem("Bedtime: " + std::string(bedtimeStr));
            rootList_->addItem(entryBedtime);
        }

        // Time until next deadline
        auto now = std::chrono::system_clock::now();
        auto now_t = std::chrono::system_clock::to_time_t(now);
        struct tm timeInfo;
        localtime_r(&now_t, &timeInfo);
        u16 currentMinutesSinceMidnight = timeInfo.tm_hour * 60 + timeInfo.tm_min;

        u16 minutesUntilDeadline = std::numeric_limits<u16>::max();

        if(remainingTime.count() > 0) {
            minutesUntilDeadline = std::min(minutesUntilDeadline, static_cast<u16>(remainingTime.count()));
        }

        if(bedtime > 0) {
            u16 minutesUntilBedtime = bedtime - currentMinutesSinceMidnight;
            if(minutesUntilBedtime > 0) {
                minutesUntilDeadline = std::min(minutesUntilDeadline, minutesUntilBedtime);
            }
        }

        if(minutesUntilDeadline != std::numeric_limits<u16>::max() && minutesUntilDeadline > 0) {
            auto durationInMinutes = minutes{minutesUntilDeadline};
            auto hoursPart = duration_cast<hours>(durationInMinutes);
            auto minutesPart = duration_cast<minutes>(durationInMinutes - hoursPart);
            auto entryNextDeadline = new tsl::elm::ListItem("Play time ends at " +(hoursPart.count() > 0 ? std::to_string(hoursPart.count()) + "h " : "") +std::to_string(minutesPart.count()) +" mn");
            rootList_->addItem(entryNextDeadline);
        }
    } else {
        rootList_->addItem(new tsl::elm::ListItem("No user / app started"));
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

/*! @brief Updates the panel state each frame. */
void MainMenuPanel::update() {    
}

/*! @brief Handles user input not captured by other UI elements.
 * @param keysDown Keys pressed this frame.
 * @param keysHeld Keys held this frame.
 * @param touchPos Current touch screen position.
 * @param joyStickPosLeft Left analog stick position.
 * @param joyStickPosRight Right analog stick position.
 * @return true if the input was handled, false otherwise. */
bool MainMenuPanel::handleInput(u64 keysDown, u64 keysHeld, const HidTouchState &touchPos, HidAnalogStickState joyStickPosLeft, HidAnalogStickState joyStickPosRight) {
    if (keysDown & HidNpadButton_B) {
        tsl::Overlay::get()->close();
        return true;
    }

    return false;
}