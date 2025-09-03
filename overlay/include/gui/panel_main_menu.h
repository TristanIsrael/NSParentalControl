#pragma once

#include "switch.h"
#include "structs.h"
#include "tesla.hpp"

class MainMenu : public tsl::Gui {
public:
    MainMenu(UserSession& user, GameSession& game, UserSessions& sessions, Settings& settings, bool& active);

    tsl::elm::Element* createUI() override;
    void rebuildUI();
    void update() override;

    // Called once every frame to handle inputs not handled by other UI elements
    virtual bool handleInput(u64 keysDown, u64 keysHeld, const HidTouchState &touchPos, HidAnalogStickState joyStickPosLeft, HidAnalogStickState joyStickPosRight) override;

private:    
    tsl::elm::OverlayFrame* rootFrame_ = nullptr;
    bool& active_;
    UserSession& user_;
    GameSession& game_;
    UserSessions& sessions_;
    Settings& settings_; 
    bool is_configuring_ = false;   
};