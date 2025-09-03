#include "gui_helpers.h"
#include "structs.h"

std::string makeUserProfileLabel(const UserSession& user) {
    return std::string("User profile: " +(user.account_id == "" ? "No profile" : user.profile_name));
}

std::string makeDailyLimitLabel(Settings& settings) {
    int daily_limit = settings[SETTING_DAILY_LIMIT_GLOBAL].int_value;
    return std::string("Daily limit: ") +std::to_string(daily_limit) +std::string(" minutes");
}

std::string makeRemainingDurationLabel(Settings& settings, UserSessions& sessions) {
    return std::string("Remaining: ") +std::string("??") +std::string(" minutes");
}
 
/*void render_text(SDL_Renderer* renderer, TTF_Font* font, const std::string& label, const SDL_Color& color, int x, int y, int w, int h) {
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
}*/

/*tsl::elm::Element* render_label(const std::string& label, const tsl::Color& color, s32 x, s32 y, s32 w, s32 h) {
    auto text = new tsl::elm::CustomDrawer([=](tsl::gfx::Renderer *renderer, s32 _x, s32 _y, s32 _w, s32 _h) {
        renderer->drawString(label.c_str(),
            false,
            x,
            y,
            h,
            color,
            w
        );
    });

    return text;
}

tsl::elm::Element* render_button(const std::string& label, const tsl::Color& bgColor, const tsl::Color& fgColor, s32 x, s32 y, s32 w, s32 h, bool active = false) {
    auto button = new tsl::elm::CustomDrawer([=](tsl::gfx::Renderer* r, s32 _x, s32 _y, s32 _w, s32 _h) {
        // Rectangle
        int rectX = w/4;
        int rectY = h/4;
        int rectW = w/2;
        int rectH = h/2;

        // Bordure
        r->drawRect(rectX, rectY, rectW, rectH, fgColor);

        // Texte centré
        r->drawString(label.c_str(), false,
            rectX + rectW/2, rectY + rectH/2,
            h,
            fgColor, 
            w); 
    });

    return button;
}

tsl::elm::Element* render_masked_pin(const tsl::Color& color, s32 y, s32 width, const std::string& pin_entered) {
    std::string pin_mask("");
    switch(pin_entered.length()) {
        case 0: pin_mask = std::string("_ _ _ _"); break;
        case 1: pin_mask = std::string("* _ _ _"); break;
        case 2: pin_mask = std::string("* * _ _"); break;
        case 3: pin_mask = std::string("* * * _"); break;
        case 4: pin_mask = std::string("* * * *"); break;
    }    

    return render_label(pin_mask, color, 0, y, width, 50);
}*/