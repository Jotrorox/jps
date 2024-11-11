#include <SDL2/SDL.h>
#include <stdbool.h>
#include <math.h>
#include <SDL_stdinc.h>

#define WINDOW_WIDTH 640
#define WINDOW_HEIGHT 480
#define BALL_SIZE 20
#define PIXELS_PER_METER 100.0f  // Conversion factor: 100 pixels = 1 meter

// Physics constants (in real-world units)
#define GRAVITY_ACCELERATION 9.81f  // m/s²
#define TIME_STEP 0.016f           // 16ms in seconds (for 60 FPS)
#define AIR_DENSITY 1.225f         // kg/m³
#define BALL_MASS 0.1f             // kg (100g)
#define BALL_RADIUS (BALL_SIZE / (2.0f * PIXELS_PER_METER))  // Convert to meters
#define DRAG_COEFFICIENT 0.47f     // Sphere drag coefficient
#define COEFFICIENT_OF_RESTITUTION 0.7f  // Bounce elasticity
#define COEFFICIENT_OF_FRICTION 0.2f     // Ground friction

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
    
    // Update velocities (v = v₀ + at)
    ball->vx += ball->ax * TIME_STEP;
    ball->vy += ball->ay * TIME_STEP;
    
    // Update positions (x = x₀ + vt + ½at²)
    ball->x += ball->vx * TIME_STEP + 0.5f * ball->ax * TIME_STEP * TIME_STEP;
    ball->y += ball->vy * TIME_STEP + 0.5f * ball->ay * TIME_STEP * TIME_STEP;
    
    // Handle collisions
    float floor_height = WINDOW_HEIGHT / PIXELS_PER_METER;
    float right_wall = WINDOW_WIDTH / PIXELS_PER_METER;
    
    // Floor collision
    if (ball->y + ball->radius > floor_height) {
        ball->y = floor_height - ball->radius;
        ball->vy = -ball->vy * COEFFICIENT_OF_RESTITUTION;
        
        // Apply ground friction to horizontal velocity
        if (fabs(ball->vx) > 0.001f) {
            float normal_force = ball->mass * GRAVITY_ACCELERATION;
            float friction_force = COEFFICIENT_OF_FRICTION * normal_force;
            float friction_deceleration = friction_force / ball->mass;
            
            float friction_delta_v = friction_deceleration * TIME_STEP;
            if (fabs(ball->vx) <= friction_delta_v) {
                ball->vx = 0;
            } else {
                ball->vx -= (ball->vx > 0 ? friction_delta_v : -friction_delta_v);
            }
        }
    }
    
    // Wall collisions
    if (ball->x + ball->radius > right_wall) {
        ball->x = right_wall - ball->radius;
        ball->vx = -ball->vx * COEFFICIENT_OF_RESTITUTION;
    }
    if (ball->x - ball->radius < 0) {
        ball->x = ball->radius;
        ball->vx = -ball->vx * COEFFICIENT_OF_RESTITUTION;
    }
}

void render_ball(SDL_Renderer* renderer, const Ball* ball) {
    SDL_Rect rect = {
        (int)(ball->x * PIXELS_PER_METER - ball->radius * PIXELS_PER_METER),
        (int)(ball->y * PIXELS_PER_METER - ball->radius * PIXELS_PER_METER),
        (int)(ball->radius * 2 * PIXELS_PER_METER),
        (int)(ball->radius * 2 * PIXELS_PER_METER)
    };
    SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
    SDL_RenderFillRect(renderer, &rect);
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
        .vx = 2.0f,                                     // 2 m/s initial velocity
        .vy = 0.0f,
        .ax = 0.0f,
        .ay = 0.0f,
        .mass = BALL_MASS,
        .radius = BALL_RADIUS
    };

    SDL_bool running = SDL_TRUE;
    SDL_Event event;
    
    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = SDL_FALSE;
            }
            // Add mouse click to launch ball
            if (event.type == SDL_MOUSEBUTTONDOWN) {
                int mouseX, mouseY;
                SDL_GetMouseState(&mouseX, &mouseY);
                ball.x = mouseX / PIXELS_PER_METER;
                ball.y = mouseY / PIXELS_PER_METER;
                ball.vy = -5.0f;  // 5 m/s upward
                ball.vx = (mouseX > WINDOW_WIDTH/2) ? -3.0f : 3.0f;  // 3 m/s horizontal
            }
        }
        
        // Clear screen
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_RenderClear(renderer);
        
        // Update physics
        update_physics(&ball);
        
        // Render ball
        render_ball(renderer, &ball);
        
        // Present render
        SDL_RenderPresent(renderer);
        
        // Add small delay to control simulation speed
        SDL_Delay(16);  // Roughly 60 FPS
    }
    
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}