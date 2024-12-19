#ifndef RENDER_H
#define RENDER_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include "ball.h"

void render_ball(SDL_Renderer* renderer, const Ball* ball);
void render_text(SDL_Renderer* renderer, TTF_Font* font, const char* text, int x, int y, SDL_Color color);
void render_fps(SDL_Renderer* renderer, TTF_Font* font, int fps);

#endif // RENDER_H 