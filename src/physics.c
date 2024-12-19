#include "physics.h"
#include "config.h"
#include <math.h>

void calculate_drag_force(Ball* ball, float* fx, float* fy) {
    float velocity_squared = ball->vx * ball->vx + ball->vy * ball->vy;
    if (velocity_squared < 0.0001f) {
        *fx = 0;
        *fy = 0;
        return;
    }
    
    float area = M_PI * ball->radius * ball->radius;
    float drag_magnitude = 0.5f * AIR_DENSITY * velocity_squared * DRAG_COEFFICIENT * area;
    
    float velocity_magnitude = sqrt(velocity_squared);
    *fx = -(drag_magnitude * ball->vx / velocity_magnitude);
    *fy = -(drag_magnitude * ball->vy / velocity_magnitude);
}

void update_physics(Ball* ball, float delta_time) {
    float drag_force_x, drag_force_y;
    calculate_drag_force(ball, &drag_force_x, &drag_force_y);
    
    ball->ax = drag_force_x / ball->mass;
    ball->ay = (gravity ? GRAVITY_ACCELERATION : 0.0f) + (drag_force_y / ball->mass);
    
    ball->vx += ball->ax * delta_time;
    ball->vy += ball->ay * delta_time;
    
    ball->x += ball->vx * delta_time + 0.5f * ball->ax * delta_time * delta_time;
    ball->y += ball->vy * delta_time + 0.5f * ball->ay * delta_time * delta_time;
    
    float max_x = (WINDOW_WIDTH - BALL_SIZE) / PIXELS_PER_METER;
    float max_y = (WINDOW_HEIGHT - BALL_SIZE) / PIXELS_PER_METER;
    
    if (ball->y > max_y) {
        ball->y = max_y;
        if (ball->vy > 0) {
            ball->vy = -ball->vy * COEFFICIENT_OF_RESTITUTION;
        }
        
        if (fabs(ball->vx) > 0.001f) {
            float normal_force = ball->mass * GRAVITY_ACCELERATION;
            float friction_force = COEFFICIENT_OF_FRICTION * normal_force;
            float friction_deceleration = friction_force / ball->mass;
            
            float friction_delta_v = friction_deceleration * delta_time;
            if (fabs(ball->vx) <= friction_delta_v) {
                ball->vx = 0;
            } else {
                ball->vx -= (ball->vx > 0 ? friction_delta_v : -friction_delta_v);
            }
        }
    }
    
    if (ball->x > max_x) {
        ball->x = max_x;
        if (ball->vx > 0) {
            ball->vx = -ball->vx * COEFFICIENT_OF_RESTITUTION;
        }
    }
    
    if (ball->x < 0) {
        ball->x = 0;
        if (ball->vx < 0) {
            ball->vx = -ball->vx * COEFFICIENT_OF_RESTITUTION;
        }
    }
}

int physics_thread(void* data) {
    ThreadData* thread_data = (ThreadData*)data;
    
    // First update physics for all balls
    for (int i = thread_data->start_index; i < thread_data->end_index; i++) {
        update_physics(&thread_data->balls[i], thread_data->delta_time);
    }
    
    // Then check for collisions
    for (int i = thread_data->start_index; i < thread_data->end_index; i++) {
        // Check against ALL other balls, not just within this thread's range
        for (int j = i + 1; j < MAX_BALLS; j++) {
            float dist = distance_between_balls(&thread_data->balls[i], &thread_data->balls[j]);
            float min_dist = (thread_data->balls[i].radius + thread_data->balls[j].radius);
            
            if (dist < min_dist) {
                SDL_LockMutex(thread_data->mutex);
                prevent_ball_overlap(&thread_data->balls[i], &thread_data->balls[j]);
                resolve_collision(&thread_data->balls[i], &thread_data->balls[j]);
                SDL_UnlockMutex(thread_data->mutex);
            }
        }
    }
    
    return 0;
} 