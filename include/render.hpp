#ifndef RENDER_HPP
#define RENDER_HPP

#include <SDL2/SDL.h>
#include "ball.hpp"

// Render the ball using a simple circle drawing algorithm.
void renderBall(SDL_Renderer* renderer, const Ball &ball);

#endif // RENDER_HPP