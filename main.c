#include <SDL2/SDL.h>
#include <stdbool.h>
#include <math.h>
#include <SDL_stdinc.h>
#define _USE_MATH_DEFINES
#include <float.h>
#include <SDL2/SDL_ttf.h>
#include <stdio.h>
#include <string.h>  // For strcmp
#include <stdlib.h>  // For atoi

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#define WINDOW_WIDTH 640
#define WINDOW_HEIGHT 480
#define BALL_SIZE 15
#define PIXELS_PER_METER 100.0f  // Conversion factor: 100 pixels = 1 meter

#define MAX_BALLS 1024

// Physics constants (in real-world units)
#define GRAVITY_ACCELERATION 9.81f  // m/s²
#define TIME_STEP 0.016f           // 16ms in seconds (for 60 FPS)
#define AIR_DENSITY 1.225f         // kg/m³
#define BALL_MASS 0.1f             // kg (100g
#define BALL_RADIUS (BALL_SIZE / (2.0f * PIXELS_PER_METER))  // Convert to meters
#define DRAG_COEFFICIENT 0.47f     // Sphere drag coefficient
#define COEFFICIENT_OF_RESTITUTION 0.75f  // Slightly more elastic
#define COEFFICIENT_OF_FRICTION 0.15f     // Slightly less friction

#define FPS_UPDATE_INTERVAL 0.5f  // Update FPS display every 0.5 seconds

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

float distance_between_balls(Ball* b1, Ball* b2) {
    float dx = (b1->x - b2->x);
    float dy = (b1->y - b2->y);
    return sqrtf(dx * dx + dy * dy);
}

void resolve_collision(Ball* b1, Ball* b2) {
    // Calculate normal vector
    float nx = (b2->x - b1->x);
    float ny = (b2->y - b1->y);
    float dist = sqrtf(nx * nx + ny * ny);
    nx /= dist;
    ny /= dist;

    // Relative velocity
    float rvx = b2->vx - b1->vx;
    float rvy = b2->vy - b1->vy;

    // Relative velocity along normal
    float velAlongNormal = rvx * nx + rvy * ny;

    // Don't resolve if objects are moving apart
    if (velAlongNormal > 0) return;

    // Restitution coefficient (bounciness)
    float e = 0.8f;

    // Calculate impulse scalar
    float j = -(1.0f + e) * velAlongNormal;
    j /= (1.0f / b1->mass + 1.0f / b2->mass);

    // Apply impulse
    float impulse_x = j * nx;
    float impulse_y = j * ny;

    b1->vx -= impulse_x / b1->mass;
    b1->vy -= impulse_y / b1->mass;
    b2->vx += impulse_x / b2->mass;
    b2->vy += impulse_y / b2->mass;

    // Position correction to prevent sinking
    const float percent = 0.2f; // penetration percentage to correct
    const float slop = 0.01f;   // penetration allowance
    float penetration = (b1->radius + b2->radius) - dist;
    if (penetration > slop) {
        float correction = (penetration * percent) / (1.0f / b1->mass + 1.0f / b2->mass);
        b1->x -= (correction / b1->mass) * nx;
        b1->y -= (correction / b1->mass) * ny;
        b2->x += (correction / b2->mass) * nx;
        b2->y += (correction / b2->mass) * ny;
    }
}

void prevent_ball_overlap(Ball* b1, Ball* b2) {
    float dx = b2->x - b1->x;
    float dy = b2->y - b1->y;
    float distance = sqrtf(dx*dx + dy*dy);
    
    // If balls are overlapping
    if (distance < (BALL_SIZE/PIXELS_PER_METER)) {
        // Calculate minimum separation distance
        float overlap = (BALL_SIZE/PIXELS_PER_METER) - distance;
        
        // Normalize direction vector
        if (distance > 0) {
            dx /= distance;
            dy /= distance;
        } else {
            // If balls are at exactly same position, push right
            dx = 1.0f;
            dy = 0.0f;
        }
        
        // Move balls apart equally
        float move_distance = overlap * 0.5f;
        b1->x -= dx * move_distance;
        b1->y -= dy * move_distance;
        b2->x += dx * move_distance;
    }
}

void update_physics(Ball* ball, float delta_time) {
    // Calculate drag forces
    float drag_force_x, drag_force_y;
    calculate_drag_force(ball, &drag_force_x, &drag_force_y);
    
    // Calculate accelerations (F = ma)
    ball->ax = drag_force_x / ball->mass;
    ball->ay = GRAVITY_ACCELERATION + (drag_force_y / ball->mass);
    
    // Update velocities using delta_time
    ball->vx += ball->ax * delta_time;
    ball->vy += ball->ay * delta_time;
    
    // Store previous position for collision detection
    float prev_x = ball->x; (void)prev_x; // Unused
    float prev_y = ball->y; (void)prev_y; // Unused
    
    // Update positions using delta_time
    ball->x += ball->vx * delta_time + 0.5f * ball->ax * delta_time * delta_time;
    ball->y += ball->vy * delta_time + 0.5f * ball->ay * delta_time * delta_time;
    
    // Handle collisions
    float max_x = (WINDOW_WIDTH - BALL_SIZE) / PIXELS_PER_METER;
    float max_y = (WINDOW_HEIGHT - BALL_SIZE) / PIXELS_PER_METER;
    
    // Floor collision
    if (ball->y > max_y) {
        ball->y = max_y;
        
        // Only bounce if moving downward
        if (ball->vy > 0) {
            ball->vy = -ball->vy * COEFFICIENT_OF_RESTITUTION;
        }
        
        // Apply ground friction to horizontal velocity
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
    
    // Right wall collision
    if (ball->x > max_x) {
        ball->x = max_x;
        
        // Only bounce if moving rightward
        if (ball->vx > 0) {
            ball->vx = -ball->vx * COEFFICIENT_OF_RESTITUTION;
        }
    }
    
    // Left wall collision
    if (ball->x < 0) {
        ball->x = 0;
        
        // Only bounce if moving leftward
        if (ball->vx < 0) {
            ball->vx = -ball->vx * COEFFICIENT_OF_RESTITUTION;
        }
    }
}

void render_ball(SDL_Renderer* renderer, const Ball* ball) {
    int x = (int)(ball->x * PIXELS_PER_METER);
    int y = (int)(ball->y * PIXELS_PER_METER);
    int size = BALL_SIZE;
    
    // Draw filled circle using the midpoint circle algorithm
    SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
    for (int w = 0; w < size; w++) {
        for (int h = 0; h < size; h++) {
            int dx = size/2 - w;
            int dy = size/2 - h;
            if ((dx*dx + dy*dy) <= ((size/2) * (size/2))) {
                SDL_RenderDrawPoint(renderer, x + w, y + h);
            }
        }
    }
}

void render_text(SDL_Renderer* renderer, TTF_Font* font, const char* text, int x, int y) {
    SDL_Color color = {0, 0, 0, 255};  // Black color
    SDL_Surface* surface = TTF_RenderText_Solid(font, text, color);
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    
    SDL_Rect dest = {x, y, surface->w, surface->h};
    SDL_RenderCopy(renderer, texture, NULL, &dest);
    
    SDL_FreeSurface(surface);
    SDL_DestroyTexture(texture);
}

int main(const int argc, char *argv[]) {
    (void)argc;
    (void)argv;
    
    SDL_Init(SDL_INIT_VIDEO);
    TTF_Init();  // Initialize SDL_ttf
    
    // Load font
    TTF_Font* font = TTF_OpenFont("rsc/Ubuntu-Regular.ttf", 24);
    if (!font) {
        printf("Failed to load font: %s\n", TTF_GetError());
        return 1;
    }
    
    SDL_Window *window = SDL_CreateWindow(
        "JPS - JoJo's Physics Simulator",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        WINDOW_WIDTH,
        WINDOW_HEIGHT,
        SDL_WINDOW_SHOWN
    );
    
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    Ball balls[MAX_BALLS];
    int num_balls = 0;

    // Initialize ball with realistic values
    Ball test_ball = {
        .x = WINDOW_WIDTH / (2.0f * PIXELS_PER_METER),  // Center position in meters
        .y = 1.0f,                                      // 1 meter from top
        .vx = 0.0f,                                     // Start at rest
        .vy = 0.0f,
        .ax = 0.0f,
        .ay = 0.0f,
        .mass = BALL_MASS,
        .radius = BALL_RADIUS
    };

    balls[num_balls++] = test_ball;

    SDL_bool running = SDL_TRUE;
    SDL_Event event;
    
    Uint32 previous_time = SDL_GetTicks();
    float delta_time = 0.0f;

    // Enable VSync if you want to prevent screen tearing
    SDL_RenderSetVSync(renderer, 0);  // 0 to disable VSync, 1 to enable

    // Add FPS tracking variables
    float fps_update_timer = 0.0f;
    int frame_count = 0;
    float current_fps = 0.0f;
    char fps_text[32];

    // Add FPS limit handling near the start of main
    int max_fps = 60;  // Default value
    float frame_time_target = 1000.0f / max_fps;  // Default target time

    // Parse command line arguments
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--max-fps") == 0 && i + 1 < argc) {
            max_fps = atoi(argv[i + 1]);
            if (max_fps < 0) max_fps = 0;  // Sanitize input
            if (max_fps > 0) {
                frame_time_target = 1000.0f / max_fps;
            }
            i++;  // Skip the next argument since we used it
        }
    }

    while (running) {
        // Calculate frame start time
        Uint32 frame_start = SDL_GetTicks();

        // Calculate delta time
        Uint32 current_time = SDL_GetTicks();
        delta_time = (current_time - previous_time) / 1000.0f;  // Convert to seconds
        previous_time = current_time;

        // Prevent spiral of death
        if (delta_time > 0.25f) {
            delta_time = 0.25f;
        }

        // Handle events
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = SDL_FALSE;
            }
            if (event.type == SDL_MOUSEBUTTONDOWN) {
                int mouseX, mouseY;
                SDL_GetMouseState(&mouseX, &mouseY);
                float dx = mouseX - (balls[0].x * PIXELS_PER_METER);
                float dy = mouseY - (balls[0].y * PIXELS_PER_METER);
                float distance = sqrt(dx*dx + dy*dy);
                
                float launch_speed = fmin(distance / PIXELS_PER_METER * 2.0f, 10.0f);
                balls[0].vx = (dx / distance) * launch_speed;
                balls[0].vy = (dy / distance) * launch_speed;
            }
            if (event.type == SDL_KEYDOWN && (event.key.keysym.sym == SDLK_ESCAPE || event.key.keysym.sym == SDLK_q)) {
                running = SDL_FALSE;
            }
            if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_s) {
                if (num_balls < MAX_BALLS) {
                    balls[num_balls++] = test_ball;
                } else {
                    printf("Max balls reached!\n");
                }
            }
        }

        // Setup collision detection
        for (int i = 0; i < num_balls; i++) {
            for (int j = i + 1; j < num_balls; j++) {
                prevent_ball_overlap(&balls[i], &balls[j]);
                if (distance_between_balls(&balls[i], &balls[j]) <= 
                    (balls[i].radius + balls[j].radius)) {
                    resolve_collision(&balls[i], &balls[j]);
                }
            }
        }

        // Update physics with delta time
        for (int i = 0; i < num_balls; i++) {
            update_physics(&balls[i], delta_time);
        }
        
        // Update FPS counter
        frame_count++;
        fps_update_timer += delta_time;
        
        if (fps_update_timer >= FPS_UPDATE_INTERVAL) {
            current_fps = frame_count / fps_update_timer;
            snprintf(fps_text, sizeof(fps_text), "FPS: %.0f", current_fps);
            frame_count = 0;
            fps_update_timer = 0.0f;
        }

        // Render
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_RenderClear(renderer);

        for (int i = 0; i < num_balls; i++) {
            render_ball(renderer, &balls[i]);
        }
        
        // Render FPS counter
        render_text(renderer, font, fps_text, 10, 10);
        
        SDL_RenderPresent(renderer);

        // Modified frame limiting code
        if (max_fps > 0) {  // Only limit frames if max_fps is not 0
            Uint32 frame_time = SDL_GetTicks() - frame_start;
            if (frame_time < frame_time_target) {
                SDL_Delay(frame_time_target - frame_time);
            }
        }
    }
    
    // Add cleanup
    TTF_CloseFont(font);
    TTF_Quit();
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}