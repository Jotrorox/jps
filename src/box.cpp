#include "box.hpp"
#include <SDL2/SDL.h>

constexpr int WINDOW_WIDTH = 800;
constexpr int WINDOW_HEIGHT = 600;
constexpr float BOUNCE_DAMPING = 0.7f;

void Box::updatePhysics(float /*dt*/) {
    // Boxes are static; do nothing.
}

void Box::render(SDL_Renderer* renderer) const {
    SDL_Rect rect;
    rect.w = static_cast<int>(width);
    rect.h = static_cast<int>(height);
    rect.x = static_cast<int>(x - width * 0.5f);
    rect.y = static_cast<int>(y - height * 0.5f);
    SDL_RenderFillRect(renderer, &rect);
}