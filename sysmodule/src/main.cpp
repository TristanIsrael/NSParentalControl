#include <cstring>
#include <iostream>
#include <thread>
#define TESLA_INIT_IMPL
#include "tesla.hpp"
#include <switch.h>
#include "structs.h"
#include "algo.h"
#include "database.h"
#include "console.h"
#include "functions.h"
#include "logger.h"
#include "gui/ovl_alert.h"

// Service initialization
/*void __appInit(void)
{
    Result rc;

    // Open a service manager session.
    rc = smInitialize();
    if (R_FAILED(rc))
        diagAbortWithResult(MAKERESULT(Module_Libnx, LibnxError_InitFail_SM));

    // Retrieve the current version of Horizon OS.
    rc = setsysInitialize();
    if (R_SUCCEEDED(rc)) {
        SetSysFirmwareVersion fw;
        rc = setsysGetFirmwareVersion(&fw);
        if (R_SUCCEEDED(rc))
            hosversionSet(MAKEHOSVERSION(fw.major, fw.minor, fw.micro));
        setsysExit();
    }

    // Disable this if you don't want to use the filesystem.
    rc = fsInitialize();
    if (R_FAILED(rc))
        diagAbortWithResult(MAKERESULT(Module_Libnx, LibnxError_InitFail_FS));

    // Disable this if you don't want to use the SD card filesystem.
    fsdevMountSdmc();

    // Close the service manager session.
    smExit();
}

// Service deinitialization.
void __appExit(void)
{
    fsdevUnmountAll(); // Disable this if you don't want to use the SD card filesystem.
    fsExit(); // Disable this if you don't want to use the filesystem.
}*/

std::atomic<bool> alert_visible = false;

void showAlert() {        
    if(alert_visible) return;

    ParentalControlState state = getParentalControlState();

    if(state.alert_type == ALERT_NO_ALERT) return;
    if(state.active) return;

    std::thread overlayThread([](){
        alert_visible = true;
        tsl::loop<OverlayAlert>(0, nullptr);        
    });
    overlayThread.detach();
}

int main(int argc, char** argv) {
    
    // Create data structures
    UserSessions sessions;
    UserSession user;
    GameSession game;
    Settings settings;
    u64 user_daily_limit(0);
    TimeLimitState limit_state(TimeLimitOk);

    // Load database
    /*loadDatabase(sessions);
    settings = loadSettings();*/
    
    logToFile("Starting NS Parental Control service");

    while (true) {
        // We reload the settings each time because they can have been modified from overlay
        /*settings = loadSettings();
        user_daily_limit = settings[SETTING_DAILY_LIMIT_GLOBAL].int_value;*/

        // Load global usage information        
        initParentalControlState();
        ParentalControlState state = getParentalControlState();

        state.today_global_usage_in_secs = user.total_daily_seconds;
        state.today_time_remaining_in_secs = user_daily_limit - user.total_daily_seconds;

        logToFile("showAlert");

        showAlert();

        // If no profile is loaded, we pass...
        /*user = getCurrentUser(sessions);
        if(user.account_id == "") goto do_nothing;

        // If no game is loaded, we pass...
        if(game.game_id == 0) goto do_nothing;

        // Verify whether time limit is reached
        limit_state = verifyTimeLimit(user, game, sessions, settings);                

        switch(limit_state) {
            case TimeLimitWarning: state.alert_type = ALERT_LIMIT_ALMOST_REACHED; break;
            case TimeLimitReached: state.alert_type = ALERT_LIMIT_REACHED; break;
        }

        printf("Time limit is %i\n", limit_state);

        // Update the state
        setParentalControlState(state);

        // Show the alert overlay if needed
        showAlert();*/

do_nothing:
        svcSleepThread(1e9); // 1000 ms
    }

    return 0;
}