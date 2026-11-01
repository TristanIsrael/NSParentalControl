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

#include "ultrahand_interface.h"
#include <ctime>
#include <cstring>
#include <format>
#include <sstream>
#include <switch.h>
#include "logger.h"
#include "i18n/i18n.h"

using namespace alefbet::pctrl::logger;
using namespace alefbet::pctrl::i18n;

const char* FLAGPATH = "/config/ultrahand/flags/NOTIFICATIONS.flag";
static FsFileSystem sdmc;

/*! @brief Writes a notification JSON file for Ultrahand to display.
 * @param message The notification text message.
 * @param fontSize The font size for the notification.
 * @param priority The priority level of the notification. */
void UltraHandInterface::writeNotification(const std::string& message, int fontSize, int priority) 
{    
    if(!R_SUCCEEDED(fsOpenSdCardFileSystem(&sdmc))) {
        logError("[Ultrahand] Could not open SD Card\n");
        return;
    }

    FsFile handle;
    FsDirEntryType fileType;    

    if (R_FAILED(fsFsGetEntryType(&sdmc, FLAGPATH, &fileType))) {
        logInfo("[Ultrahand] Notifications are not enabled in Ultrahand\n");
        fsFsClose(&sdmc);
        return;
    }

    u64 tick = armGetSystemTick();
    std::string filename = "NSParentalControl-" + std::to_string(tick) + ".notify";
    std::string fullPath = "/config/ultrahand/notifications/" + filename;

    if(R_FAILED(fsFsCreateFile(&sdmc, fullPath.c_str(), 0, 0))) {
        logError("[Ultrahand] Could not create notification file\n");
        fsFsClose(&sdmc);
        return;
    }

    if(R_SUCCEEDED(fsFsOpenFile(&sdmc, fullPath.c_str(), FsOpenMode_Write | FsOpenMode_Append, &handle))) {
        std::ostringstream notif;
        notif << "{\n";
        notif << "  \"text\": \"" << message << "\",\n";
        notif << "  \"font_size\": " << fontSize << ",\n";
        notif << "  \"priority\": " << priority << "\n";
        notif << "}\n";

        const auto s_data = notif.str();

        if(R_SUCCEEDED(fsFileWrite(&handle, 0, s_data.data(), s_data.length(), FsWriteOption_Flush))) {
            logDebug("[Ultrahand] Notification file has been written\n");
        } else {
            logError("[Ultrahand] Notification file has not been written\n");
        }

        fsFileClose(&handle);
    }

    fsFsClose(&sdmc);
}

/*! \brief Shows a short notification to draw users attention */
void UltraHandInterface::drawAttention() 
{    
    writeNotification(tr("Parental Control"), 32, 2);
}