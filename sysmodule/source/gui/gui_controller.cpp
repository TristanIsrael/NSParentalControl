#include "gui_controller.h"
#include "logger.h"
#include "gui/renderer.hpp"
#include "utils.h"
#include "screen_timeout.h"
#include "database.h"
#include <mutex>

using namespace alefbet::pctrl::logger;
using namespace alefbet::pctrl::gfx;
using namespace alefbet::pctrl::gui;
using namespace alefbet::pctrl::helpers;
using namespace alefbet::pctrl::database;

namespace images {
    /*#include "inc/chrono_0_pc.inc"    
    #include "inc/chrono_9_pc.inc"
    #include "inc/chrono_17_pc.inc"
    #include "inc/chrono_25_pc.inc"
    #include "inc/chrono_34_pc.inc"
    #include "inc/chrono_42_pc.inc"
    #include "inc/chrono_50_pc.inc"
    #include "inc/chrono_59_pc.inc"
    #include "inc/chrono_67_pc.inc"
    #include "inc/chrono_75_pc.inc"
    #include "inc/chrono_84_pc.inc"
    #include "inc/chrono_92_pc.inc"
    #include "inc/chrono_100_pc.inc"
    #include "inc/chrono_1_mn.inc"
    #include "inc/chrono_2_mn.inc"
    #include "inc/chrono_3_mn.inc"
    #include "inc/chrono_4_mn.inc"
    #include "inc/chrono_5_mn.inc"*/
    #include "inc/logo.inc"
}

static std::mutex s_mutexVisible;

namespace colors {
    namespace authenticator {
        constexpr Color textColor         = Color(0xf, 0xf, 0xf, 0xf);    // White
        constexpr Color circleColor       = Color(0xf, 0xf, 0xf, 0xf);    // White
        constexpr Color backgroundColor   = Color(0x2, 0x4, 0x6, 0xe);    // Deep blue
        constexpr Color titleColor        = Color(0x1, 0xc, 0xe, 0xf);    // Light blue
        constexpr Color errorColor        = Color(0xf, 0x0, 0x0, 0xf);    // Plain red
        constexpr Color successColor      = Color(0x0, 0xf, 0xd, 0xf);    // Green
    }

    namespace pctrl {
        constexpr Color backgroundColor   = Color(0x3, 0x3, 0x3, 0xe);    // Deep gray
        constexpr Color textColor         = Color(0xf, 0xf, 0xf, 0xf);    // White
        constexpr Color titleColor        = Color(0x1, 0xc, 0xe, 0xf);    // Light blue
    }
}

/* There should only be a single transfer memory (for nv). */
alignas(ams::os::MemoryPageSize) constinit u8 g_nv_transfer_memory[0x40000];
extern "C" ::Result __nx_nv_create_tmem(TransferMemory *t, u32 *out_size, Permission perm) {
    *out_size = sizeof(g_nv_transfer_memory);
    return tmemCreateFromMemory(t, g_nv_transfer_memory, sizeof(g_nv_transfer_memory), perm);
}

void GuiController::showScreenTimeout() {
    logDebug("[Gui] ShowScreenTimeout\n");

    ScreenTimeout *screen = new ScreenTimeout();
    screen->setTransferMemory(g_nv_transfer_memory, sizeof(g_nv_transfer_memory));
    screen->ShowScreenTimeout();

    // Instance is not freed because the console will be rebooted

    /*width_ = ScreenWidth;
    height_ = ScreenHeight;

    showOverlay(width_, height_, 0, 0);

    auto& renderer = Renderer::get();
    renderer.startFrame();

    renderer.fillScreen(Color(0x44, 0x44, 0xff, 0x77));
    renderer.drawString("Timeout!", false, 100, 100, 40, renderer.a(Color(0xff, 0xff, 0xff, 0xff)));
    renderer.drawString("Press Vol+ to reboot", false, 100, 200, 30, renderer.a(Color(0xff, 0xff, 0xff, 0xff)));

    renderer.endFrame();*/
}

void GuiController::showPanelTitleBlacklisted() {
    logDebug("[Gui] Show title blacklisted screen\n");

    width_ = 1216; //Width must be a multiple of 64
    height_ = 693;

    u16 posX = (ScreenWidth - width_) / 2;
    u16 posY = (ScreenHeight - height_) / 2;

    showOverlay(width_, height_, posX, posY);

    auto& renderer = Renderer::get();
    renderer.startFrame();
        
    // Draw the background
    renderer.drawRect(0, 0, width_, height_, colors::pctrl::backgroundColor, height_/10);

    /* Draw the logo in the center of the screen */
    /*size_t imgSize = images::LogoWidth * images::LogoHeight;
    u8 logo4444[imgSize*2];    
    rgb565ToRgb4444(images::pctrl_logo, imgSize, logo4444, 0x800);
    renderer.drawBitmap((width_ - images::LogoWidth) / 2, (height_ - images::LogoHeight)/2, images::LogoWidth, images::LogoHeight, logo4444);*/

    // Draw the title
    std::string str = "Unauthorized";
    auto width = calculateTextWidth(str, 62);
    renderer.drawString(str.c_str(), false, (width_ - width)/2, 138, 62, colors::pctrl::titleColor);    

    str = "Sorry, you are not allowed to";
    width = calculateTextWidth(str, 50);
    renderer.drawString(str.c_str(), false, (width_ - width)/2, 366, 50, colors::pctrl::textColor);    

    str = "play this game.";
    width = calculateTextWidth(str, 50);
    renderer.drawString(str.c_str(), false, (width_ - width)/2, 420, 50, colors::pctrl::textColor);    

    // Press any key...
    str = "Press any key...";
    width = calculateTextWidth(str, 32);
    renderer.drawString(str.c_str(), false, (width_ - width)/2, height_ - 48 - 16, 32, colors::pctrl::textColor);    

    renderer.endFrame();
}

void GuiController::showPanelAuthentication() {
    logDebug("[Gui] Show the authentication panel\n");

    keysDown_.clear();
    enteredPin_.clear();
    user_.clear();

    // Verify whether a PIN has been set for the user
    user_ = getCurrentUser();
    auto passwords = loadPasswords();
    const auto& uid = accountUidToString(user_.uid);
    const auto& password = passwords[uid];
    logDebug("[Gui] Password is %s\n", password.c_str());
    
    // If there is no password for the user we need a setup
    pinStage_ = password.empty() ? PinSetup : PinVerification;
    
    width_ = 1216; // Width must be a multiple of 64
    height_ = 768;

    u16 posX = (ScreenWidth - width_) / 2;   // Centered
    u16 posY = (ScreenHeight - height_) / 2; // Centered

    showOverlay(width_, height_, posX, posY);    

    needsRefresh_ = true;
    
    initUserInput();
    requestForeground(true);
    
    authenticationRunning_ = true;

    while(authenticationRunning_) {
        if(needsRefresh_) {                    
            refreshAuthenticationPanel();
            needsRefresh_ = false;
        }
        
        verifyUserInput();
                
        // Verify whether an application is running
        if(getRunningApplicationPid() == 0) {
            break;
        }

        svcSleepThread(100'000'000); // Wait 100 ms
    }

    hideAll();

    logDebug("[Gui] Ended authentication\n");
}

void GuiController::refreshAuthenticationPanel() {
    //if(!isVisible()) return;

    logDebug("[Gui] refreshing panel\n");

    auto& renderer = Renderer::get();
    renderer.startFrame();
    clearScreen(false);
        
    // Draw the background
    renderer.drawRect(0, 0, width_, height_, colors::authenticator::backgroundColor, height_/10);

    // Draw the title
    std::string str = "Authentication";
    const auto& width = calculateTextWidth(str, 62);
    renderer.drawString(str.c_str(), false, (width_ - width)/2, 116, 62, colors::authenticator::titleColor);
    
    switch(pinStage_) {
        case PinSetup: {
            // If the user does not already have a code we ask him to create one
            str = user_.nickname + ", please enter a new PIN.";
            const auto& width = calculateTextWidth(str, 62);
            renderer.drawString(str.c_str(), false, (width_ - width)/2, 348, 62, colors::authenticator::textColor);            
            break;
        }
        case PinSetupVerification: {
            str = "Please re-enter your PIN.";
            const auto& width = calculateTextWidth(str, 62);
            renderer.drawString(str.c_str(), false, (width_ - width)/2, 348, 62, colors::authenticator::textColor);
            break;
        }
        case PinsDontMatch: {
            str = "The PINs don't match. Try again.";
            const auto& width = calculateTextWidth(str, 62);
            renderer.drawString(str.c_str(), false, (width_ - width)/2, 348, 62, colors::authenticator::errorColor);
            break;
        }
        case PinError: {
            str = "Wrong PIN.";
            const auto& width = calculateTextWidth(str, 62);
            renderer.drawString(str.c_str(), false, (width_ - width)/2, 348, 62, colors::authenticator::errorColor);
            break;
        }
        case PinOk: {
            str = "Correct PIN.";
            const auto& width = calculateTextWidth(str, 62);
            renderer.drawString(str.c_str(), false, (width_ - width)/2, 348, 62, colors::authenticator::successColor);
            break;
        }
        case PinVerification: {
            // Otherwise we ask the user password
            str = user_.nickname + ", please enter your PIN.";
            const auto& width = calculateTextWidth(str, 62);
            renderer.drawString(str.c_str(), false, (width_ - width)/2, 348, 62, colors::authenticator::textColor);  
            break;  
        }
    }
    
    // Draw the circles
    renderer.drawCircle(416, 496, 24, keysDown_.size() >= 1, colors::authenticator::circleColor);
    renderer.drawCircle(550, 496, 24, keysDown_.size() >= 2, colors::authenticator::circleColor);
    renderer.drawCircle(680, 496, 24, keysDown_.size() >= 3, colors::authenticator::circleColor);
    renderer.drawCircle(818, 496, 24, keysDown_.size() > 3, colors::authenticator::circleColor);

    renderer.endFrame();    
}

void GuiController::clearScreen(bool ownFrame) {
    auto& renderer = Renderer::get();

    if(ownFrame) {
        renderer.startFrame();
    }

    renderer.clearScreen();
    
    if(ownFrame) {
        renderer.endFrame();
    }
}

int GuiController::calculateTextWidth(const std::string& text, int fontSize, bool monospace)
{
    auto& renderer = Renderer::get();
    const auto& dimension = renderer.drawString(text.c_str(), monospace, 0, 100, fontSize, Color(0, 0, 0, 0));
    return dimension.first;
}

void GuiController::hideScreenTimeout() {
    logDebug("[Gui] Hide timeout screen\n");
    clearScreen();
}

void GuiController::hideAll() {
    logDebug("[Gui] Hide all screens\n");

    auto& renderer = Renderer::get();
    renderer.startFrame();
    renderer.clearScreen();
    renderer.endFrame();
    renderer.exit();

    std::lock_guard<std::mutex> mutex(s_mutexVisible);
    //visible_ = false;
}

/*void GuiController::showRemainingTimePanel() {    
    if(remaining_time_visible_) return;
    remaining_time_visible_ = true;

    logDebug("[Gui] Show remaining time panel\n");
    width_ = 128; //Width must be a multiple of 64
    height_ = 144;

    u16 posX = ScreenWidth - width_ - 16;
    u16 posY = 16;

    showOverlay(width_, height_, posX, posY);

    auto& renderer = Renderer::get();
    renderer.startFrame();
        
    // Draw the timer icon
    renderer.drawBitmap(0, 0, width_, height_, images::chrono_100_pc);
    //renderer.fillScreen(Color(0xf, 0x0, 0x0, 0xc));
    
    renderer.endFrame();
}   

void GuiController::updateRemainingTimePanel(const u16& remaining_time_in_minutes, const u16& limit_in_minutes) {
    if(!remaining_time_visible_) return;    

    logDebug("[Gui] Update remaining time panel\n");
    auto& renderer = Renderer::get();
    if(!renderer.isInitialized()) {
        logError("[Gui] The renderer is not initialized\n");
        return;
    }

    renderer.startFrame();        
    renderer.clearScreen();

    // Draw the timer icon
    const u8* image_data = nullptr;
    
    float remaining_time_in_percent = (float)remaining_time_in_minutes * 100.0f / (float)limit_in_minutes;
    logDebug("[Gui] remaining_time_in_percent=%f, remaining_time_in_minutes=%i, limit_in_minutes=%i\n", remaining_time_in_percent, remaining_time_in_minutes, limit_in_minutes);

    if(remaining_time_in_minutes >= 0 && remaining_time_in_minutes <= 5) {
        // Manage the 5 last minutes
        switch(remaining_time_in_minutes) {
            case 5: image_data = images::chrono_5_mn; break;
            case 4: image_data = images::chrono_4_mn; break;
            case 3: image_data = images::chrono_3_mn; break;
            case 2: image_data = images::chrono_2_mn; break;
            case 1: image_data = images::chrono_1_mn; break;
            case 0: image_data = images::chrono_0_pc; break;
        }        
    } else if(remaining_time_in_minutes > 5 && remaining_time_in_percent <= 9) {
        image_data = images::chrono_9_pc;
    } else if(remaining_time_in_percent > 9 && remaining_time_in_percent <= 17) {
        image_data = images::chrono_17_pc;
    } else if(remaining_time_in_percent > 17 && remaining_time_in_percent <= 25) {
        image_data = images::chrono_25_pc;
    } else if(remaining_time_in_percent > 25 && remaining_time_in_percent <= 34) {
        image_data = images::chrono_34_pc;
    } else if(remaining_time_in_percent > 34 && remaining_time_in_percent <= 42) {
        image_data = images::chrono_42_pc;
    } else if(remaining_time_in_percent > 42 && remaining_time_in_percent <= 50) {
        image_data = images::chrono_50_pc;
    } else if(remaining_time_in_percent > 50 && remaining_time_in_percent <= 59) {
        image_data = images::chrono_59_pc;
    } else if(remaining_time_in_percent > 59 && remaining_time_in_percent <= 67) {
        image_data = images::chrono_67_pc;
    } else if(remaining_time_in_percent > 67 && remaining_time_in_percent <= 75) {
        image_data = images::chrono_75_pc;
    } else if(remaining_time_in_percent > 75 && remaining_time_in_percent <= 84) {
        image_data = images::chrono_84_pc;
    } else if(remaining_time_in_percent > 84 && remaining_time_in_percent <= 92) {
        image_data = images::chrono_92_pc;
    } else if(remaining_time_in_percent > 92) {
        image_data = images::chrono_100_pc;
    }

    if(image_data != nullptr) {
        renderer.drawBitmap(0, 0, width_, height_, image_data);
    } else {
        logError("[Gui] No rule found to show an image.\n");
    }
    
    renderer.endFrame();
}

void GuiController::hideRemainingTimePanel() {
    if(!remaining_time_visible_) return;

    auto& renderer = Renderer::get();    
    if(!renderer.isInitialized()) return;

    logDebug("[Gui] Hide remaining time panel\n");
    clearScreen();
    remaining_time_visible_ = false;
}*/

void GuiController::showOverlay(u16 width, u16 height, u16 posX, u16 posY) {
    logDebug("[Gui] show Overlay of size %ix%i\n", width, height);

    auto& renderer = Renderer::get();    
    renderer.init(width, height, posX, posY);
    clearScreen();

    //requestForeground(true);
}

/**
 * @brief libnx hid:sys shim that gives or takes away frocus to or from the process with the given aruid
 *
 * @param enable Give focus or take focus
 * @param aruid Aruid of the process to focus/unfocus
 * @return Result Result
 */
/*Result GuiController::hidsysEnableAppletToGetInput(bool enable, u64 aruid) {
    const struct {
        u8 permitInput;
        u64 appletResourceUserId;
    } in = { enable != 0, aruid };

    return serviceDispatchIn(hidsysGetServiceSession(), 503, in);
}*/

/*void GuiController::requestForeground(bool enabled) {
    u64 applicationAruid = 0, appletAruid = 0;

    for (u64 programId = 0x0100000000001000UL; programId < 0x0100000000001020UL; programId++) {
        pmdmntGetProcessId(&appletAruid, programId);

        if (appletAruid != 0)
            hidsysEnableAppletToGetInput(!enabled, appletAruid);
    }

    pmdmntGetApplicationProcessId(&applicationAruid);
    hidsysEnableAppletToGetInput(!enabled, applicationAruid);

    hidsysEnableAppletToGetInput(true, 0);
}*/

void GuiController::handlePinInput() {
    // When this function is called it means that a PIN has been entered
    // There are 2 possibilities:
    // - This is the new PIN and we have to ask the user to re-enter for verification
    // - This is the control PIN and we have to verify it
    if(pinStage_ == PinSetup || pinStage_ == PinsDontMatch) {
        enteredPin_ = encodePassword(keysDown_);
        keysDown_.clear();
        pinStage_ = PinSetupVerification;
        needsRefresh_ = true;
    } else if(pinStage_ == PinSetupVerification) {
        const auto& verifPin = encodePassword(keysDown_);
        keysDown_.clear();
        
        pinStage_ = verifPin == enteredPin_ ? PinOk : PinsDontMatch;

        if(pinStage_ == PinOk) {
            const auto& uid = accountUidToString(user_.uid);
            savePassword(uid, verifPin);
            authenticationRunning_ = false;
        }
        
        needsRefresh_ = true;
    }
 }

 void GuiController::initUserInput() {
    logDebug("[Gui] Initialize user input\n");

    /*hidInitialize();
    hidsysInitialize();*/

    // Allow only Player 1 and handheld mode
    HidNpadIdType id_list[2] = { HidNpadIdType_No1, HidNpadIdType_Handheld };
    
    // Configure HID system to only listen to these IDs
    hidSetSupportedNpadIdType(id_list, 2);
    
    // Configure input for up to 2 supported controllers (P1 + Handheld)
    padConfigureInput(2, HidNpadStyleSet_NpadStandard | HidNpadStyleTag_NpadSystemExt);
    
    // Initialize separate pad states for both controllers    
    padInitialize(&pad_p1_, HidNpadIdType_No1);
    padInitialize(&pad_handheld_, HidNpadIdType_Handheld);
    
    // Touch screen init
    //hidInitializeTouchScreen();

    // Clear any stale input from both controllers
    padUpdate(&pad_p1_);
    padUpdate(&pad_handheld_);
}

void GuiController::verifyUserInput() {
    //if(!visible_) return;  
    
    padUpdate(&pad_p1_);
    padUpdate(&pad_handheld_);

    const u64 kDown_p1 = padGetButtonsDown(&pad_p1_);
    const u64 kDown_handheld = padGetButtonsDown(&pad_handheld_);

    u64 keysDown = kDown_p1 | kDown_handheld;

    if(keysDown != 0) {
        logDebug("[Gui] keysDown=%i\n", keysDown);
        keysDown_.push_back(keysDown); 

        if(keysDown_.size() == 4) {
            handlePinInput();
        }

        needsRefresh_ = true;
    }
}

void GuiController::rgb565ToRgb4444(const u16* source, size_t size, u8* dest, const u16 alpha) {
    const u16 alphaval = alpha << 12;

    for(size_t i = 0 ; i < size ; i++) {        
        u16 pix = source[i];

        u8 r5 = (pix >> 11) & 0x1f;
        u8 g6 = (pix >> 5)  & 0x3f;
        u8 b5 = pix         & 0x1f;

        u8 r4 = r5 >> 1;
        u8 g4 = g6 >> 2;
        u8 b4 = b5 >> 1;

        u16 pixout = alphaval | (r4 << 8) | (g4 << 4) | b4;

        dest[i*2] = (u8)(pixout & 0xff);        // LSB
        dest[i*2+1] = (u8)((pixout >> 8) & 0xff); // MSB
    }
}

/*bool GuiController::isVisible() const {
    std::lock_guard<std::mutex> mutex(s_mutexVisible);
    return visible_;
}*/

/**
 * @brief libnx hid:sys shim that gives or takes away frocus to or from the process with the given aruid
 *
 * @param enable Give focus or take focus
 * @param aruid Aruid of the process to focus/unfocus
 * @return Result Result
 */
Result GuiController::hidsysEnableAppletToGetInput(bool enable, u64 aruid) {
    const struct {
        u8 permitInput;
        u64 appletResourceUserId;
    } in = { enable != 0, aruid };

    return serviceDispatchIn(hidsysGetServiceSession(), 503, in);
}

void GuiController::requestForeground(bool enabled) {
    u64 applicationAruid = 0, appletAruid = 0;

    //logToFile("[Gui] Request foreground\n");
    Result rc = 0;
    for (u64 programId = 0x0100000000001000UL; programId < 0x0100000000001020UL; programId++) {
        rc = pmdmntGetProcessId(&appletAruid, programId);
        //logToFile("[Gui] programId=%i, appletAruid=%i, result=%i:%i\n", programId, appletAruid, R_MODULE(rc), R_DESCRIPTION(rc));

        if (appletAruid != 0) {
            rc = hidsysEnableAppletToGetInput(!enabled, appletAruid);
            //logToFile("[Gui] hidsysEnableAppletToGetInput -> false, result=%i:%i\n", R_MODULE(rc), R_DESCRIPTION(rc));
        }
    }

    rc = pmdmntGetApplicationProcessId(&applicationAruid);
    //logToFile("[Gui] pmdmntGetApplicationProcessId, applicationAruid=%i, result=%i:%i\n", applicationAruid, R_MODULE(rc), R_DESCRIPTION(rc));
    rc = hidsysEnableAppletToGetInput(!enabled, applicationAruid);
    //logToFile("[Gui] hidsysEnableAppletToGetInput -> false, applicationAruid=%i, result=%i:%i\n", applicationAruid, R_MODULE(rc), R_DESCRIPTION(rc));

    rc = hidsysEnableAppletToGetInput(true, 0);
    //logToFile("[Gui] hidsysEnableAppletToGetInput -> true (0), result=%i:%i\n", R_MODULE(rc), R_DESCRIPTION(rc));
}