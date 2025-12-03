#pragma once

#include "switch.h"
#include "ipc_helpers.h"
#include <tesla.hpp>
#include <list>

using namespace alefbet::pctrl::helpers;

class SetupBlacklistUserPanel : public tsl::Gui {
public:
    SetupBlacklistUserPanel(const UserData& user);
    ~SetupBlacklistUserPanel();

    tsl::elm::Element* createUI() override;
    void rebuildUI();
    void update() override;    

    // Called once every frame to handle inputs not handled by other UI elements
    virtual bool handleInput(u64 keysDown, u64 keysHeld, const HidTouchState &touchPos, HidAnalogStickState joyStickPosLeft, HidAnalogStickState joyStickPosRight) override;

private:    
    tsl::elm::OverlayFrame* rootFrame_ = nullptr;
    tsl::elm::List* rootList_ = nullptr;
    //tsl::elm::List* titlesList_ = nullptr;
    UserData user_;
    std::vector<tsl::elm::ListItem*> removedItems_;
    //bool needsRefresh_ = false;  
    //bool isClosing_ = false;
};