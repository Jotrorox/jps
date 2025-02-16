#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <iostream>
#include <thread>
#include <mutex>
#include <sstream>
#include "ball.hpp"
#include "physics.hpp"
#include "render.hpp"
#include "font_data.hpp"  // Embedded font data

// Window dimensions
constexpr int WINDOW_WIDTH  = 800;
constexpr int WINDOW_HEIGHT = 600;

int main(int argc, char* argv[]) {
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        std::cerr << "SDL_Init Error: " << SDL_GetError() << "\n";
        return 1;
    }

    if (TTF_Init() != 0) {
        std::cerr << "TTF_Init Error: " << TTF_GetError() << "\n";
        SDL_Quit();
        return 1;
    }

    // Create the window and renderer.
    SDL_Window* window = SDL_CreateWindow("Physics Simulation with FPS & Embedded Font",
                                          SDL_WINDOWPOS_CENTERED,
                                          SDL_WINDOWPOS_CENTERED,
                                          WINDOW_WIDTH, WINDOW_HEIGHT,
                                          SDL_WINDOW_SHOWN);
    if (!window) {
         std::cerr << "SDL_CreateWindow Error: " << SDL_GetError() << "\n";
         TTF_Quit();
         SDL_Quit();
         return 1;
    }

    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
         std::cerr << "SDL_CreateRenderer Error: " << SDL_GetError() << "\n";
         SDL_DestroyWindow(window);
         TTF_Quit();
         SDL_Quit();
         return 1;
    }

    // Instead of loading the font from disk, create an SDL_RWops from the embedded font data.
    SDL_RWops *rw = SDL_RWFromConstMem(rsc_SNPro_Regular_ttf, rsc_SNPro_Regular_ttf_len);
    if (!rw) {
        std::cerr << "SDL_RWFromConstMem Error: " << SDL_GetError() << "\n";
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        TTF_Quit();
        SDL_Quit();
        return 1;
    }
    // Open the font from the RWops; the "1" flag tells SDL_ttf to free the RWops for us.
    TTF_Font* font = TTF_OpenFontRW(rw, 1, 24);
    if (!font) {
        std::cerr << "TTF_OpenFontRW Error: " << TTF_GetError() << "\n";
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        TTF_Quit();
        SDL_Quit();
        return 1;
    }

    // Initialize the ball in the center with an initial velocity.
    Ball ball;
    ball.x = WINDOW_WIDTH / 2.0f;
    ball.y = WINDOW_HEIGHT / 2.0f;
    ball.vx = 200.0f;  // horizontal velocity (pixels/s)
    ball.vy = -300.0f; // vertical velocity (pixels/s)
    ball.radius = 20.0f;

    bool simulationRunning = true;
    std::mutex ballMutex;

    // Start the physics simulation thread.
    std::thread physicsThread(physicsThreadFunction, std::ref(simulationRunning), std::ref(ball), std::ref(ballMutex));

    bool quit = false;
    SDL_Event event;
    Uint32 fpsTimer = SDL_GetTicks();
    int frames = 0;
    float currentFPS = 0.0f;

    while (!quit) {
        Uint32 frameStart = SDL_GetTicks();
        while(SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                quit = true;
            }
        }

        // Clear the screen.
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        // Render a ground line.
        SDL_SetRenderDrawColor(renderer, 150, 75, 0, 255);
        SDL_RenderDrawLine(renderer, 0, WINDOW_HEIGHT - 1, WINDOW_WIDTH, WINDOW_HEIGHT - 1);

        // Render the ball.
        {
            std::lock_guard<std::mutex> lock(ballMutex);
            SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
            renderBall(renderer, ball);
        }

        // Calculate and render FPS.
        frames++;
        Uint32 currentTicks = SDL_GetTicks();
        if (currentTicks - fpsTimer >= 1000) {
            currentFPS = frames * 1000.0f / (currentTicks - fpsTimer);
            fpsTimer = currentTicks;
            frames = 0;
        }
        std::stringstream fpsText;
        fpsText << "FPS: " << static_cast<int>(currentFPS);
        SDL_Color white = {255, 255, 255, 255};
        SDL_Surface* fpsSurface = TTF_RenderText_Solid(font, fpsText.str().c_str(), white);
        if (fpsSurface) {
            SDL_Texture* fpsTexture = SDL_CreateTextureFromSurface(renderer, fpsSurface);
            if (fpsTexture) {
                SDL_Rect dstRect = {10, 10, fpsSurface->w, fpsSurface->h};
                SDL_RenderCopy(renderer, fpsTexture, nullptr, &dstRect);
                SDL_DestroyTexture(fpsTexture);
            }
            SDL_FreeSurface(fpsSurface);
        }

        // Present the rendered frame.
        SDL_RenderPresent(renderer);

        // Frame delay to cap near 60 FPS using delta time.
        Uint32 frameTime = SDL_GetTicks() - frameStart;
        if (frameTime < 16) {
            SDL_Delay(16 - frameTime);
        }
    }

    // Clean up.
    simulationRunning = false;
    physicsThread.join();

    TTF_CloseFont(font);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    TTF_Quit();
    SDL_Quit();

    return 0;
}