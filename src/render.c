#include "render.h"
#include "config.h"

void render_ball(SDL_Renderer* renderer, const Ball* ball) {
    int x = (int)(ball->x * PIXELS_PER_METER);
    int y = (int)(ball->y * PIXELS_PER_METER);
    int size = BALL_SIZE;
    
    SDL_SetRenderDrawColor(renderer, ball->color.r, ball->color.g, ball->color.b, ball->color.a);
    for (int w = 0; w < size; w++) {
        for (int h = 0; h < size; h++) {
            int dx = size/2 - w;
            int dy = size/2 - h;
            if ((dx*dx + dy*dy) <= ((size/2) * (size/2))) {
                SDL_RenderDrawPoint(renderer, x + w, y + h);
            }
        }
    }
}

void render_text(SDL_Renderer* renderer, TTF_Font* font, const char* text, int x, int y, SDL_Color color) {
    SDL_Surface* surface = TTF_RenderText_Blended(font, text, color);
    if (!surface) {
        return;
    }
    
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_Rect dest = {x, y, surface->w, surface->h};
    
    SDL_RenderCopy(renderer, texture, NULL, &dest);
    
    SDL_FreeSurface(surface);
    SDL_DestroyTexture(texture);
}

void render_fps(SDL_Renderer* renderer, TTF_Font* font, int fps) {
    char fps_text[32];
    snprintf(fps_text, sizeof(fps_text), "FPS: %d", fps);
    
    SDL_Color fps_color = {0, 0, 0, 255};
    render_text(renderer, font, fps_text, 10, 10, fps_color);
} 