#pragma once

/*#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>*/
#include "structs.h"

/*const SDL_Color ColorFrameBackground = { 0x0, 0x0, 0x0, 0xDD };
const SDL_Color ColorTransparent = { 0x0, 0x0, 0x0, 0x0 };
const SDL_Color ColorHighlight = { 0x0, 0xFF, 0xDD, 0xFF };
const SDL_Color ColorFrame = { 0x77, 0x77, 0x77, 0xFF };
const SDL_Color ColorHandle = { 0x55, 0x55, 0x55, 0xFF };
const SDL_Color ColorText = { 0xFF, 0xFF, 0xFF, 0xFF };
const SDL_Color ColorDescription = { 0xAA, 0xAA, 0xAA, 0xFF };
const SDL_Color ColorHeaderBar = { 0xCC, 0xCC, 0xCC, 0xFF };
const SDL_Color ColorClickAnimation = { 0x0, 0x22, 0x22, 0xFF };
const SDL_Color ColorAlertBackground = { 0x22, 0x22, 0x22, 0x77 };
const SDL_Color ColorError = { 0xFF, 0x00, 0x00, 0xFF };*/

/*tsl::elm::Element* render_label(const std::string& label, const tsl::Color& color, s32 x, s32 y, s32 w, s32 h);

tsl::elm::Element* render_button(const std::string& label, const tsl::Color& bgColor, const tsl::Color& fgColor, s32 x, s32 y, s32 w, s32 h, bool active = false);

tsl::elm::Element* render_masked_pin(const tsl::Color& color, s32 y, s32 width);*/

/*void render_text(SDL_Renderer* renderer, TTF_Font* font, const std::string& label, const SDL_Color& color, int x, int y, int w, int h);

void render_button(SDL_Renderer* renderer, TTF_Font* font, const std::string& label, const SDL_Color& bgColor, const SDL_Color& fgColor, int x, int y, int w, int h, bool active = false);*/

std::string makeUserProfileLabel(const UserSession& user);

std::string makeDailyLimitLabel(Settings& settings);

std::string makeRemainingDurationLabel(Settings& settings, UserSessions& sessions);
