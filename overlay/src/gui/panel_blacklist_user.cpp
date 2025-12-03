#include "panel_blacklist_user.h"
#include <switch.h>
#include <chrono>
#include <string>
#include <list>
#include <mutex>
#include "version.h"
#include "logger.h"
#include "Command.hpp"
#include "AppContext.h"
#include "helpers/ipc_helpers.h"
#include "helpers/switch_helpers.h"
#include "panel_blacklist_add_title.h"

using namespace alefbet::pctrl;
using namespace alefbet::pctrl::logger;
using namespace std::chrono;

std::mutex mutex_;

SetupBlacklistUserPanel::SetupBlacklistUserPanel(const UserData& user) {    
    user_ = user;    
}

SetupBlacklistUserPanel::~SetupBlacklistUserPanel() {      
}

tsl::elm::Element* SetupBlacklistUserPanel::createUI() {
    logDebug("createUI\n");

    std::string subTitle = std::string("Blacklist for ") +user_.nickname;
    rootFrame_ = new tsl::elm::OverlayFrame("Parental Control", subTitle);
    rootList_ = new tsl::elm::List();
    rootList_->disableCaching();
    //titlesList_ = new tsl::elm::List();
    
    rebuildUI();        

    return rootFrame_;
}

void SetupBlacklistUserPanel::rebuildUI() {
    logDebug("début rebuildUI\n");

    rootList_->addItem(new tsl::elm::CategoryHeader("Actions"));

    auto entryAddTitle = new tsl::elm::ListItem("Add a title");
    rootList_->addItem(entryAddTitle);
    entryAddTitle->setClickListener([this](u64 keys) {
        if(keys & HidNpadButton_A) {
            tsl::changeTo<SetupBlacklistAddTitlePanel>(user_);
            return true;
        }

        return false;
    });

    rootList_->addItem(new tsl::elm::CustomDrawer([](tsl::gfx::Renderer* renderer, s32 x, s32 y, s32 w, s32 h) {
        std::string str = "To remove a title, select it and press \uE0B6";
        const auto& size = renderer->drawString(str, false, x, y+40, 18, renderer->a(tsl::Color(0x0, 0x0, 0x0, 0x0)));
        renderer->drawString(str, false, x + (w-size.first)/2, y+40, 18, renderer->a(tsl::Color(0xf, 0xf, 0xf, 0xf)));
    }), 50);    

    rootList_->addItem(new tsl::elm::CategoryHeader("Titles list"));

    const auto& userId = accountUidToString(user_.uid);
    const auto& blacklist = ipc::getBlacklistedTitles(userId);

    for(size_t i = 0 ; i < blacklist.size() ; i++) {
        const auto& titleId = blacklist[i];
        const auto& titleName = helpers::getTitleName(titleId);
        tsl::elm::ListItem* entryTitle = new tsl::elm::ListItem(titleName);
        rootList_->addItem(entryTitle);
        entryTitle->setClickListener([this, titleId, entryTitle](u64 keys) {
            if(keys & HidNpadButton_Minus) {
                std::scoped_lock lock(mutex_);

                logDebug("Before IPC\n");
                ipc::removeTitleFromBlacklist(user_, titleId);  
                //needsRefresh_ = true;
                logDebug("Add to list\n");
                removedItems_.push_back(entryTitle);
                return true;
            }

            return false;
        });
    }
    
    //titlesList_->setFocusedIndex(0);

    rootFrame_->setContent(rootList_);

    logDebug("fin rebuildUI\n");
    //rootFrame_->setContent(titlesList_);

    //needsRefresh_ = true;
}

void SetupBlacklistUserPanel::update() {
    std::scoped_lock lock(mutex_);

    if(removedItems_.empty()) return;
    
    for(tsl::elm::Element* item: removedItems_) {
        //auto& item = removedItems_[i];

        if(item != nullptr) {
            logDebug("Remove item\n");
            rootList_->removeItem(item);
        }
    }

    logDebug("Clear list\n");
    removedItems_.clear();
    rootList_->setFocusedIndex(1);

    /*mutex_.lock();
    if(isClosing_) return;
    mutex_.unlock();*/

    /*if(needsRefresh_) {                
        titlesList_->clear();

        titlesList_->addItem(new tsl::elm::CategoryHeader("Titles list"));

        const auto& userId = accountUidToString(user_.uid);
        const auto& blacklist = ipc::getBlacklistedTitles(userId);

        for(size_t i = 0 ; i < blacklist.size() ; i++) {
            const auto& titleId = blacklist[i];
            const auto& titleName = helpers::getTitleName(titleId);
            tsl::elm::ListItem* entryTitle = new tsl::elm::ListItem(titleName);
            titlesList_->addItem(entryTitle);
            entryTitle->setClickListener([this, titleId, entryTitle](u64 keys) {
                if(keys & HidNpadButton_Minus) {
                    ipc::removeTitleFromBlacklist(user_, titleId);  
                    needsRefresh_ = true;
                    return true;
                }

                return false;
            });
        }

        auto entryAddTitle = new tsl::elm::ListItem("Add a title");
        titlesList_->addItem(entryAddTitle);
        entryAddTitle->setClickListener([this](u64 keys) {
            if(keys & HidNpadButton_A) {
                tsl::changeTo<SetupBlacklistAddTitlePanel>(user_);
                return true;
            }

            return false;
        });

        titlesList_->setFocusedIndex(0);

        needsRefresh_ = false;
    }*/
}

// Called once every frame to handle inputs not handled by other UI elements
bool SetupBlacklistUserPanel::handleInput(u64 keysDown, u64 keysHeld, const HidTouchState &touchPos, HidAnalogStickState joyStickPosLeft, HidAnalogStickState joyStickPosRight) {
    if (keysDown & HidNpadButton_B) {
        std::scoped_lock lock(mutex_);
        tsl::goBack();

        return true;
    } 

    return false;
}