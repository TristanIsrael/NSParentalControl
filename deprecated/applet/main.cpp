#include <switch.h>
#include <string>
#include <switch.h>
#include <unistd.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include "gui_helpers.h"
#include "structs.h"
#include "database.h"

#define HEIGHT_PORTABLE 720
#define WIDTH_PORTABLE 1280
#define LABEL_FONT_SIZE 16
#define TITLE_FONT_SIZE 24
#define LINE_SPACING 16
#define FONT_PATH "data/NotoSans-VariableFont_wdth,wght.ttf"

SharedMemory shm;
ParentalControlState* state;

int main(int argc, char* argv[]) {
    // Initialize access to resources
    romfsInit();
    chdir("romfs:/");

    // Initialisation des services graphiques et HID
    SDL_Init(SDL_INIT_VIDEO);
    TTF_Init();    

    // Initialise memory
    initState(state);

    // Load settings
    Settings settings = loadSettings();

    Handle shmemHandle = settings[SETTING_SHMEM_HANDLE].int_value;
    if(shmemHandle == 0) {
        SDL_Log("There is no Handle for shmem");
        SDL_Quit();
        return -1;
    }

    // Initialize shared memory    
    shmemLoadRemote(&shm, shmemHandle, sizeof(ParentalControlState), Perm_Rw);
    Result rc = shmemMap(&shm);
    if(R_FAILED(rc)) {
        SDL_Log("Could not map shmem");
        SDL_Quit();
        return -1;
    }
    
    int screen_height = HEIGHT_PORTABLE;
    int screen_width = WIDTH_PORTABLE;
    int alert_width = screen_width /2;
    int alert_height = screen_height /2;

    SDL_Window* window = SDL_CreateWindow("Parental Control", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, alert_width, alert_height, SDL_WINDOW_SHOWN);
    if (!window) {
        SDL_Log("SDL_CreateWindow: %s\n", SDL_GetError());
        SDL_Quit();
        return -1;
    }
    SDL_Renderer* renderer = SDL_CreateRenderer(window, 0, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK) < 0) {
        SDL_Log("SDL_Init: %s\n", SDL_GetError());
        return -1;
    }

    TTF_Font* labelFont = TTF_OpenFont(FONT_PATH, LABEL_FONT_SIZE); 
    if (!labelFont) {
        SDL_Log("Error TTF_OpenFont: %s\n", TTF_GetError());
        return -1;
    }

    TTF_Font* titleFont = TTF_OpenFont(FONT_PATH, TITLE_FONT_SIZE);     
    
    // Initialize the default gamepad (which reads handheld mode inputs as well as the first connected controller)
    padConfigureInput(1, HidNpadStyleSet_NpadStandard);
    PadState pad;
    padInitializeDefault(&pad);
    bool running(true);    

    AlertType alert_mode(ALERT_NO_ALERT);
    uint16_t remaining_seconds(0);

    while (appletMainLoop()) {
        padUpdate(&pad);

        u64 kDown = padGetButtonsDown(&pad);        
        u64 kHeld = padGetButtons(&pad);

        // Quit with B
        if (kDown & HidNpadButton_B) break;

        // Monitor alerts
        remaining_seconds = 500;
        // TODO
        
        // If there is no alert, we do nothing
        if(alert_mode == ALERT_NO_ALERT) {
            continue;
        }

        // Show GUI if needed
        SDL_Event e;
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) running = false;
        }

        //Gray: 50, 50, 50, 255
        SDL_SetRenderDrawColor(renderer, 230, 230, 230, 255);
        SDL_RenderClear(renderer);

        //Overlay limits
        SDL_Rect rect = { 0, 0, alert_width, alert_height };
        SDL_SetRenderDrawColor(renderer, ColorAlertBackground.r, ColorAlertBackground.g, ColorAlertBackground.b, ColorAlertBackground.a);
        SDL_RenderFillRect(renderer, &rect);

        //Overlay title
        render_text(renderer, titleFont, std::string("Parental Control"), ColorText, 0, 0, alert_width, 50);
        
        std::string alert_text = "";
        switch(alert_mode) {
            case ALERT_LIMIT_ALMOST_REACHED: alert_text = std::string("The playing limit is almost reached. Please quit the game."); break;
            case ALERT_LIMIT_REACHED: alert_text = std::string("The playing limit is reached. The game will be quit."); break;
            default: alert_text = std::string("ERR#1");
        }

        render_text(renderer, labelFont, alert_text, ColorError, 0, 0, alert_width, alert_height);

        if(alert_mode == ALERT_LIMIT_ALMOST_REACHED) {            
            std::string remaining_text = std::string("Remaining : " +std::to_string(remaining_seconds) +std::string(" seconds"));
            render_text(renderer, labelFont, remaining_text, ColorText, 0, 20, alert_width, alert_height);
        }

        SDL_RenderPresent(renderer);
        SDL_Delay(16); // ~60fps
    }
    
    // Clean 
    TTF_Quit();
    SDL_Quit();
    romfsExit();

    return 0;
}