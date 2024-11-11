#include <SDL2/SDL.h>
#include <stdbool.h>
#include <math.h>
#include <SDL_stdinc.h>
#define _USE_MATH_DEFINES
#include <float.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#define WINDOW_WIDTH 640
#define WINDOW_HEIGHT 480
#define BALL_SIZE 15
#define PIXELS_PER_METER 100.0f  // Conversion factor: 100 pixels = 1 meter

// Physics constants (in real-world units)
#define GRAVITY_ACCELERATION 9.81f  // m/s²
#define TIME_STEP 0.016f           // 16ms in seconds (for 60 FPS)
#define AIR_DENSITY 1.225f         // kg/m³
#define BALL_MASS 0.1f             // kg (100g)
#define BALL_RADIUS (BALL_SIZE / (2.0f * PIXELS_PER_METER))  // Convert to meters
#define DRAG_COEFFICIENT 0.47f     // Sphere drag coefficient
#define COEFFICIENT_OF_RESTITUTION 0.75f  // Slightly more elastic
#define COEFFICIENT_OF_FRICTION 0.15f     // Slightly less friction

#define TARGET_FPS 60
#define TARGET_FRAME_TIME (1000.0f / TARGET_FPS)  // in milliseconds

typedef struct {
    float x, y;        // position in meters
    float vx, vy;      // velocity in m/s
    float ax, ay;      // acceleration in m/s²
    float mass;        // mass in kg
    float radius;      // radius in meters
} Ball;

void calculate_drag_force(Ball* ball, float* fx, float* fy) {
    float velocity_squared = ball->vx * ball->vx + ball->vy * ball->vy;
    if (velocity_squared < 0.0001f) {
        *fx = 0;
        *fy = 0;
        return;
    }
    
    // Drag force magnitude = 0.5 * ρ * v² * Cd * A
    float area = M_PI * ball->radius * ball->radius;
    float drag_magnitude = 0.5f * AIR_DENSITY * velocity_squared * DRAG_COEFFICIENT * area;
    
    float velocity_magnitude = sqrt(velocity_squared);
    *fx = -(drag_magnitude * ball->vx / velocity_magnitude);
    *fy = -(drag_magnitude * ball->vy / velocity_magnitude);
}

void update_physics(Ball* ball) {
    // Calculate drag forces
    float drag_force_x, drag_force_y;
    calculate_drag_force(ball, &drag_force_x, &drag_force_y);
    
    // Calculate accelerations (F = ma)
    ball->ax = drag_force_x / ball->mass;
    ball->ay = GRAVITY_ACCELERATION + (drag_force_y / ball->mass);
    
    const float time_step = 1.0f / TARGET_FPS;  // Use fixed time step
    
    // Update velocities (v = v₀ + at)
    ball->vx += ball->ax * time_step;
    ball->vy += ball->ay * time_step;
    
    // Store previous position for collision detection
    float prev_x = ball->x;
    float prev_y = ball->y;
    
    // Update positions (x = x₀ + vt + ½at²)
    ball->x += ball->vx * time_step + 0.5f * ball->ax * time_step * time_step;
    ball->y += ball->vy * time_step + 0.5f * ball->ay * time_step * time_step;
    
    // Handle collisions
    float floor_height = WINDOW_HEIGHT / PIXELS_PER_METER;
    float right_wall = WINDOW_WIDTH / PIXELS_PER_METER;
    
    // Floor collision
    if (ball->y + ball->radius > floor_height) {
        // Calculate the exact collision point
        float penetration = (ball->y + ball->radius) - floor_height;
        ball->y = floor_height - ball->radius;
        
        // Only bounce if moving downward
        if (ball->vy > 0) {
            ball->vy = -ball->vy * COEFFICIENT_OF_RESTITUTION;
        }
        
        // Apply ground friction to horizontal velocity
        if (fabs(ball->vx) > 0.001f) {
            float normal_force = ball->mass * GRAVITY_ACCELERATION;
            float friction_force = COEFFICIENT_OF_FRICTION * normal_force;
            float friction_deceleration = friction_force / ball->mass;
            
            float friction_delta_v = friction_deceleration * time_step;
            if (fabs(ball->vx) <= friction_delta_v) {
                ball->vx = 0;
            } else {
                ball->vx -= (ball->vx > 0 ? friction_delta_v : -friction_delta_v);
            }
        }
    }
    
    // Right wall collision
    if (ball->x + ball->radius > right_wall) {
        // Calculate the exact collision point
        float penetration = (ball->x + ball->radius) - right_wall;
        ball->x = right_wall - ball->radius;
        
        // Only bounce if moving rightward
        if (ball->vx > 0) {
            ball->vx = -ball->vx * COEFFICIENT_OF_RESTITUTION;
        }
    }
    
    // Left wall collision
    if (ball->x - ball->radius < 0) {
        // Calculate the exact collision point
        float penetration = ball->radius - ball->x;
        ball->x = ball->radius;
        
        // Only bounce if moving leftward
        if (ball->vx < 0) {
            ball->vx = -ball->vx * COEFFICIENT_OF_RESTITUTION;
        }
    }
}

void render_ball(SDL_Renderer* renderer, const Ball* ball) {
    int x_center = (int)(ball->x * PIXELS_PER_METER);
    int y_center = (int)(ball->y * PIXELS_PER_METER);
    int radius = (int)(ball->radius * PIXELS_PER_METER);
    
    // Draw filled circle using the midpoint circle algorithm
    SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
    for (int w = 0; w < radius * 2; w++) {
        for (int h = 0; h < radius * 2; h++) {
            int dx = radius - w;
            int dy = radius - h;
            if ((dx*dx + dy*dy) <= (radius * radius)) {
                SDL_RenderDrawPoint(renderer, 
                    x_center + dx - radius, 
                    y_center + dy - radius);
            }
        }
    }
}

int main(const int argc, char *argv[]) {
    (void)argc;
    (void)argv;
    
    SDL_Init(SDL_INIT_VIDEO);
    SDL_Window *window = SDL_CreateWindow(
        "JPS - JoJo's Physics Simulator",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        WINDOW_WIDTH,
        WINDOW_HEIGHT,
        SDL_WINDOW_SHOWN
    );
    
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    
    // Initialize ball with realistic values
    Ball ball = {
        .x = WINDOW_WIDTH / (2.0f * PIXELS_PER_METER),  // Center position in meters
        .y = 1.0f,                                      // 1 meter from top
        .vx = 0.0f,                                     // Start at rest
        .vy = 0.0f,
        .ax = 0.0f,
        .ay = 0.0f,
        .mass = BALL_MASS,
        .radius = BALL_RADIUS
    };

    SDL_bool running = SDL_TRUE;
    SDL_Event event;
    
    Uint32 previous_time = SDL_GetTicks();
    float delta_time = 0.0f;
    float accumulator = 0.0f;
    const float fixed_time_step = 1.0f / TARGET_FPS;  // For physics updates

    while (running) {
        // Calculate delta time
        Uint32 current_time = SDL_GetTicks();
        delta_time = (current_time - previous_time) / 1000.0f;  // Convert to seconds
        previous_time = current_time;

        // Prevent spiral of death
        if (delta_time > 0.25f) {
            delta_time = 0.25f;
        }

        accumulator += delta_time;

        // Handle events
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = SDL_FALSE;
            }
            if (event.type == SDL_MOUSEBUTTONDOWN) {
                int mouseX, mouseY;
                SDL_GetMouseState(&mouseX, &mouseY);
                float dx = mouseX - (ball.x * PIXELS_PER_METER);
                float dy = mouseY - (ball.y * PIXELS_PER_METER);
                float distance = sqrt(dx*dx + dy*dy);
                
                float launch_speed = fmin(distance / PIXELS_PER_METER * 2.0f, 10.0f);
                ball.vx = (dx / distance) * launch_speed;
                ball.vy = (dy / distance) * launch_speed;
            }
        }

        // Update physics with fixed time step
        while (accumulator >= fixed_time_step) {
            update_physics(&ball);
            accumulator -= fixed_time_step;
        }
        
        // Clear screen
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_RenderClear(renderer);
        
        // Render ball
        render_ball(renderer, &ball);
        
        // Present render
        SDL_RenderPresent(renderer);

        // Frame rate control
        Uint32 frame_time = SDL_GetTicks() - current_time;
        if (frame_time < TARGET_FRAME_TIME) {
            SDL_Delay(TARGET_FRAME_TIME - frame_time);
        }
    }
    
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}