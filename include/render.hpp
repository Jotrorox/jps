#ifndef RENDER_HPP
#define RENDER_HPP

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include "object.hpp"

// A helper that calls the object's render() method.
void renderObject(SDL_Renderer* renderer, const Object* obj);
void renderDebugInfo(SDL_Renderer* renderer, TTF_Font* font, const Object* obj);

#endif // RENDER_HPP