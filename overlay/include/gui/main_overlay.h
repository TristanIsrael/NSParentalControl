#pragma once

#include "switch.h"
#include "structs.h"

class MainOverlay : public tsl::Overlay {
public:    
    MainOverlay();
    
    void initServices() override;
    void exitServices() override;

    virtual void onShow() override {}
    virtual void onHide() override {}

    virtual std::unique_ptr<tsl::Gui> loadInitialGui() override;

private:
    bool active_ = false;
    UserSession user_;
    GameSession game_;
    UserSessions sessions_;
    Settings settings_;
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