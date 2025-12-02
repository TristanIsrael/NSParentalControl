#pragma once

#include "switch.h"
#include "ipc_helpers.h"
#include <tesla.hpp>

using namespace alefbet::pctrl::helpers;

class SetupBlacklistAddTitlePanel : public tsl::Gui {
public:
    SetupBlacklistAddTitlePanel(const UserData& user);
    ~SetupBlacklistAddTitlePanel();

    tsl::elm::Element* createUI() override;
    void rebuildUI();
    void update() override;    

    // Called once every frame to handle inputs not handled by other UI elements
    virtual bool handleInput(u64 keysDown, u64 keysHeld, const HidTouchState &touchPos, HidAnalogStickState joyStickPosLeft, HidAnalogStickState joyStickPosRight) override;

private:    
    tsl::elm::OverlayFrame* rootFrame_ = nullptr;
    tsl::elm::List* rootList_ = nullptr;
    UserData user_;
};