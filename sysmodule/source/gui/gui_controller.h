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

class GuiController {
    public:
        void showScreenTimeout();
        void hideScreenTimeout();

        // Disabled because of the conflict with Tesla... to be solved
        /*void showRemainingTimePanel();
        void updateRemainingTimePanel(const u16& remaining_time_in_minutes, const u16& limit_in_minutes);
        void hideRemainingTimePanel();
        
        inline bool isRemainingTimePanelVisible() {
            return remaining_time_visible_;
        }*/

    private:
        //void showOverlay(u16 width, u16 height, u16 posX, u16 posY);
        void clearScreen();
        
    private:
        u16 width_ = 0;
        u16 height_ = 0;
        bool remaining_time_visible_ = false;
};
