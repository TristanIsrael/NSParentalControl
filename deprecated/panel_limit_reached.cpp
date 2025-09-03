#include "panel_limit_reached.h"
#include "tesla.hpp"
#include "switch.h"
#include "database.h"
#include "console.h"

tsl::elm::Element* LimitReachedContent::createUI() {
    rootFrame_ = new tsl::elm::OverlayFrame("Parental Control", "");

    rebuildUI();

    return rootFrame_;
}

void LimitReachedContent::rebuildUI() {
    // Texte centré
    auto text = new tsl::elm::CustomDrawer([](tsl::gfx::Renderer *renderer, s32 x, s32 y, s32 w, s32 h) {
        renderer->drawString("⏰ Temps de jeu écoulé ⏰",
            false,
            w / 2,
            h / 2,
            30,
            tsl::style::color::ColorText,
            w);
    });
    
    rootFrame_->setContent(text);
}

void LimitReachedContent::update() {
    
}

// Called once every frame to handle inputs not handled by other UI elements
bool LimitReachedContent::handleInput(u64 keysDown, u64 keysHeld, const HidTouchState &touchPos, HidAnalogStickState joyStickPosLeft, HidAnalogStickState joyStickPosRight) {
    return false;   // Return true here to signal the inputs have been consumed
}
