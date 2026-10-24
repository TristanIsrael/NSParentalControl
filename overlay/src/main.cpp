/*
 * NS Parental Control
 * Copyright (C) 2025 Tristan Israël
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include <sys/stat.h>
#include <sys/types.h>
#include <vector>
#include <map>
#include <ctime>
#include <cstring>
#include <switch.h>

#define TESLA_INIT_IMPL
#include <tesla.hpp>
#include "gui/main_overlay.h"
#include "logger.h"
#include "helpers/ipc_helpers.h"
#include "AppContext.h"

//Examples : https://github.com/masagrator/Status-Monitor-Overlay/blob/master/source/main.cpp

using namespace alefbet::pctrl::logger;

constexpr SmServiceName service_name = smEncodeName("pctrl");

/*! @brief Connects to the pctrl service and initializes the service session.
 * @return true if connected successfully, false otherwise. */
bool connectToService() {
    u8 exists(0);
    Result res = tipcDispatchInOut(smGetServiceSessionTipc(), 65100, service_name, exists);
    if(R_FAILED(res) || !exists) {
        getAppContext().is_available = false;
        logError("[Main] could not find the service\n");
        return false;
    } 
        
    getAppContext().is_available = true;
    logDebug("[Main] the service pctrl has been found\n");    

    res = smGetServiceWrapper(&getAppContext().pctrl_service, service_name);
    if(R_FAILED(res)) {
        logError("[Overlay] could not open service (%i.%i)\n", R_MODULE(res), R_DESCRIPTION(res));
        return false;
    }

    logInfo("[Main] Connected to pctrl service\n");
    logDebug("[Main] Session=%i\n", getAppContext().pctrl_service.session);
    getAppContext().is_enabled = true;

    return true;
}

/*! @brief Entry point for the overlay application, initializing services and running the UI loop.
 * @param argc Argument count.
 * @param argv Argument values.
 * @return Exit code. */
int main(int argc, char** argv) {
    smInitialize();
    fsInitialize();
    fsdevMountSdmc();

    clearLog();

    connectToService();  

    // Get current log level
    bool debugEnabled = alefbet::pctrl::ipc::isDebugLogEnabled();
    getAppContext().is_debug = debugEnabled;
    setLogLevel(debugEnabled ? DEBUG : INFO);  

    tsl::loop<MainOverlay>(argc, argv);    

    logDebug("[Main] Close session\n");
    if(getAppContext().is_enabled) {
        svcCloseHandle(getAppContext().pctrl_service.session);   
    }

    logDebug("[Main] Exiting overlay\n");

    // Clean all
    if(getAppContext().pctrl_service.session > 0) {
        serviceClose(&getAppContext().pctrl_service);
    }
    fsdevUnmountAll(); 
    fsExit(); 
    smExit();    
}