#include "panel_blacklist_add_title.h"
#include <switch.h>
#include <chrono>
#include <string>
#include "version.h"
#include "logger.h"
#include "Command.hpp"
#include "AppContext.h"
#include "helpers/ipc_helpers.h"

using namespace alefbet::pctrl;
using namespace alefbet::pctrl::logger;
using namespace std::chrono;

SetupBlacklistAddTitlePanel::SetupBlacklistAddTitlePanel(const UserData& user) {    
    user_ = user;
}

SetupBlacklistAddTitlePanel::~SetupBlacklistAddTitlePanel() {      
}

tsl::elm::Element* SetupBlacklistAddTitlePanel::createUI() {
    rootFrame_ = new tsl::elm::OverlayFrame("Parental Control", "Add title to blacklist");
    rootList_ = new tsl::elm::List();
    
    rebuildUI();        

    return rootFrame_;
}

void SetupBlacklistAddTitlePanel::rebuildUI() {
    rootList_->addItem(new tsl::elm::CategoryHeader("Installed titles"));    

    bool ok = true;

    ok = R_SUCCEEDED(nsInitialize());
    if(!ok) {
        logError("[Blacklist] Could not initialize NS service\n");
        rootList_->addItem(new tsl::elm::ListItem("Error #11"));
    }

    s32 count = 0;
    NsApplicationRecord records[100]; 
    if(ok) {               
        ok = R_SUCCEEDED(nsListApplicationRecord(records, sizeof(records), 0, &count));
    } 

    if(!ok) {
        logError("[Blacklist] Could not get application record count\n");
        rootList_->addItem(new tsl::elm::ListItem("Error #12"));
        nsExit();
    }

    NsApplicationControlData nacp;
    NacpLanguageEntry* langEntry = nullptr;
    tsl::elm::ListItem* entryTitle = nullptr;

    if(ok) {
        for (s32 i = 0; i < count; i++) {
            const auto& record = records[i];
            const auto& appId = record.application_id;
            u64 actual_size = 0;
            
            ok = R_SUCCEEDED(nsGetApplicationControlData(NsApplicationControlSource_Storage, appId, &nacp, sizeof(nacp), &actual_size));
            if(!ok) {
                logError("[Blacklist] Could not get application information for %ull\n", appId);
                rootList_->addItem(new tsl::elm::ListItem("Error #13"));                
            } else {                
                if(R_SUCCEEDED(nacpGetLanguageEntry(&nacp.nacp, &langEntry)) && langEntry != nullptr) {
                    entryTitle = new tsl::elm::ListItem(langEntry->name);
                    rootList_->addItem(entryTitle);
                    entryTitle->setClickListener([this, appId](u64 keys) {
                        if(keys & HidNpadButton_A) {
                            // Add title to blacklist
                            ipc::addTitleToBlacklist(user_, appId); 
                            return true;
                        }

                        return false;
                    });
                } else {
                    rootList_->addItem(new tsl::elm::ListItem("Error #14"));
                }                
            }
        }
    }

    rootFrame_->setContent(rootList_);
}

void SetupBlacklistAddTitlePanel::update() {    
}

// Called once every frame to handle inputs not handled by other UI elements
bool SetupBlacklistAddTitlePanel::handleInput(u64 keysDown, u64 keysHeld, const HidTouchState &touchPos, HidAnalogStickState joyStickPosLeft, HidAnalogStickState joyStickPosRight) {
    if (keysDown & HidNpadButton_B) {
        tsl::goBack();
        return true;
    }

    return false;
}