#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include <float.h>
#include <stdio.h>
#include <string.h>

#include <SDL2/SDL.h>
#include <SDL_stdinc.h>
#include <SDL2/SDL_ttf.h>

#define _USE_MATH_DEFINES

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#define WINDOW_WIDTH 1280
#define WINDOW_HEIGHT 720
#define BALL_SIZE 15
#define PIXELS_PER_METER 100.0f  // Conversion factor: 100 pixels = 1 meter

#define MAX_BALLS 4096

#define GRAVITY_ACCELERATION 9.81f  // m/s²
#define TIME_STEP 0.016f           // 16ms in seconds (for 60 FPS)
#define AIR_DENSITY 1.225f         // kg/m³
#define BALL_MASS 0.1f             // kg (100g
#define BALL_RADIUS (BALL_SIZE / (2.0f * PIXELS_PER_METER))  // Convert to meters
#define DRAG_COEFFICIENT 0.47f     // Sphere drag coefficient
#define COEFFICIENT_OF_RESTITUTION 0.75f  // Slightly more elastic
#define COEFFICIENT_OF_FRICTION 0.15f     // Slightly less friction

#define FPS_UPDATE_INTERVAL 0.5f  // Update FPS display every 0.5 seconds

#define MENU_BUTTON_SIZE 40
#define MENU_PANEL_WIDTH 250
#define MENU_PANEL_HEIGHT 350
#define MENU_OPTION_HEIGHT 50

#define FONT_SIZE 18
#define MENU_TITLE_SIZE 24
#define TEXT_PADDING 15

SDL_bool gravity = SDL_TRUE;
int target_fps = 60;
float frame_delay = 1000.0f / 60.0f;  // Default to 60 FPS
Uint32 fps_last_time = 0;
int fps_counter = 0;
int current_fps = 0;

typedef struct {
    float x, y;        // position in meters
    float vx, vy;      // velocity in m/s
    float ax, ay;      // acceleration in m/s²
    float mass;        // mass in kg
    float radius;      // radius in meters
    SDL_Color color;   // color
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
                SDL_SetRenderDrawColor(renderer, ball->color.r, ball->color.g, ball->color.b, ball->color.a);
                SDL_RenderDrawPoint(renderer, x + w, y + h);
            }
        }
    }
}

void render_text(SDL_Renderer* renderer, TTF_Font* font, const char* text, int x, int y, SDL_Color color) {
    SDL_Surface* surface = TTF_RenderText_Blended(font, text, color);
    if (!surface) {
        return;
    }
    
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_Rect dest = {x, y, surface->w, surface->h};
    
    SDL_RenderCopy(renderer, texture, NULL, &dest);
    
    SDL_FreeSurface(surface);
    SDL_DestroyTexture(texture);
}

void draw_menu(SDL_Renderer* renderer, TTF_Font* regular_font, TTF_Font* title_font, SDL_Rect menu_panel) {
    // Colors for a more modern look
    SDL_Color text_color = {40, 40, 40, 255};
    SDL_Color title_color = {20, 20, 20, 255};
    SDL_Color hover_color = {230, 230, 230, 255};
    
    // Draw panel background with subtle shadow
    SDL_SetRenderDrawColor(renderer, 200, 200, 200, 180);
    SDL_Rect shadow = {menu_panel.x + 3, menu_panel.y + 3, menu_panel.w, menu_panel.h};
    SDL_RenderFillRect(renderer, &shadow);
    
    // Main panel with slightly rounded corners effect
    SDL_SetRenderDrawColor(renderer, 248, 248, 248, 255);
    SDL_RenderFillRect(renderer, &menu_panel);
    
    // Add a subtle border
    SDL_SetRenderDrawColor(renderer, 220, 220, 220, 255);
    SDL_RenderDrawRect(renderer, &menu_panel);
    
    // Draw menu title with padding
    render_text(renderer, title_font, "Settings", 
                menu_panel.x + menu_panel.w/2 - 50, // Center title
                menu_panel.y + TEXT_PADDING, 
                title_color);
    
    // Draw separator line below title
    SDL_SetRenderDrawColor(renderer, 220, 220, 220, 255);
    SDL_RenderDrawLine(renderer, 
                      menu_panel.x + TEXT_PADDING,
                      menu_panel.y + MENU_TITLE_SIZE + TEXT_PADDING * 2,
                      menu_panel.x + menu_panel.w - TEXT_PADDING,
                      menu_panel.y + MENU_TITLE_SIZE + TEXT_PADDING * 2);

    // Draw menu options with hover effect
    const char* options[] = {
        "Reset Balls",
        "Toggle Gravity",
        "Clear All",
        "Toggle FPS"
    };
    
    int mouseX, mouseY;
    SDL_GetMouseState(&mouseX, &mouseY);
    
    for (int i = 0; i < 4; i++) {
        SDL_Rect option_rect = {
            menu_panel.x + TEXT_PADDING/2,
            menu_panel.y + MENU_TITLE_SIZE + TEXT_PADDING * 2 + (i * MENU_OPTION_HEIGHT),
            menu_panel.w - TEXT_PADDING,
            MENU_OPTION_HEIGHT
        };
        
        // Check if mouse is hovering over option
        if (mouseX >= option_rect.x && mouseX <= option_rect.x + option_rect.w &&
            mouseY >= option_rect.y && mouseY <= option_rect.y + option_rect.h) {
            SDL_SetRenderDrawColor(renderer, hover_color.r, hover_color.g, hover_color.b, hover_color.a);
            SDL_RenderFillRect(renderer, &option_rect);
        }
        
        render_text(renderer, regular_font, options[i],
                   option_rect.x + TEXT_PADDING,
                   option_rect.y + (MENU_OPTION_HEIGHT - FONT_SIZE)/2,
                   text_color);
    }
}

void render_fps(SDL_Renderer* renderer, TTF_Font* font, int fps) {
    char fps_text[32];
    snprintf(fps_text, sizeof(fps_text), "FPS: %d", fps);
    
    SDL_Color fps_color = {0, 0, 0, 255};  // Black text
    
    SDL_Surface* surface = TTF_RenderText_Blended(font, fps_text, fps_color);
    if (!surface) {
        return;
    }
    
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_Rect dest = {10, 10, surface->w, surface->h};
    
    SDL_RenderCopy(renderer, texture, NULL, &dest);
    
    SDL_FreeSurface(surface);
    SDL_DestroyTexture(texture);
}

int main(const int argc, char *argv[]) {
    (void)argc;
    (void)argv;
    
    SDL_Init(SDL_INIT_VIDEO);
    TTF_Init();  // Initialize SDL_ttf
    
    if (TTF_Init() < 0) {
        printf("TTF initialization failed: %s\n", TTF_GetError());
        return 1;
    }

    TTF_Font* regular_font = TTF_OpenFont("/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf", FONT_SIZE);
    TTF_Font* title_font = TTF_OpenFont("/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf", MENU_TITLE_SIZE);

    if (!regular_font || !title_font) {
        // Try alternate font paths
        regular_font = TTF_OpenFont("rsc/Ubuntu-Regular.ttf", FONT_SIZE);
        title_font = TTF_OpenFont("rsc/Ubuntu-Regular.ttf", MENU_TITLE_SIZE);
        
        if (!regular_font || !title_font) {
            printf("Failed to load fonts: %s\n", TTF_GetError());
            return 1;
        }
    }

    // Enable font hinting for better rendering
    TTF_SetFontHinting(regular_font, TTF_HINTING_LIGHT);
    TTF_SetFontHinting(title_font, TTF_HINTING_LIGHT);

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
        .radius = BALL_RADIUS,
        .color = {255, 0, 0, 255}
    };
    
    // Initialize ball with realistic values
    Ball start_ball = {
        .x = WINDOW_WIDTH / (2.0f * PIXELS_PER_METER),  // Center position in meters
        .y = 1.0f,                                      // 1 meter from top
        .vx = 0.0f,                                     // Start at rest
        .vy = 0.0f,
        .ax = 0.0f,
        .ay = 0.0f,
        .mass = BALL_MASS,
        .radius = BALL_RADIUS,
        .color = {255, 0, 255, 255}
    };

    balls[num_balls++] = start_ball;

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
    int max_fps = 120;  // Default value
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

    SDL_bool menu_open = SDL_FALSE;
    SDL_Rect menu_button = {0, 0, MENU_BUTTON_SIZE, MENU_BUTTON_SIZE}; // Will position in init
    SDL_Rect menu_panel = {0, 0, MENU_PANEL_WIDTH, MENU_PANEL_HEIGHT}; // Will position in init

    menu_button.x = WINDOW_WIDTH - MENU_BUTTON_SIZE - 10;
    menu_button.y = 10;
    menu_panel.x = WINDOW_WIDTH - MENU_PANEL_WIDTH - 10;
    menu_panel.y = MENU_BUTTON_SIZE + 20;

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
                
                // Check if menu button was clicked
                if (mouseX >= menu_button.x && mouseX <= menu_button.x + menu_button.w &&
                    mouseY >= menu_button.y && mouseY <= menu_button.y + menu_button.h) {
                    menu_open = !menu_open;
                }
                // If menu is open, check for menu option clicks
                else if (menu_open && 
                         mouseX >= menu_panel.x && mouseX <= menu_panel.x + menu_panel.w) {
                    // Calculate which option was clicked based on Y position
                    int clickY = mouseY - (menu_panel.y + MENU_TITLE_SIZE + TEXT_PADDING * 2);
                    if (clickY >= 0) {
                        int option = clickY / MENU_OPTION_HEIGHT;
                        if (option >= 0 && option < 4) {  // Make sure click is within valid options
                            switch(option) {
                                case 0: // Reset all balls
                                    num_balls = 1;
                                    balls[0] = start_ball;  // Use start_ball instead of test_ball
                                    break;
                                case 1: // Toggle gravity
                                    gravity = !gravity;
                                    break;
                                case 2: // Clear all balls
                                    num_balls = 0;
                                    break;
                                case 3: // Toggle FPS (60/144)
                                    target_fps = (target_fps == 60) ? 144 : 60;
                                    frame_delay = 1000.0f / target_fps;
                                    break;
                            }
                        }
                    }
                }
                else {
                    // Existing ball launch code
                    float dx = mouseX - (balls[0].x * PIXELS_PER_METER);
                    float dy = mouseY - (balls[0].y * PIXELS_PER_METER);
                    float distance = sqrt(dx*dx + dy*dy);
                    
                    float launch_speed = fmin(distance / PIXELS_PER_METER * 2.0f, 10.0f);
                    balls[0].vx = (dx / distance) * launch_speed;
                    balls[0].vy = (dy / distance) * launch_speed;
                }
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
        render_fps(renderer, regular_font, current_fps);

        // Draw menu button
        SDL_SetRenderDrawColor(renderer, 200, 200, 200, 255);
        SDL_RenderFillRect(renderer, &menu_button);
        SDL_SetRenderDrawColor(renderer, 100, 100, 100, 255);
        SDL_RenderDrawLine(renderer, menu_button.x + 5, menu_button.y + 7, 
                          menu_button.x + 25, menu_button.y + 7);
        SDL_RenderDrawLine(renderer, menu_button.x + 5, menu_button.y + 15, 
                          menu_button.x + 25, menu_button.y + 15);
        SDL_RenderDrawLine(renderer, menu_button.x + 5, menu_button.y + 23, 
                          menu_button.x + 25, menu_button.y + 23);

        // Draw menu panel if open
        if (menu_open) {
            draw_menu(renderer, regular_font, title_font, menu_panel);
        }
        
        // Add FPS calculation before SDL_RenderPresent
        fps_counter++;
        if (SDL_GetTicks() - fps_last_time >= 1000) {
            current_fps = fps_counter;
            fps_counter = 0;
            fps_last_time = SDL_GetTicks();
        }

        SDL_RenderPresent(renderer);

        // Modified frame limiting code
        if (max_fps > 0) {  // Only limit frames if max_fps is not 0
            Uint32 frame_time = SDL_GetTicks() - frame_start;
            if (frame_time < frame_time_target) {
                SDL_Delay(frame_time_target - frame_time);
            }
        }

        // Add frame delay at end of main loop
        Uint32 frame_time = SDL_GetTicks() - frame_start;
        if (frame_delay > frame_time) {
            SDL_Delay(frame_delay - frame_time);
        }
    }
    
    // Add cleanup
    TTF_CloseFont(regular_font);
    TTF_CloseFont(title_font);
    TTF_CloseFont(font);
    TTF_Quit();
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}