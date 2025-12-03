#include "gui_controller.h"
#include "logger.h"
#include "gui/renderer.hpp"
#include "utils.h"
#include "screen_timeout.h"

using namespace alefbet::pctrl::logger;
using namespace alefbet::pctrl::gfx;
using namespace alefbet::pctrl::gui;

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

constexpr Color backgroundColor =   Color(0x3, 0x3, 0x3, 0xe);    // Deep gray
constexpr Color textColor =         Color(0xf, 0xf, 0xf, 0xf);    // White
constexpr Color titleColor =        Color(0x1, 0xc, 0xe, 0xf);    // Light blue

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

void GuiController::showScreenTitleBlacklisted() {
    logDebug("[Gui] Show title blacklisted screen\n");

    width_ = 1216; //Width must be a multiple of 64
    height_ = 693;

    u16 posX = (ScreenWidth - width_) / 2;
    u16 posY = (ScreenHeight - height_) / 2;

    showOverlay(width_, height_, posX, posY);

    auto& renderer = Renderer::get();
    renderer.startFrame();
        
    // Draw the background
    renderer.drawRect(0, 0, width_, height_, backgroundColor, height_/10);

    /* Draw the logo in the center of the screen */
    /*size_t imgSize = images::LogoWidth * images::LogoHeight;
    u8 logo4444[imgSize*2];    
    rgb565ToRgb4444(images::pctrl_logo, imgSize, logo4444, 0x800);
    renderer.drawBitmap((width_ - images::LogoWidth) / 2, (height_ - images::LogoHeight)/2, images::LogoWidth, images::LogoHeight, logo4444);*/

    // Draw the title
    std::string str = "Unauthorized";
    auto width = calculateTextWidth(str, 62);
    renderer.drawString(str.c_str(), false, (width_ - width)/2, 138, 62, titleColor);    

    str = "Sorry, you are not allowed to";
    width = calculateTextWidth(str, 50);
    renderer.drawString(str.c_str(), false, (width_ - width)/2, 366, 50, textColor);    

    str = "play this game.";
    width = calculateTextWidth(str, 50);
    renderer.drawString(str.c_str(), false, (width_ - width)/2, 420, 50, textColor);    

    // Press any key...
    str = "Press any key...";
    width = calculateTextWidth(str, 32);
    renderer.drawString(str.c_str(), false, (width_ - width)/2, height_ - 48 - 16, 32, textColor);    

    renderer.endFrame();
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

void GuiController::clearScreen() {
    auto& renderer = Renderer::get();
    renderer.startFrame();
    renderer.clearScreen();
    renderer.endFrame();
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