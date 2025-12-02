#include "panel_blacklist_user.h"
#include <switch.h>
#include <chrono>
#include <string>
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

SetupBlacklistUserPanel::SetupBlacklistUserPanel(const UserData& user) {    
    user_ = user;
}

SetupBlacklistUserPanel::~SetupBlacklistUserPanel() {      
}

tsl::elm::Element* SetupBlacklistUserPanel::createUI() {
    std::string subTitle = std::string("Blacklist for ") +user_.nickname;
    rootFrame_ = new tsl::elm::OverlayFrame("Parental Control", subTitle);
    rootList_ = new tsl::elm::List();
    
    rebuildUI();        

    return rootFrame_;
}

void SetupBlacklistUserPanel::rebuildUI() {
    rootList_->addItem(new tsl::elm::CategoryHeader("Titles list"));    

    const auto& userId = accountUidToString(user_.uid);
    const auto& blacklist = ipc::getBlacklistedTitles(userId);

    tsl::elm::ListItem* entryTitle = nullptr;

    for(size_t i = 0 ; i < blacklist.size() ; i++) {
        const auto& titleId = blacklist[i];
        const auto& titleName = helpers::getTitleName(titleId);
        entryTitle = new tsl::elm::ListItem(titleName);
        rootList_->addItem(entryTitle);
        entryTitle->setClickListener([this, titleId](u64 keys) {
            if(keys & HidNpadButton_Minus) {
                ipc::removeTitleFromBlacklist(user_, titleId);
                return true;
            }

            return false;
        });
    }

    auto entryAddTitle = new tsl::elm::ListItem("Add a title");
    rootList_->addItem(entryAddTitle);
    entryAddTitle->setClickListener([this](u64 keys) {
        if(keys & HidNpadButton_A) {
            tsl::changeTo<SetupBlacklistAddTitlePanel>(user_);
            return true;
        }

        return false;
    });

    rootFrame_->setContent(rootList_);
}

void SetupBlacklistUserPanel::update() {    
}

// Called once every frame to handle inputs not handled by other UI elements
bool SetupBlacklistUserPanel::handleInput(u64 keysDown, u64 keysHeld, const HidTouchState &touchPos, HidAnalogStickState joyStickPosLeft, HidAnalogStickState joyStickPosRight) {
    if (keysDown & HidNpadButton_B) {
        tsl::goBack();
        return true;
    }

    return false;
}