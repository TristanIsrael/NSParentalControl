#include "panel_main_menu.h"
#include "tesla.hpp"
#include "switch.h"
#include "database.h"
#include "console.h"
#include "version.h"
#include "gui_helpers.h"

MainMenu::MainMenu(UserSession& user, GameSession& game, UserSessions& sessions, Settings& settings, bool& active) :
    active_(active), user_(user), game_(game), sessions_(sessions), settings_(settings)
{
}

tsl::elm::Element* MainMenu::createUI() {
    rootFrame_ = new tsl::elm::OverlayFrame("Parental Control", "Setup");

    rebuildUI();

    return rootFrame_;
}

void MainMenu::rebuildUI() {
    if(!active_) return;

    // The content is layed out using a List containing panels than will
    // be visible or not depending on the needs
    auto list = new tsl::elm::List();

    list->addItem(new tsl::elm::ListItem(makeUserProfileLabel(user_)));
    list->addItem(new tsl::elm::ListItem(makeDailyLimitLabel(settings_)));
    list->addItem(new tsl::elm::ListItem(makeRemainingDurationLabel(settings_, sessions_)));

    list->addItem(new tsl::elm::ListItem(app_version().c_str()));
    
    rootFrame_->setContent(list);
}

void MainMenu::update() {
    
}

// Called once every frame to handle inputs not handled by other UI elements
bool MainMenu::handleInput(u64 keysDown, u64 keysHeld, const HidTouchState &touchPos, HidAnalogStickState joyStickPosLeft, HidAnalogStickState joyStickPosRight) {
    if (keysDown & HidNpadButton_B) {
        tsl::goBack();
        return true;
    }

    return false;
}