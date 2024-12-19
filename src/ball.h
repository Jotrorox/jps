#ifndef BALL_H
#define BALL_H

#include <SDL2/SDL.h>
#include "config.h"

typedef struct {
    float x, y;        // position in meters
    float vx, vy;      // velocity in m/s
    float ax, ay;      // acceleration in m/s²
    float mass;        // mass in kg
    float radius;      // radius in meters
    SDL_Color color;   // color
} Ball;

// Ball management functions
Ball create_default_ball(void);
float distance_between_balls(const Ball* b1, const Ball* b2);
void prevent_ball_overlap(Ball* b1, Ball* b2);
void resolve_collision(Ball* b1, Ball* b2);

#endif // BALL_H 