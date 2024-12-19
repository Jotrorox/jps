#include "ball.h"
#include <math.h>

Ball create_default_ball(void) {
    return (Ball){
        .x = WINDOW_WIDTH / (2.0f * PIXELS_PER_METER),
        .y = 1.0f,
        .vx = 0.0f,
        .vy = 0.0f,
        .ax = 0.0f,
        .ay = 0.0f,
        .mass = BALL_MASS,
        .radius = BALL_RADIUS,
        .color = {255, 0, 0, 255}
    };
}

float distance_between_balls(const Ball* b1, const Ball* b2) {
    float dx = (b1->x - b2->x);
    float dy = (b1->y - b2->y);
    return sqrtf(dx * dx + dy * dy);
}

void prevent_ball_overlap(Ball* b1, Ball* b2) {
    float dx = b2->x - b1->x;
    float dy = b2->y - b1->y;
    float distance = sqrtf(dx*dx + dy*dy);
    
    if (distance == 0.0f) {
        b2->x += b2->radius;
        return;
    }
    
    float min_dist = b1->radius + b2->radius;
    if (distance < min_dist) {
        float overlap = min_dist - distance;
        float move_x = (dx / distance) * overlap * 0.5f;
        float move_y = (dy / distance) * overlap * 0.5f;
        
        b1->x -= move_x;
        b1->y -= move_y;
        b2->x += move_x;
        b2->y += move_y;
    }
}

void resolve_collision(Ball* b1, Ball* b2) {
    float dx = b2->x - b1->x;
    float dy = b2->y - b1->y;
    float distance = sqrtf(dx*dx + dy*dy);
    
    if (distance == 0.0f) return;
    
    float nx = dx / distance;
    float ny = dy / distance;
    
    float dvx = b2->vx - b1->vx;
    float dvy = b2->vy - b1->vy;
    
    float normal_vel = dvx * nx + dvy * ny;
    
    if (normal_vel > 0) return;
    
    float restitution = COEFFICIENT_OF_RESTITUTION;
    
    float j = -(1.0f + restitution) * normal_vel;
    j /= (1.0f / b1->mass + 1.0f / b2->mass);
    
    float impulse_x = j * nx;
    float impulse_y = j * ny;
    
    b1->vx -= impulse_x / b1->mass;
    b1->vy -= impulse_y / b1->mass;
    b2->vx += impulse_x / b2->mass;
    b2->vy += impulse_y / b2->mass;
} 