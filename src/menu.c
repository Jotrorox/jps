#include "menu.h"
#include "config.h"

void draw_menu(SDL_Renderer* renderer, TTF_Font* regular_font, TTF_Font* title_font, SDL_Rect menu_panel) {
    SDL_Color text_color = {40, 40, 40, 255};
    SDL_Color title_color = {20, 20, 20, 255};
    SDL_Color hover_color = {230, 230, 230, 255};
    
    // Draw shadow
    SDL_SetRenderDrawColor(renderer, 200, 200, 200, 180);
    SDL_Rect shadow = {menu_panel.x + 3, menu_panel.y + 3, menu_panel.w, menu_panel.h};
    SDL_RenderFillRect(renderer, &shadow);
    
    // Main panel
    SDL_SetRenderDrawColor(renderer, 248, 248, 248, 255);
    SDL_RenderFillRect(renderer, &menu_panel);
    
    // Border
    SDL_SetRenderDrawColor(renderer, 220, 220, 220, 255);
    SDL_RenderDrawRect(renderer, &menu_panel);
    
    // Title
    render_text(renderer, title_font, "Settings", 
                menu_panel.x + menu_panel.w/2 - 50,
                menu_panel.y + TEXT_PADDING, 
                title_color);
    
    // Separator
    SDL_SetRenderDrawColor(renderer, 220, 220, 220, 255);
    SDL_RenderDrawLine(renderer, 
                      menu_panel.x + TEXT_PADDING,
                      menu_panel.y + MENU_TITLE_SIZE + TEXT_PADDING * 2,
                      menu_panel.x + menu_panel.w - TEXT_PADDING,
                      menu_panel.y + MENU_TITLE_SIZE + TEXT_PADDING * 2);

    const char* options[] = {
        "Reset Balls",
        "Toggle Gravity",
        "Clear All",
        "Toggle FPS"
    };
    
    int mouseX, mouseY;
    SDL_GetMouseState(&mouseX, &mouseY);
    
    for (int i = 0; i < 4; i++) {
        SDL_Rect option_rect = {
            menu_panel.x + TEXT_PADDING/2,
            menu_panel.y + MENU_TITLE_SIZE + TEXT_PADDING * 2 + (i * MENU_OPTION_HEIGHT),
            menu_panel.w - TEXT_PADDING,
            MENU_OPTION_HEIGHT
        };
        
        if (mouseX >= option_rect.x && mouseX <= option_rect.x + option_rect.w &&
            mouseY >= option_rect.y && mouseY <= option_rect.y + option_rect.h) {
            SDL_SetRenderDrawColor(renderer, hover_color.r, hover_color.g, hover_color.b, hover_color.a);
            SDL_RenderFillRect(renderer, &option_rect);
        }
        
        render_text(renderer, regular_font, options[i],
                   option_rect.x + TEXT_PADDING,
                   option_rect.y + (MENU_OPTION_HEIGHT - FONT_SIZE)/2,
                   text_color);
    }
}

MenuAction handle_menu_click(int mouseX, int mouseY, SDL_Rect menu_panel) {
    int clickY = mouseY - (menu_panel.y + MENU_TITLE_SIZE + TEXT_PADDING * 2);
    if (clickY >= 0) {
        int option = clickY / MENU_OPTION_HEIGHT;
        switch(option) {
            case 0: return MENU_ACTION_RESET;
            case 1: return MENU_ACTION_TOGGLE_GRAVITY;
            case 2: return MENU_ACTION_CLEAR;
            case 3: return MENU_ACTION_TOGGLE_FPS;
            default: return MENU_ACTION_NONE;
        }
    }
    return MENU_ACTION_NONE;
} 