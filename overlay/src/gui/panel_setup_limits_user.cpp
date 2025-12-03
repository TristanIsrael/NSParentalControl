#include "panel_setup_limits_user.h"
#include <switch.h>
#include <mutex>
#include <chrono>
#include "logger.h"
#include "Command.hpp"
#include "AppContext.h"
#include "helpers/ipc_helpers.h"
#include "panel_admin_menu.h"

using namespace alefbet::pctrl;
using namespace std::chrono;

constexpr tsl::Color ColorSelected = tsl::style::color::ColorHighlight;
constexpr tsl::Color ColorWhite = tsl::Color(0xffff);

SetupLimitsUserPanel::SetupLimitsUserPanel(const UserData& user) {
    user_ = user;
}

SetupLimitsUserPanel::~SetupLimitsUserPanel() {  
    closeAndClean();
}

void SetupLimitsUserPanel::closeAndClean() {
}

tsl::elm::Element* SetupLimitsUserPanel::createUI() {
    rootFrame_ = new tsl::elm::OverlayFrame("Parental Control", "Set limits for " +user_.nickname);
    rootList_ = new tsl::elm::List();
    
    // Load settings
    const auto& dailyLimitInMinutes = ipc::getDailyLimit(user_);
    const auto& duration = minutes{dailyLimitInMinutes};
    auto hoursPart = duration_cast<hours>(duration);
    auto minutesPart = duration_cast<minutes>(duration - hoursPart);
    dailyLimitHours_ = hoursPart.count();
    dailyLimitMinutes_ = minutesPart.count();

    rebuildUI();

    return rootFrame_;
}

void SetupLimitsUserPanel::rebuildUI() {    
    rootList_->addItem(new tsl::elm::CustomDrawer([this](tsl::gfx::Renderer* renderer, s32 x, s32 y, s32 w, s32 h) {
        renderer->drawString("Please use \u2190 and \u2192 to move.", false, x + 50, y + 30, 16, renderer->a(ColorWhite));
        renderer->drawString("Modify the limits with \u2191 and \u2193.", false, x + 45, y + 55, 16, renderer->a(ColorWhite));
    }), 60);

    rootList_->addItem(new tsl::elm::CategoryHeader("Play limits"));
    
    // Daily Limit
    rootList_->addItem(new tsl::elm::CustomDrawer([this](tsl::gfx::Renderer* renderer, s32 x, s32 y, s32 w, s32 h) {
        s32 yPos = y;

        yPos += 35;
        renderer->drawString("Daily", false, x, yPos, 20, renderer->a(ColorWhite));
        
        // Minutes
        renderer->drawString("mn", false, w-40, yPos, 20, renderer->a(selectedItem_ == DailyLimitMinutes ? ColorSelected : ColorWhite));
        std::string str = std::to_string(dailyLimitMinutes_);
        renderer->drawString(str.c_str(), false, w-60 -(dailyLimitMinutes_ > 9 ? 10 : 0), yPos, 20, renderer->a(selectedItem_ == DailyLimitMinutes ? ColorSelected : ColorWhite));
                
        // Hours
        renderer->drawString("h", false, w-90, yPos, 20, renderer->a(selectedItem_ == DailyLimitHours ? ColorSelected : ColorWhite));
        str = std::to_string(dailyLimitHours_);
        renderer->drawString(str.c_str(), false, w-110 -(dailyLimitHours_ > 9 ? 10 : 0), yPos, 20, renderer->a(selectedItem_ == DailyLimitHours ? ColorSelected : ColorWhite));        
        
    }), 60);

    
    rootFrame_->setContent(rootList_);
}

void SetupLimitsUserPanel::update() {    
    
}

// Called once every frame to handle inputs not handled by other UI elements
bool SetupLimitsUserPanel::handleInput(u64 keysDown, u64 keysHeld, const HidTouchState &touchPos, HidAnalogStickState joyStickPosLeft, HidAnalogStickState joyStickPosRight) {
    if (keysDown == 0) return false;    

    // B to cancel
    if(keysDown & HidNpadButton_B) {
        tsl::goBack();

        // Update the setting only when exitting
        u16 limitInMinutes = dailyLimitHours_*60 + dailyLimitMinutes_;
        ipc::setDailyLimit(user_, limitInMinutes);

        return true;
    } else if(keysDown & HidNpadButton_AnyDown) {
        decreaseValue();
    } else if(keysDown & HidNpadButton_AnyUp) {
        increaseValue();
    } else if(keysDown & HidNpadButton_AnyLeft) {
        selectBeforeItem();
    } else if(keysDown & HidNpadButton_AnyRight) {
        selectNextItem();
    }
    
    return false; 
}

void SetupLimitsUserPanel::selectNextItem() {
    switch(selectedItem_) {
        case DailyLimitHours: selectedItem_ = DailyLimitMinutes; break;
        case DailyLimitMinutes: selectedItem_ = DailyLimitHours; break;
        default: break;
    }
}

void SetupLimitsUserPanel::selectBeforeItem() {
    switch(selectedItem_) {
        case DailyLimitHours: selectedItem_ = DailyLimitMinutes; break;
        case DailyLimitMinutes: selectedItem_ = DailyLimitHours; break;
        default: break;
    }
}

void SetupLimitsUserPanel::decreaseValue() {
    switch(selectedItem_) {
        case DailyLimitHours: dailyLimitHours_ = valueRanged(dailyLimitHours_, -1, 0, 12); break;
        case DailyLimitMinutes: dailyLimitMinutes_ = valueRanged(dailyLimitMinutes_, -1, 0, 59); break;
        default: break;
    }
}

void SetupLimitsUserPanel::increaseValue() {
    switch(selectedItem_) {
        case DailyLimitHours: dailyLimitHours_ = valueRanged(dailyLimitHours_, +1, 0, 12); break;
        case DailyLimitMinutes: dailyLimitMinutes_ = valueRanged(dailyLimitMinutes_, +1, 0, 59); break;
        default: break;
    }
}

u8 SetupLimitsUserPanel::valueRanged(u8 value, s8 diff, u8 min, u8 max, bool cycle) {
    if(value + diff < min) {
        return cycle ? max : min;
    } else if(value + diff > max) {
        return cycle ? min : max;
    } else {
        return value + diff;
    }
}