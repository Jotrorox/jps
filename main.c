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
#define AIR_RESISTANCE 0.1
#define FRICTION 0.1

typedef struct {
    float x, y;  // position in pixels
    float vx, vy;  // velocity in m/s
    float radius;  // radius in pixels
    float mass;  // mass in kg
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

    // Apply air resistance
    ball->vx *= (1 - AIR_RESISTANCE * deltaTime);
    ball->vy *= (1 - AIR_RESISTANCE * deltaTime);

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

        // Apply friction when on the ground
        ball->vx *= (1 - FRICTION * deltaTime);
    }
}

void addBall(int x, int y, float radius, float mass) {
    if (ballCount < MAX_BALLS) {
        Ball newBall = {
            .x = x,
            .y = y,
            .vx = (rand() % 200 - 100) / 100.0f,  // Random velocity between -1 and 1 m/s
            .vy = 0,
            .radius = radius,
            .mass = mass,
            .color = getRandomColor()
        };
        balls[ballCount++] = newBall;
    }
}

void handleBallCollisions() {
    for (int i = 0; i < ballCount; i++) {
        for (int j = i + 1; j < ballCount; j++) {
            Ball* ball1 = &balls[i];
            Ball* ball2 = &balls[j];

            float dx = ball2->x - ball1->x;
            float dy = ball2->y - ball1->y;
            float distance = sqrt(dx * dx + dy * dy);

            if (distance < ball1->radius + ball2->radius) {
                // Calculate normal and tangent vectors
                float nx = dx / distance;
                float ny = dy / distance;
                float tx = -ny;
                float ty = nx;

                // Decompose velocities into normal and tangential components
                float v1n = ball1->vx * nx + ball1->vy * ny;
                float v1t = ball1->vx * tx + ball1->vy * ty;
                float v2n = ball2->vx * nx + ball2->vy * ny;
                float v2t = ball2->vx * tx + ball2->vy * ty;

                // Calculate new normal velocities using conservation of momentum
                float v1n_after = (v1n * (ball1->mass - ball2->mass) + 2 * ball2->mass * v2n) / (ball1->mass + ball2->mass);
                float v2n_after = (v2n * (ball2->mass - ball1->mass) + 2 * ball1->mass * v1n) / (ball1->mass + ball2->mass);

                // Recalculate the velocities
                ball1->vx = v1n_after * nx + v1t * tx;
                ball1->vy = v1n_after * ny + v1t * ty;
                ball2->vx = v2n_after * nx + v2t * tx;
                ball2->vy = v2n_after * ny + v2t * ty;

                // Separate the balls to avoid overlap
                float overlap = ball1->radius + ball2->radius - distance;
                ball1->x -= overlap * nx / 2;
                ball1->y -= overlap * ny / 2;
                ball2->x += overlap * nx / 2;
                ball2->y += overlap * ny / 2;
            }
        }
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

    float nextBallRadius = 10.0f;  // Default radius
    float nextBallMass = 1.0f;     // Default mass

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
                addBall(mouseX, mouseY, nextBallRadius, nextBallMass);
            } else if (e.type == SDL_KEYDOWN) {
                switch (e.key.keysym.sym) {
                    case SDLK_UP:
                        nextBallRadius += 1.0f;
                        break;
                    case SDLK_DOWN:
                        nextBallRadius -= 1.0f;
                        if (nextBallRadius < 1.0f) nextBallRadius = 1.0f;
                        break;
                    case SDLK_RIGHT:
                        nextBallMass += 1.0f;
                        break;
                    case SDLK_LEFT:
                        nextBallMass -= 1.0f;
                        if (nextBallMass < 1.0f) nextBallMass = 1.0f;
                        break;
                }
            }
        }

        for (int i = 0; i < ballCount; i++) {
            updateBall(&balls[i], deltaTime);
        }

        handleBallCollisions();

        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_RenderClear(renderer);

        for (int i = 0; i < ballCount; i++) {
            drawCircle(renderer, (int)balls[i].x, (int)balls[i].y, (int)balls[i].radius, balls[i].color);
        }

        // Render instructions
        SDL_Surface* surfaceInstructions = TTF_RenderText_Solid(font, "Click to add a ball", (SDL_Color){0, 0, 0, 255});
        SDL_Texture* instructions = SDL_CreateTextureFromSurface(renderer, surfaceInstructions);
        SDL_Rect instructionsRect = {10, 10, surfaceInstructions->w, surfaceInstructions->h};
        SDL_RenderCopy(renderer, instructions, NULL, &instructionsRect);
        SDL_FreeSurface(surfaceInstructions);
        SDL_DestroyTexture(instructions);

        // Render ball properties
        char ballPropertiesText[50];
        snprintf(ballPropertiesText, sizeof(ballPropertiesText), "Radius: %.0f Mass: %.0f", nextBallRadius, nextBallMass);
        SDL_Surface* surfaceBallProperties = TTF_RenderText_Solid(font, ballPropertiesText, (SDL_Color){0, 0, 0, 255});
        SDL_Texture* ballProperties = SDL_CreateTextureFromSurface(renderer, surfaceBallProperties);
        SDL_Rect ballPropertiesRect = {10, 40, surfaceBallProperties->w, surfaceBallProperties->h};
        SDL_RenderCopy(renderer, ballProperties, NULL, &ballPropertiesRect);
        SDL_FreeSurface(surfaceBallProperties);
        SDL_DestroyTexture(ballProperties);

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