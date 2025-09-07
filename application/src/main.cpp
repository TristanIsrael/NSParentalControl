#include <cstring>
#include <atomic>
#include <thread>

#include <switch.h>
#define TESLA_INIT_IMPL
#include "tesla.hpp"
#include "gui/ovl_alert.h"

#define __DEBUG_AS_NRO

PadState pad;

void initNRO() {
    //consoleInit(NULL);
    
    // Configure our supported input layout: a single player with standard controller styles
    padConfigureInput(1, HidNpadStyleSet_NpadStandard);

    // Initialize the default gamepad (which reads handheld mode inputs as well as the first connected controller)    
    padInitializeDefault(&pad);
}

bool loopNRO() {
    padUpdate(&pad);

    // padGetButtonsDown returns the set of buttons that have been
    // newly pressed in this frame compared to the previous one
    u64 kDown = padGetButtonsDown(&pad);

    if (kDown & HidNpadButton_Plus)
        return false; // break in order to return to hbmenu

    return true;
}

void deinitNRO() {
    // Deinitialize and clean up resources used by the console (important!)
    //consoleExit(NULL);
}

std::atomic<bool> alert_visible = false;
void showAlert() {        
    if(alert_visible) return;

    /*ParentalControlState state = getParentalControlState();

    if(state.alert_type == ALERT_NO_ALERT) return;
    if(state.active) return;*/

    std::thread overlayThread([](){
        alert_visible = true;
        tsl::loop<OverlayAlert>(0, nullptr);        
    });
    overlayThread.detach();
}

void mainLoop() {
    showAlert();
}

// Main program entrypoint
int main(int argc, char* argv[])
{        
#ifdef __DEBUG_AS_NRO
    initNRO();
#endif

    // Other initialization goes here. As a demonstration, we print hello world.
    printf("Hello World!\n");

    // Main loop
    while (appletMainLoop())
    {
#ifdef __DEBUG_AS_NRO      
        if(!loopNRO()) {
            break;
        }
#endif

        mainLoop();

#ifdef __DEBUG_AS_NRO      
        // Update the console, sending a new frame to the display
        consoleUpdate(NULL);
#endif
    }

#ifdef __DEBUG_AS_NRO    
    deinitNRO();
#endif

    return 0;
}
