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

#pragma once
#include <switch.h>
#include "database/history.h"
#include "service.h"
#include "helpers.h"

using alefbet::pctrl::helpers;

namespace alefbet::pctrl::srv {

    /*! \brief The monitor class is used to monitor applications usage and update the database records.
    */
    class Monitor {
        public:            
            void start();
            void stop();
            void loop();
            bool isRunning() const {
                return running_;
            }            

            void setService(alefbet::pctrl::srv::Service* service) {
                service_ = service;
                service->setMonitor(this);
            }

            u16 getAdjustedCurrentTime() {
                const auto currentTime = getCurrentTimeInMinutesSinceMidnight();
                u16 result = getAdjustedTime(currentTime, lastTime_, timeJumpOffset_);
                lastTime_ = currentTime;
                return result;
            }

            void resetTimeJumpState() {
                lastTime_ = 0;
                timeJumpOffset_ = 0;
            }

        private:
            bool shouldSendNotification(int remainingTimeInMinutes);

        private:
            alefbet::pctrl::srv::Service* service_ = nullptr;
            bool running_ = false;
            bool notified_ = false;
            u64 currentTitle_ = 0;
            u16 lastTime_ = 0;
            s32 timeJumpOffset_ = 0;
    };    

};