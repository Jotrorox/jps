#ifndef CONFIG_H
#define CONFIG_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

#define _USE_MATH_DEFINES

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// Window configuration
#define WINDOW_WIDTH 1280
#define WINDOW_HEIGHT 720

// Physics configuration
#define BALL_SIZE 15
#define PIXELS_PER_METER 100.0f
#define MAX_BALLS 4096
#define GRAVITY_ACCELERATION 9.81f
#define TIME_STEP 0.016f
#define AIR_DENSITY 1.225f
#define BALL_MASS 0.1f
#define BALL_RADIUS (BALL_SIZE / (2.0f * PIXELS_PER_METER))
#define DRAG_COEFFICIENT 0.47f
#define COEFFICIENT_OF_RESTITUTION 0.75f
#define COEFFICIENT_OF_FRICTION 0.15f

// Threading configuration
#define NUM_THREADS 4
#define BALLS_PER_THREAD (MAX_BALLS / NUM_THREADS)

// UI configuration
#define FPS_UPDATE_INTERVAL 0.5f
#define MENU_BUTTON_SIZE 40
#define MENU_PANEL_WIDTH 250
#define MENU_PANEL_HEIGHT 350
#define MENU_OPTION_HEIGHT 50
#define FONT_SIZE 18
#define MENU_TITLE_SIZE 24
#define TEXT_PADDING 15

// Global variables (consider moving to a proper state management system)
extern SDL_bool gravity;
extern int target_fps;
extern float frame_delay;
extern Uint32 fps_last_time;
extern int fps_counter;
extern int current_fps;

#endif // CONFIG_H 