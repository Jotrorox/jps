#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <stdbool.h>
#include <math.h>
#include <stdlib.h>
#include <time.h>

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600
#define BALL_RADIUS 10
#define GRAVITY 9.81  // m/s^2
#define PIXELS_PER_METER 100  // This defines our scale
#define BOUNCE_DAMPING 0.8
#define MAX_BALLS 100

typedef struct {
    float x, y;  // position in pixels
    float vx, vy;  // velocity in m/s
    SDL_Color color;
} Ball;

Ball balls[MAX_BALLS];
int ballCount = 0;

SDL_Color getRandomColor() {
    SDL_Color color;
    color.r = rand() % 256;
    color.g = rand() % 256;
    color.b = rand() % 256;
    color.a = 255;
    return color;
}

void drawCircle(SDL_Renderer* renderer, int centerX, int centerY, int radius, SDL_Color color) {
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
    for (int x = centerX - radius; x <= centerX + radius; x++) {
        for (int y = centerY - radius; y <= centerY + radius; y++) {
            if ((pow(centerX - x, 2) + pow(centerY - y, 2)) <= pow(radius, 2)) {
                SDL_RenderDrawPoint(renderer, x, y);
            }
        }
    }
}

void updateBall(Ball* ball, float deltaTime) {
    // Apply gravity
    ball->vy += GRAVITY * deltaTime;

    // Update position
    ball->x += ball->vx * PIXELS_PER_METER * deltaTime;
    ball->y += ball->vy * PIXELS_PER_METER * deltaTime;

    // Bounce off walls
    if (ball->x - BALL_RADIUS < 0) {
        ball->x = BALL_RADIUS;
        ball->vx = -ball->vx * BOUNCE_DAMPING;
    } else if (ball->x + BALL_RADIUS > WINDOW_WIDTH) {
        ball->x = WINDOW_WIDTH - BALL_RADIUS;
        ball->vx = -ball->vx * BOUNCE_DAMPING;
    }

    // Bounce off floor and ceiling
    if (ball->y - BALL_RADIUS < 0) {
        ball->y = BALL_RADIUS;
        ball->vy = -ball->vy * BOUNCE_DAMPING;
    } else if (ball->y + BALL_RADIUS > WINDOW_HEIGHT) {
        ball->y = WINDOW_HEIGHT - BALL_RADIUS;
        ball->vy = -ball->vy * BOUNCE_DAMPING;
    }
}

void addBall(int x, int y) {
    if (ballCount < MAX_BALLS) {
        Ball newBall = {
            .x = x,
            .y = y,
            .vx = (rand() % 200 - 100) / 100.0f,  // Random velocity between -1 and 1 m/s
            .vy = 0,
            .color = getRandomColor()
        };
        balls[ballCount++] = newBall;
    }
}

int main(int argc, char* argv[]) {
    (void)argc;
    (void)argv;

    SDL_Window* window = NULL;
    SDL_Renderer* renderer = NULL;
    TTF_Font* font = NULL;

    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        SDL_Log("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
        return 1;
    }

    if (TTF_Init() == -1) {
        SDL_Log("TTF could not initialize! TTF_Error: %s\n", TTF_GetError());
        return 1;
    }

    window = SDL_CreateWindow("Interactive Multi-Ball Physics Simulation", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_SHOWN);
    if (window == NULL) {
        SDL_Log("Window could not be created! SDL_Error: %s\n", SDL_GetError());
        return 1;
    }

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (renderer == NULL) {
        SDL_Log("Renderer could not be created! SDL_Error: %s\n", SDL_GetError());
        return 1;
    }

    font = TTF_OpenFont("rsc/Ubuntu-Regular.ttf", 24);
    if (font == NULL) {
        SDL_Log("Failed to load font! TTF_Error: %s\n", TTF_GetError());
        return 1;
    }

    srand(time(NULL));  // Initialize random seed

    bool quit = false;
    SDL_Event e;

    Uint64 NOW = SDL_GetPerformanceCounter();
    Uint64 LAST = 0;
    double deltaTime = 0;
    int frameCount = 0;
    double fps = 0;

    while (!quit) {
        LAST = NOW;
        NOW = SDL_GetPerformanceCounter();
        deltaTime = (double)((NOW - LAST) / (double)SDL_GetPerformanceFrequency());
        frameCount++;

        if (frameCount >= 60) {
            fps = 1.0 / deltaTime;
            frameCount = 0;
        }

        while (SDL_PollEvent(&e) != 0) {
            if (e.type == SDL_QUIT) {
                quit = true;
            } else if (e.type == SDL_MOUSEBUTTONDOWN) {
                int mouseX, mouseY;
                SDL_GetMouseState(&mouseX, &mouseY);
                addBall(mouseX, mouseY);
            }
        }

        for (int i = 0; i < ballCount; i++) {
            updateBall(&balls[i], deltaTime);
        }

        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_RenderClear(renderer);

        for (int i = 0; i < ballCount; i++) {
            drawCircle(renderer, (int)balls[i].x, (int)balls[i].y, BALL_RADIUS, balls[i].color);
        }

        // Render FPS
        char fpsText[20];
        snprintf(fpsText, sizeof(fpsText), "FPS: %.0f", fps);
        SDL_Surface* surfaceMessage = TTF_RenderText_Solid(font, fpsText, (SDL_Color){0, 0, 0, 255});
        SDL_Texture* message = SDL_CreateTextureFromSurface(renderer, surfaceMessage);
        SDL_Rect messageRect = {WINDOW_WIDTH - 125, 10, surfaceMessage->w, surfaceMessage->h};
        SDL_RenderCopy(renderer, message, NULL, &messageRect);
        SDL_FreeSurface(surfaceMessage);
        SDL_DestroyTexture(message);

        SDL_RenderPresent(renderer);
    }

    TTF_CloseFont(font);
    TTF_Quit();
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}