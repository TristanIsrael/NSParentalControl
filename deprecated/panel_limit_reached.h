#pragma once

#include "tesla.hpp"
#include "structs.h"

class LimitReachedContent : public tsl::Gui {
public:
    LimitReachedContent() {}

    tsl::elm::Element* createUI() override;
    void rebuildUI();
    void update() override;

    // Called once every frame to handle inputs not handled by other UI elements
    virtual bool handleInput(u64 keysDown, u64 keysHeld, const HidTouchState &touchPos, HidAnalogStickState joyStickPosLeft, HidAnalogStickState joyStickPosRight) override;

private:
    tsl::elm::OverlayFrame* rootFrame_ = nullptr;    
};
