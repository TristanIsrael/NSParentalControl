#pragma once
#include <switch.h>
#include <string>
#include <vector>
#include "helpers.h"

using namespace alefbet::pctrl::structs;

class GuiController {
    public:
        void showScreenTimeout();
        void hideScreenTimeout();
        void showPanelAuthentication();
        void showPanelTitleBlacklisted();
        void hideAll();        
        bool isVisible() const;

        


        // Disabled because of the conflict with Tesla... to be solved
        /*void showRemainingTimePanel();
        void updateRemainingTimePanel(const u16& remaining_time_in_minutes, const u16& limit_in_minutes);
        void hideRemainingTimePanel();
        
        inline bool isRemainingTimePanelVisible() {
            return remaining_time_visible_;
        }*/

    private:
        void showOverlay(u16 width, u16 height, u16 posX, u16 posY);
        int calculateTextWidth(const std::string& text, int fontSize, bool monospace = false);
        void rgb565ToRgb4444(const u16* source, size_t size, u8* dest, const u16 alpha = 0xffff);
        void clearScreen(bool ownFrame = true);

        // Authentication
        void refreshAuthenticationPanel();
        void initUserInput();
        void verifyUserInput();
        void handlePinInput();        

        typedef enum {
            PinSetup,
            PinSetupVerification,
            PinVerification,
            PinsDontMatch,
            PinOk,
            PinError
        } PinStage;
        
    private:
        u16 width_ = 0;
        u16 height_ = 0;
        bool remaining_time_visible_ = false;
        bool visible_ = false;
        bool needsRefresh_ = false;
        PadState pad_p1_;
        PadState pad_handheld_;
        std::vector<u64> keysDown_;
        PinStage pinStage_ = PinSetup;
        std::string enteredPin_;
        UserData user_;
};
