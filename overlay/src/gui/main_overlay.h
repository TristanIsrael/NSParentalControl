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
#include <tesla.hpp>

class MainOverlay : public tsl::Overlay {
public:    
    MainOverlay();
    
    void initServices() override {}
    void exitServices() override {}

    virtual void onShow() override {}
    virtual void onHide() override {}

    virtual std::unique_ptr<tsl::Gui> loadInitialGui() override;

private:
    
};

//bool askPIN();
//void configureLimits(UserSession &user, GameSession &game);

//A voir:
/*
struct OverlayState {
    bool overlayActive;
};

// Récupération de la SharedMemory créée par le sysmodule
extern SharedMemory shm;
extern OverlayState* state;

class MainOverlay : public tsl::Overlay {
public:
    tsl::elm::Element* createUI() override {
        auto frame = new tsl::elm::OverlayFrame("Alert", "Temps écoulé !");
        return frame;
    }

    void update() override {
        // Vérifier si le sysmodule a demandé l'affichage
        if (state && state->overlayActive) {
            tsl::hlp::dbg::log("Overlay activé automatiquement !");
            this->show();          // afficher l'overlay
            state->overlayActive = false; // reset
        }
    }
};*/