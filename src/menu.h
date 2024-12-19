#ifndef MENU_H
#define MENU_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

typedef enum {
    MENU_ACTION_NONE,
    MENU_ACTION_RESET,
    MENU_ACTION_TOGGLE_GRAVITY,
    MENU_ACTION_CLEAR,
    MENU_ACTION_TOGGLE_FPS
} MenuAction;

void draw_menu(SDL_Renderer* renderer, TTF_Font* regular_font, TTF_Font* title_font, SDL_Rect menu_panel);
MenuAction handle_menu_click(int mouseX, int mouseY, SDL_Rect menu_panel);

#endif // MENU_H 