#include <stdlib.h>
#include <string.h>
#include "config.h"
#include "ball.h"
#include "physics.h"
#include "render.h"
#include "menu.h"

// Global variables implementation
SDL_bool gravity = SDL_TRUE;
int target_fps = 60;
float frame_delay = 1000.0f / 60.0f;
Uint32 fps_last_time = 0;
int fps_counter = 0;
int current_fps = 0;

int main(int argc, char *argv[]) {
    (void)argc;
    (void)argv;
    
    if (SDL_Init(SDL_INIT_VIDEO) < 0 || TTF_Init() < 0) {
        printf("Initialization failed: %s\n", SDL_GetError());
        return 1;
    }

    TTF_Font* regular_font = TTF_OpenFont("/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf", FONT_SIZE);
    TTF_Font* title_font = TTF_OpenFont("/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf", MENU_TITLE_SIZE);

    if (!regular_font || !title_font) {
        regular_font = TTF_OpenFont("rsc/Ubuntu-Regular.ttf", FONT_SIZE);
        title_font = TTF_OpenFont("rsc/Ubuntu-Regular.ttf", MENU_TITLE_SIZE);
        
        if (!regular_font || !title_font) {
            printf("Failed to load fonts: %s\n", TTF_GetError());
            return 1;
        }
    }

    SDL_Window* window = SDL_CreateWindow(
        "Physics Simulator",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        WINDOW_WIDTH,
        WINDOW_HEIGHT,
        SDL_WINDOW_SHOWN
    );
    
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    // Initialize balls array and create first ball
    Ball* balls = calloc(MAX_BALLS, sizeof(Ball));
    int num_balls = 0;
    
    // Create initial ball
    balls[num_balls] = create_default_ball();
    num_balls++;

    SDL_bool running = SDL_TRUE;
    SDL_bool menu_open = SDL_FALSE;
    SDL_Event event;
    
    SDL_Rect menu_button = {
        WINDOW_WIDTH - MENU_BUTTON_SIZE - 10,
        10,
        MENU_BUTTON_SIZE,
        MENU_BUTTON_SIZE
    };
    
    SDL_Rect menu_panel = {
        WINDOW_WIDTH - MENU_PANEL_WIDTH - 10,
        MENU_BUTTON_SIZE + 20,
        MENU_PANEL_WIDTH,
        MENU_PANEL_HEIGHT
    };

    Uint32 previous_time = SDL_GetTicks();
    float delta_time = 0.0f;
    float fps_update_timer = 0.0f;

    while (running) {
        Uint32 frame_start = SDL_GetTicks();
        delta_time = (frame_start - previous_time) / 1000.0f;
        previous_time = frame_start;

        if (delta_time > 0.25f) delta_time = 0.25f;

        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT || 
                (event.type == SDL_KEYDOWN && 
                 (event.key.keysym.sym == SDLK_ESCAPE || event.key.keysym.sym == SDLK_q))) {
                running = SDL_FALSE;
            }
            
            if (event.type == SDL_MOUSEBUTTONDOWN) {
                int mouseX, mouseY;
                SDL_GetMouseState(&mouseX, &mouseY);
                
                if (mouseX >= menu_button.x && mouseX <= menu_button.x + menu_button.w &&
                    mouseY >= menu_button.y && mouseY <= menu_button.y + menu_button.h) {
                    menu_open = !menu_open;
                }
                else if (menu_open && mouseX >= menu_panel.x && mouseX <= menu_panel.x + menu_panel.w) {
                    MenuAction action = handle_menu_click(mouseX, mouseY, menu_panel);
                    switch(action) {
                        case MENU_ACTION_RESET:
                            num_balls = 1;
                            balls[0] = create_default_ball();
                            break;
                        case MENU_ACTION_TOGGLE_GRAVITY:
                            gravity = !gravity;
                            break;
                        case MENU_ACTION_CLEAR:
                            num_balls = 0;
                            break;
                        case MENU_ACTION_TOGGLE_FPS:
                            target_fps = (target_fps == 60) ? 144 : 60;
                            frame_delay = 1000.0f / target_fps;
                            break;
                        default:
                            break;
                    }
                }
                else if (event.button.button == SDL_BUTTON_LEFT && num_balls > 0) {
                    float dx = mouseX - (balls[0].x * PIXELS_PER_METER);
                    float dy = mouseY - (balls[0].y * PIXELS_PER_METER);
                    float distance = sqrt(dx*dx + dy*dy);
                    
                    if (distance > 0) {
                        float launch_speed = fmin(distance / PIXELS_PER_METER * 2.0f, 10.0f);
                        balls[0].vx = (dx / distance) * launch_speed;
                        balls[0].vy = (dy / distance) * launch_speed;
                    }
                }
            }
            
            if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_s && num_balls < MAX_BALLS) {
                balls[num_balls] = create_default_ball();
                // Offset new ball slightly to prevent overlap
                balls[num_balls].x += (float)(num_balls % 3) * 0.3f;
                balls[num_balls].y -= (float)(num_balls / 3) * 0.3f;
                num_balls++;
            }
        }

        // Physics update
        if (num_balls > 0) {  // Only process physics if there are balls
            SDL_mutex* mutex = SDL_CreateMutex();
            SDL_Thread* threads[NUM_THREADS];
            ThreadData thread_data[NUM_THREADS];

            for (int i = 0; i < NUM_THREADS; i++) {
                thread_data[i].balls = balls;
                thread_data[i].start_index = i * (num_balls / NUM_THREADS);
                thread_data[i].end_index = (i == NUM_THREADS - 1) ? num_balls : (i + 1) * (num_balls / NUM_THREADS);
                thread_data[i].delta_time = delta_time;
                thread_data[i].mutex = mutex;
                
                threads[i] = SDL_CreateThread(physics_thread, "PhysicsThread", &thread_data[i]);
            }

            for (int i = 0; i < NUM_THREADS; i++) {
                SDL_WaitThread(threads[i], NULL);
            }

            SDL_DestroyMutex(mutex);
        }

        // Render
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_RenderClear(renderer);

        // Only render if there are balls
        for (int i = 0; i < num_balls; i++) {
            render_ball(renderer, &balls[i]);
        }
        
        render_fps(renderer, regular_font, current_fps);

        // Draw menu button
        SDL_SetRenderDrawColor(renderer, 200, 200, 200, 255);
        SDL_RenderFillRect(renderer, &menu_button);
        
        SDL_SetRenderDrawColor(renderer, 100, 100, 100, 255);
        for (int i = 0; i < 3; i++) {
            SDL_Rect menu_line = {
                menu_button.x + 5,
                menu_button.y + MENU_BUTTON_SIZE - 10 - (i * 8),
                MENU_BUTTON_SIZE - 10,
                3
            };
            SDL_RenderFillRect(renderer, &menu_line);
        }

        if (menu_open) {
            draw_menu(renderer, regular_font, title_font, menu_panel);
        }

        SDL_RenderPresent(renderer);

        // Frame limiting
        Uint32 frame_time = SDL_GetTicks() - frame_start;
        if (frame_delay > frame_time) {
            SDL_Delay(frame_delay - frame_time);
        }

        // FPS calculation
        fps_counter++;
        fps_update_timer += delta_time;
        if (fps_update_timer >= FPS_UPDATE_INTERVAL) {
            current_fps = fps_counter / fps_update_timer;
            fps_counter = 0;
            fps_update_timer = 0;
        }
    }
    
    free(balls);
    TTF_CloseFont(regular_font);
    TTF_CloseFont(title_font);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    TTF_Quit();
    SDL_Quit();
    
    return 0;
} 