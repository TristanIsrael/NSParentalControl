/*
 * Copyright (c) Atmosphère-NX
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms and conditions of the GNU General Public License,
 * version 2, as published by the Free Software Foundation.
 *
 * This program is distributed in the hope it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#pragma once
#include <switch.h>
#include <vector>
#include "ipc/Server.hpp"
#include "gui_controller.h"
//#include "pctrl_screen.hpp"

namespace alefbet::pctrl::srv {

    class Monitor;
    constexpr SmServiceName service_name_ = smEncodeName("pctrl");    

    class Service {
        public:
            Service(Ipc::Server* ipcServer);
            ~Service();
            
            void listen();
            GuiController&& gui() {
                return std::move(gui_);
            }

            void setMonitor(Monitor* monitor) {
                monitor_ = monitor;
            }

            void showScreenTimeout();
            void showScreenTitleBlacklisted();

        private:
            Ipc::Result commandThread(Ipc::Request*);

            // Handlers            
            Ipc::Result getRunningApplication(Ipc::Request*);
            Ipc::Result getCurrentUserUid(Ipc::Request*);
            Ipc::Result getCurrentUserNickname(Ipc::Request*);
            Ipc::Result getUsersList(Ipc::Request*);
            Ipc::Result getUserUsageTime(Ipc::Request*);
            Ipc::Result getUserRemainingTime(Ipc::Request*);
            Ipc::Result setUserLimits(Ipc::Request*);
            Ipc::Result setAdminPin(Ipc::Request*);
            Ipc::Result verifyAdminPin(Ipc::Request*);
            Ipc::Result setWorkingMode(Ipc::Request*);
            Ipc::Result getWorkingMode(Ipc::Request*);
            Ipc::Result setShowRemainingTime(Ipc::Request*);
            Ipc::Result getShowRemainingTime(Ipc::Request*);
            Ipc::Result isEnabled(Ipc::Request*);
            Ipc::Result setEnabled(Ipc::Request*);
            Ipc::Result getCurrentVersion(Ipc::Request*);
            Ipc::Result getDailyLimit(Ipc::Request*);
            Ipc::Result setDailyLimit(Ipc::Request*);
            Ipc::Result setLogLevel(Ipc::Request*);
            Ipc::Result getLogLevel(Ipc::Request*);
            Ipc::Result isDatabaseTampered(Ipc::Request*);
            Ipc::Result isMustUpgradeDatabase(Ipc::Request*);
            Ipc::Result getBlacklistedTitlesCount(Ipc::Request*);
            Ipc::Result getBlacklistedTitle(Ipc::Request*);
            Ipc::Result addTitleToBlacklist(Ipc::Request*);
            Ipc::Result removeTitleFromBlacklist(Ipc::Request*);

        private:
            bool is_ready_ = false;
            bool session_opened_ = false;
            Ipc::Server *ipcServer_ = nullptr;
            GuiController gui_;
            bool enabled_ = true;
            Monitor* monitor_ = nullptr;
    };

}

