#include "ball.hpp"
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <cmath>
#include <algorithm>

constexpr int WINDOW_WIDTH = 800;
constexpr int WINDOW_HEIGHT = 600;
constexpr float GRAVITY = 980.0f;
constexpr float AIR_DRAG = 0.1f;
constexpr float GROUND_FRICTION = 500.0f;
constexpr float BOUNCE_DAMPING = 0.7f;

// RK4 integration helper function.
static void RK4Step(float &pos, float &vel, float dt, float (*acceleration)(const float, const float)) {
    float k1_v = acceleration(pos, vel);
    float k1_x = vel;

    float v_temp = vel + 0.5f * dt * k1_v;
    float k2_v = acceleration(pos, v_temp);
    float k2_x = v_temp;

    v_temp = vel + 0.5f * dt * k2_v;
    float k3_v = acceleration(pos, v_temp);
    float k3_x = v_temp;

    v_temp = vel + dt * k3_v;
    float k4_v = acceleration(pos, v_temp);
    float k4_x = v_temp;

    pos += dt / 6.0f * (k1_x + 2.0f*k2_x + 2.0f*k3_x + k4_x);
    vel += dt / 6.0f * (k1_v + 2.0f*k2_v + 2.0f*k3_v + k4_v);
}

// Acceleration functions: horizontal (drag only) and vertical (gravity and drag).
static float accelerationX(const float, const float v) {
    return -AIR_DRAG * v;
}

static float accelerationY(const float, const float v) {
    return GRAVITY - AIR_DRAG * v;
}

void Ball::updatePhysics(float dt) {
    RK4Step(x, vx, dt, accelerationX);
    RK4Step(y, vy, dt, accelerationY);

    float half = radius;
    // Floor collision
    if (y + half > WINDOW_HEIGHT) {
        y = WINDOW_HEIGHT - half;
        vy = -vy * BOUNCE_DAMPING;
        float frictionDelta = GROUND_FRICTION * dt;
        if (std::fabs(vx) < frictionDelta)
            vx = 0;
        else
            vx -= (vx > 0 ? frictionDelta : -frictionDelta);
    }
    // Ceiling collision
    if (y - half < 0) {
        y = half;
        vy = -vy * BOUNCE_DAMPING;
    }
    // Left wall collision
    if (x - half < 0) {
        x = half;
        vx = -vx * BOUNCE_DAMPING;
    }
    // Right wall collision
    if (x + half > WINDOW_WIDTH) {
        x = WINDOW_WIDTH - half;
        vx = -vx * BOUNCE_DAMPING;
    }
}

void Ball::render(SDL_Renderer* renderer) const {
    int centerX = static_cast<int>(x);
    int centerY = static_cast<int>(y);
    int r = static_cast<int>(radius);
    int dx = r - 1, dy = 0;
    int err = dx - (r << 1);
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

void Ball::renderVelocityInfo(SDL_Renderer* renderer, TTF_Font* font) const {
    std::stringstream ss;
    ss << "v: (" << static_cast<int>(vx) << ", " << static_cast<int>(vy) << ")";
    SDL_Color white = {255, 255, 255, 255};
    SDL_Surface* surface = TTF_RenderText_Solid(font, ss.str().c_str(), white);
    if (surface) {
        SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
        if (texture) {
            SDL_Rect rect;
            rect.x = static_cast<int>(x - surface->w / 2);
            rect.y = static_cast<int>(y - radius - surface->h - 2);
            rect.w = surface->w;
            rect.h = surface->h;
            SDL_RenderCopy(renderer, texture, nullptr, &rect);
            SDL_DestroyTexture(texture);
        }
        SDL_FreeSurface(surface);
    }
}