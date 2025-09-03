//gcc -I/opt/homebrew/Cellar/sdl2/2.32.10/include -L/opt/homebrew/Cellar/sdl2/2.32.10/lib -L/opt/homebrew/Cellar/sdl2_ttf/2.24.0/lib -lsdl2 -lsdl2_ttf -lstdc++ gui_mock.cpp

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <iostream>

#define HEIGHT_PORTABLE 720
#define WIDTH_PORTABLE 1280
#define HEIGHT_TV 1024
#define WIDTH_TV 1920
#define LABEL_FONT_SIZE 16
#define TITLE_FONT_SIZE 24
#define LINE_SPACING 16
#define FONT_PATH "/Users/tristanisrael/Library/Fonts/NotoSans-VariableFont_wdth,wght.ttf"

struct Button {
    int x, y, w, h;
    std::string label;
};

typedef enum {
    ALERT_NO_ALERT = 0,
    ALERT_LIMIT_ALMOST_REACHED = 1,
    ALERT_LIMIT_REACHED = 2
} AlertType;

const SDL_Color ColorFrameBackground = { 0x0, 0x0, 0x0, 0xDD };
const SDL_Color ColorTransparent = { 0x0, 0x0, 0x0, 0x0 };
const SDL_Color ColorHighlight = { 0x0, 0xFF, 0xDD, 0xFF };
const SDL_Color ColorFrame = { 0x77, 0x77, 0x77, 0xFF };
const SDL_Color ColorHandle = { 0x55, 0x55, 0x55, 0xFF };
const SDL_Color ColorText = { 0xFF, 0xFF, 0xFF, 0xFF };
const SDL_Color ColorDescription = { 0xAA, 0xAA, 0xAA, 0xFF };
const SDL_Color ColorHeaderBar = { 0xCC, 0xCC, 0xCC, 0xFF };
const SDL_Color ColorClickAnimation = { 0x0, 0x22, 0x22, 0xFF };
const SDL_Color ColorAlertBackground = { 0x22, 0x22, 0x22, 0x77 };
const SDL_Color ColorError = { 0xFF, 0x00, 0x00, 0xFF };

void render_text(SDL_Renderer* renderer, TTF_Font* font, const std::string& label, const SDL_Color& color, int x, int y, int w, int h) {
    // Rendu du texte
    SDL_Surface* surface = TTF_RenderText_Blended(font, label.c_str(), color);
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);

    int textW = 0, textH = 0;
    SDL_QueryTexture(texture, NULL, NULL, &textW, &textH);

    SDL_Rect textRect;
    textRect.x = x + (w - textW)/2;
    textRect.y = y + (h - textH)/2;
    textRect.w = textW;
    textRect.h = textH;

    SDL_RenderCopy(renderer, texture, NULL, &textRect);
}

void render_button(SDL_Renderer* renderer, TTF_Font* font, const std::string& label, const SDL_Color& bgColor, const SDL_Color& fgColor, int x, int y, int w, int h, bool active = false) {
    SDL_Rect rect = { x, y, w, h };

    //Fill the rect
    SDL_SetRenderDrawColor(renderer, bgColor.r, bgColor.g, bgColor.b, bgColor.a);
    SDL_RenderFillRect(renderer, &rect);

    //Draw the border if the button is active
    if(active) {
        SDL_SetRenderDrawColor(renderer, ColorHighlight.r, ColorHighlight.g, ColorHighlight.b, ColorHighlight.a);
        SDL_RenderDrawRect(renderer, &rect);
    }
    
    // Rendu du texte  
    render_text(renderer, font, label, fgColor, x, y, w, h); 
}

int main() {
    SDL_Init(SDL_INIT_VIDEO);
    TTF_Init();    

    int screen_height = HEIGHT_PORTABLE;
    int screen_width = WIDTH_PORTABLE;
    int alert_width = screen_width /2;
    int alert_height = screen_height /2;

    SDL_Window* window = SDL_CreateWindow("Parental Control", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, alert_width, alert_height, 0);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, 0, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

    TTF_Font* labelFont = TTF_OpenFont(FONT_PATH, LABEL_FONT_SIZE); 
    if (!labelFont) {
        std::cerr << "Error TTF_OpenFont: " << TTF_GetError() << std::endl;
        return 1;
    }

    TTF_Font* titleFont = TTF_OpenFont(FONT_PATH, TITLE_FONT_SIZE);     
    
    int currentY(0);
    bool running = true;   
    AlertType alert_mode = ALERT_LIMIT_ALMOST_REACHED; 
    uint16_t remaining_seconds = 500;

    while (running) {
        currentY = 0;

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
        currentY += 50 + LINE_SPACING;
        
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

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}