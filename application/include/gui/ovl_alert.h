#include <string>
#include "tesla.hpp"

class PanelLimitReached : public tsl::Gui {
public:
    PanelLimitReached() {}

    tsl::elm::Element* createUI() override;
    void rebuildUI();
    void update() override;

    // Called once every frame to handle inputs not handled by other UI elements
    virtual bool handleInput(u64 keysDown, u64 keysHeld, const HidTouchState &touchPos, HidAnalogStickState joyStickPosLeft, HidAnalogStickState joyStickPosRight) override;

private:
    tsl::elm::OverlayFrame* rootFrame_ = nullptr;    
};


class OverlayAlert : public tsl::Overlay {
public:    
    OverlayAlert() {}
    virtual ~OverlayAlert();

    void initServices() override;
    void exitServices() override;

    void onShow() override {}
    void onHide() override {}

    std::unique_ptr<tsl::Gui> loadInitialGui() override;

    void show();

private:
    bool active_ = false;
};