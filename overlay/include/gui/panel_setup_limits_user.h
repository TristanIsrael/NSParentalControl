#pragma once

#include "switch_helpers.h"
#include <tesla.hpp>
#include <switch.h>

typedef enum {
    NoItem = 0,
    DailyLimitHours = 1,
    DailyLimitMinutes = 2
} SelectedItem;

using namespace alefbet::pctrl::helpers;

class SetupLimitsUserPanel : public tsl::Gui {
public:
    SetupLimitsUserPanel(const UserData& user);
    ~SetupLimitsUserPanel();

    tsl::elm::Element* createUI() override;
    void rebuildUI();
    void update() override;
    void closeAndClean();

    // Called once every frame to handle inputs not handled by other UI elements
    virtual bool handleInput(u64 keysDown, u64 keysHeld, const HidTouchState &touchPos, HidAnalogStickState joyStickPosLeft, HidAnalogStickState joyStickPosRight) override;

private:
    void selectNextItem();
    void selectBeforeItem();
    void decreaseValue();
    void increaseValue();
    u8 valueRanged(u8 value, s8 diff, u8 min, u8 max, bool cycle = true);

private:    
    UserData user_;
    tsl::elm::OverlayFrame* rootFrame_ = nullptr;
    tsl::elm::List* rootList_ = nullptr;
    bool error_ = false;
    SelectedItem selectedItem_ = DailyLimitHours;
    u8 dailyLimitHours_ = 0;
    u8 dailyLimitMinutes_ = 0;
};