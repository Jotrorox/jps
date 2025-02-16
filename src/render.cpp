#include "render.hpp"

// Draw the ball using a midpoint circle drawing algorithm.
void renderBall(SDL_Renderer* renderer, const Ball &ball) {
    int centerX = static_cast<int>(ball.x);
    int centerY = static_cast<int>(ball.y);
    int radius = static_cast<int>(ball.radius);

    int dx = radius - 1;
    int dy = 0;
    int err = dx - (radius << 1);

    while (dx >= dy) {
        SDL_RenderDrawPoint(renderer, centerX + dx, centerY + dy);
        SDL_RenderDrawPoint(renderer, centerX + dy, centerY + dx);
        SDL_RenderDrawPoint(renderer, centerX - dy, centerY + dx);
        SDL_RenderDrawPoint(renderer, centerX - dx, centerY + dy);
        SDL_RenderDrawPoint(renderer, centerX - dx, centerY - dy);
        SDL_RenderDrawPoint(renderer, centerX - dy, centerY - dx);
        SDL_RenderDrawPoint(renderer, centerX + dy, centerY - dx);
        SDL_RenderDrawPoint(renderer, centerX + dx, centerY - dy);

        if (err <= 0) {
            dy++;
            err += dy * 2 + 1;
        }
        if (err > 0) {
            dx--;
            err -= dx * 2 + 1;
        }
    }
}