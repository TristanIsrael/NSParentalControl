#pragma once

#include "switch.h"
#include "structs.h"
#include "tesla.hpp"

class MainMenu : public tsl::Gui {
public:
    MainMenu();

    tsl::elm::Element* createUI() override;
    void rebuildUI();
    void update() override;

    // Called once every frame to handle inputs not handled by other UI elements
    virtual bool handleInput(u64 keysDown, u64 keysHeld, const HidTouchState &touchPos, HidAnalogStickState joyStickPosLeft, HidAnalogStickState joyStickPosRight) override;

private:    
    tsl::elm::OverlayFrame* rootFrame_ = nullptr;
    tsl::elm::List* rootList_ = nullptr;
    bool active_ = false;
    UserSession user_;
    GameSession game_;
    UserSessions sessions_;
    Settings settings_; 
    bool is_configuring_ = false;   
};