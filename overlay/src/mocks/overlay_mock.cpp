//gcc -I/opt/homebrew/Cellar/sdl2/2.32.10/include -L/opt/homebrew/Cellar/sdl2/2.32.10/lib -L/opt/homebrew/Cellar/sdl2_ttf/2.24.0/lib -lsdl2 -lsdl2_ttf -lstdc++ gui_mock.cpp

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <iostream>

#define TVMODE false
#define HEIGHT_PORTABLE 720
#define WIDTH_PORTABLE 1280
#define HEIGHT_TV 1024
#define WIDTH_TV 1920
#define LABEL_FONT_SIZE 16
#define TITLE_FONT_SIZE 24
#define LINE_SPACING 16

std::string pin_entered("12");

struct Button {
    int x, y, w, h;
    std::string label;
};

const SDL_Color ColorFrameBackground = { 0x0, 0x0, 0x0, 0xDD };
const SDL_Color ColorTransparent = { 0x0, 0x0, 0x0, 0x0 };
const SDL_Color ColorHighlight = { 0x0, 0xFF, 0xDD, 0xFF };
const SDL_Color ColorFrame = { 0x77, 0x77, 0x77, 0xFF };
const SDL_Color ColorHandle = { 0x55, 0x55, 0x55, 0xFF };
const SDL_Color ColorText = { 0xFF, 0xFF, 0xFF, 0xFF };
const SDL_Color ColorDescription = { 0xAA, 0xAA, 0xAA, 0xFF };
const SDL_Color ColorHeaderBar = { 0xCC, 0xCC, 0xCC, 0xFF };
const SDL_Color ColorClickAnimation = { 0x0, 0x22, 0x22, 0xFF };

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

void render_masked_pin(SDL_Renderer* renderer, TTF_Font* font, const SDL_Color& color, int y, int width) {
    std::string pin_mask("");
    switch(pin_entered.length()) {
        case 0: pin_mask = std::string("_ _ _ _"); break;
        case 1: pin_mask = std::string("* _ _ _"); break;
        case 2: pin_mask = std::string("* * _ _"); break;
        case 3: pin_mask = std::string("* * * _"); break;
        case 4: pin_mask = std::string("* * * *"); break;
    }    

    render_text(renderer, font, pin_mask, color, 0, y, width, 50);
}

int main() {
    SDL_Init(SDL_INIT_VIDEO);
    TTF_Init();    

    int screen_height = TVMODE ? HEIGHT_TV : HEIGHT_PORTABLE;
    int screen_width = TVMODE ? WIDTH_TV : WIDTH_PORTABLE;
    int overlay_width = screen_width * 0.3125;

    SDL_Window* window = SDL_CreateWindow("Overlay Preview", SDL_WINDOWPOS_CENTERED, 
        SDL_WINDOWPOS_CENTERED, screen_width, screen_height, 0);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    TTF_Font* labelFont = TTF_OpenFont("/Users/tristanisrael/Library/Fonts/NotoSans-VariableFont_wdth,wght.ttf", LABEL_FONT_SIZE); 
    if (!labelFont) {
        std::cerr << "Error TTF_OpenFont: " << TTF_GetError() << std::endl;
        return 1;
    }

    TTF_Font* titleFont = TTF_OpenFont("/Users/tristanisrael/Library/Fonts/NotoSans-VariableFont_wdth,wght.ttf", TITLE_FONT_SIZE);     
    
    int currentY(0);

    bool running = true;
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
        SDL_Rect rect = { 0, 0, overlay_width, screen_height };
        SDL_SetRenderDrawColor(renderer, 50, 50, 50, 255);
        SDL_RenderFillRect(renderer, &rect);

        //Overlay title
        render_text(renderer, titleFont, std::string("Parental Control"), ColorText, 0, 0, overlay_width, 50);
        currentY += 50 + LINE_SPACING;
        
        //Curent limit
        render_text(renderer, labelFont, std::string("Daily limit : ") + std::to_string(60) + std::string(" minutes"), ColorText, 0, currentY, overlay_width, LABEL_FONT_SIZE);
        currentY += LABEL_FONT_SIZE + LINE_SPACING;

        //Remaining time
        render_text(renderer, labelFont, std::string("Remaining : ") + std::to_string(14) + std::string(" minutes"), ColorText, 0, currentY, overlay_width, LABEL_FONT_SIZE);
        currentY += LABEL_FONT_SIZE + LINE_SPACING;

        //Setup button
        render_button(renderer, labelFont, std::string("Setup"), ColorTransparent, ColorText, (overlay_width - overlay_width / 3)/2, screen_height - 60, overlay_width / 3, 50);
        currentY += 50 + LINE_SPACING;

        //Show the PIN layout
        if(true) {
            //render_text(renderer, titleFont, std::string("_ _ _ _"), textColor, 0, currentY, overlay_width, 50);
            render_masked_pin(renderer, titleFont, ColorText, currentY, overlay_width);
            currentY += 50 + LINE_SPACING;

            int button_width = 50;
            int button_height = 40;
            int spacing = 8;

            //Draw the buttons
            //7..9
            int button_x = (overlay_width - (button_width*3 + spacing*2))/2;
            render_button(renderer, labelFont, std::to_string(7), ColorTransparent, ColorText, button_x, currentY, button_width, button_height);
            button_x += button_width + spacing;
            render_button(renderer, labelFont, std::to_string(8), ColorTransparent, ColorText, button_x, currentY, button_width, button_height);
            button_x += button_width + spacing;
            render_button(renderer, labelFont, std::to_string(9), ColorTransparent, ColorText, button_x, currentY, button_width, button_height);            
            currentY += button_height + spacing;

            //4..6
            button_x = (overlay_width - (button_width*3 + spacing*2))/2;
            render_button(renderer, labelFont, std::to_string(4), ColorTransparent, ColorText, button_x, currentY, button_width, button_height);
            button_x += button_width + spacing;
            render_button(renderer, labelFont, std::to_string(5), ColorTransparent, ColorText, button_x, currentY, button_width, button_height);
            button_x += button_width + spacing;
            render_button(renderer, labelFont, std::to_string(6), ColorTransparent, ColorText, button_x, currentY, button_width, button_height);            
            currentY += button_height + spacing;

            //1..3
            button_x = (overlay_width - (button_width*3 + spacing*2))/2;
            render_button(renderer, labelFont, std::to_string(1), ColorTransparent, ColorText, button_x, currentY, button_width, button_height);
            button_x += button_width + spacing;
            render_button(renderer, labelFont, std::to_string(2), ColorTransparent, ColorText, button_x, currentY, button_width, button_height);
            button_x += button_width + spacing;
            render_button(renderer, labelFont, std::to_string(3), ColorTransparent, ColorText, button_x, currentY, button_width, button_height);            
            currentY += button_height + spacing;

            //0
            button_x = (overlay_width - (button_width*3 + spacing*2))/2 +button_width + spacing;
            render_button(renderer, labelFont, std::to_string(0), ColorTransparent, ColorText, button_x, currentY, button_width, button_height, true);            

            currentY += 50 + LINE_SPACING;
        }        

        SDL_RenderPresent(renderer);
        SDL_Delay(16); // ~60fps
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}