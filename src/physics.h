#ifndef PHYSICS_H
#define PHYSICS_H

#include "ball.h"
#include <SDL2/SDL_thread.h>

typedef struct {
    Ball* balls;
    int start_index;
    int end_index;
    float delta_time;
    SDL_mutex* mutex;
} ThreadData;

void calculate_drag_force(Ball* ball, float* fx, float* fy);
void update_physics(Ball* ball, float delta_time);
int physics_thread(void* data);
int collision_thread(void* data);

#endif // PHYSICS_H 