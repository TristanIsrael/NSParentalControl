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
#include <switch.h>
#include <chrono>

using namespace std::chrono;

/*! @brief Sends a notification that parental control monitoring has started. */
void NotificationsController::notifyMonitoringStarted() 
{
    UltraHandInterface::writeNotification("Parental control enabled");
}

/*! @brief Sends an attention notification with the remaining time.
 * @param remainingTimeInMinutes The remaining time in minutes. */
void NotificationsController::notifyRemainingTime(int remainingTimeInMinutes)
{
    //std::string message = std::to_string(remainingTimeInMinutes) + " minute" +(remainingTimeInMinutes > 1 ? "s" : "") +" left";
    std::string message = formatTime(remainingTimeInMinutes) +" left";

    UltraHandInterface::drawAttention();
    UltraHandInterface::writeNotification(message);
}

/*! @brief Formats a duration in minutes to a human-readable string (e.g., "2h 30mn").
 * @param duration_in_minutes The duration in minutes.
 * @return Formatted time string. */
std::string NotificationsController::formatTime(int duration_in_minutes) {
    const auto& duration = minutes{duration_in_minutes};
    auto hoursPart = duration_cast<hours>(duration);
    auto minutesPart = duration_cast<minutes>(duration - hoursPart);
    std::string formatted = std::to_string(hoursPart.count()) + "h " +std::to_string(minutesPart.count()) +"mn";
    return formatted;
}