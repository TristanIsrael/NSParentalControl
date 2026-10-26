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

#include "notifications_controller.h"
#include "ultrahand_interface.h"
#include "i18n.h"
#include <switch.h>
#include <chrono>

using namespace std::chrono;
using namespace alefbet::pctrl::i18n;

/*! @brief Sends a notification that parental control monitoring has started. */
void NotificationsController::notifyMonitoringStarted() 
{
    UltraHandInterface::writeNotification(tr(Key::MonitoringStarted));
}

/*! @brief Sends an attention notification with the remaining time.
 * @param remainingTimeInMinutes The remaining time in minutes. */
void NotificationsController::notifyRemainingTime(int remainingTimeInMinutes)
{
    std::string message = format(Key::TimeLeft, formatDuration(remainingTimeInMinutes));

    UltraHandInterface::drawAttention();
    UltraHandInterface::writeNotification(message);
}

/*! @brief Sends an attention notification with the remaining seconds.
 * @param remainingSeconds The remaining time in seconds. */
void NotificationsController::notifyRemainingSeconds(int remainingSeconds)
{
    std::string message = format(Key::TimeLeft, std::to_string(remainingSeconds) + "s");

    UltraHandInterface::drawAttention();
    UltraHandInterface::writeNotification(message);
}