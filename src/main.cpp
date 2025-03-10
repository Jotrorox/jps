#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL2_gfxPrimitives.h>
#include <iostream>
#include <thread>
#include <mutex>
#include <sstream>
#include <vector>
#include <unordered_map>
#include "object.hpp"
#include "ball.hpp"
#include "box.hpp"
#include "physics.hpp"
#include "render.hpp"
#include "collision.hpp"
#include "font_data.hpp"

constexpr int WINDOW_WIDTH  = 800;
constexpr int WINDOW_HEIGHT = 600;

// At the global scope or in a manager class
std::unordered_map<std::string, SDL_Texture*> textCache;

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
    SDL_Window* window = SDL_CreateWindow("JPS",
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
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, 
        SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
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
    TTF_Font* font = TTF_OpenFontRW(rw, 1, 32);
    if (!font) {
        std::cerr << "TTF_OpenFont Error: " << TTF_GetError() << "\n";
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        TTF_Quit();
        SDL_Quit();
        return 1;
    }

    // Store objects as pointers to the base class.
    std::vector<Object*> objects;
    std::mutex objectsMutex;

    // Start the physics thread.
    bool simulationRunning = true;
    std::thread physicsThread(physicsThreadFunction, std::ref(simulationRunning),
                              std::ref(objects), std::ref(objectsMutex));

    bool quit = false;
    SDL_Event event;

    // Variables for spawning objects using mouse drag.
    bool dragging = false;
    int dragStartX = 0, dragStartY = 0;
    int currentDragX = 0, currentDragY = 0;
    constexpr float VELOCITY_MULTIPLIER = 3.0f;
    
    // Mode flag: if true, user is creating a box by dragging, else a ball.
    bool previewBoxMode = false;
    
    // Toggle for showing velocity info above Balls.
    bool showVelocityInfo = false;
    // Toggle for debug mode
    bool debugMode = false;

    Uint32 fpsTimer = SDL_GetTicks();
    int frames = 0;
    float currentFPS = 0.0f;

    while (!quit) {
        Uint32 frameStart = SDL_GetTicks();
        while(SDL_PollEvent(&event)) {
            switch(event.type) {
                case SDL_QUIT:
                    quit = true;
                    break;
                case SDL_KEYDOWN:
                    // Toggle velocity info with V key.
                    if (event.key.keysym.sym == SDLK_v)
                        showVelocityInfo = !showVelocityInfo;
                    // Toggle debug mode with D key
                    else if (event.key.keysym.sym == SDLK_d)
                        debugMode = !debugMode;
                    break;
                case SDL_MOUSEBUTTONDOWN:
                    if (event.button.button == SDL_BUTTON_LEFT) {
                        dragging = true;
                        dragStartX = event.button.x;
                        dragStartY = event.button.y;
                        currentDragX = event.button.x;
                        currentDragY = event.button.y;
                        // Check modifier key: if SHIFT is held at start then set box mode.
                        previewBoxMode = (SDL_GetModState() & KMOD_SHIFT) != 0;
                    }
                    break;
                case SDL_MOUSEMOTION:
                    if (dragging) {
                        currentDragX = event.motion.x;
                        currentDragY = event.motion.y;
                    }
                    break;
                case SDL_MOUSEBUTTONUP:
                    if (dragging && event.button.button == SDL_BUTTON_LEFT) {
                        dragging = false;
                        int dragEndX = event.button.x;
                        int dragEndY = event.button.y;
                        if (previewBoxMode) {
                            // For box creation, determine width and height from drag.
                            int width = std::abs(dragEndX - dragStartX);
                            int height = std::abs(dragEndY - dragStartY);
                            // Avoid creating zero-sized boxes.
                            if(width < 5) width = 5;
                            if(height < 5) height = 5;
                            // Set center to the midpoint.
                            float centerX = (dragStartX + dragEndX) / 2.0f;
                            float centerY = (dragStartY + dragEndY) / 2.0f;
                            // Create a new Box with specified size.
                            Object* newObj = new Box(centerX, centerY, static_cast<float>(width), static_cast<float>(height));
                            {
                                std::lock_guard<std::mutex> lock(objectsMutex);
                                objects.push_back(newObj);
                            }
                        } else {
                            // For ball creation, use drag vector to determine initial velocity.
                            float vx = (dragEndX - dragStartX) * VELOCITY_MULTIPLIER;
                            float vy = (dragEndY - dragStartY) * VELOCITY_MULTIPLIER;
                            Object* newObj = new Ball(static_cast<float>(dragStartX),
                                                      static_cast<float>(dragStartY),
                                                      vx, vy, 20.0f);
                            {
                                std::lock_guard<std::mutex> lock(objectsMutex);
                                objects.push_back(newObj);
                            }
                        }
                    }
                    break;
            }
        }
        // Clear the screen.
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);
        
        // Draw ground line.
        SDL_SetRenderDrawColor(renderer, 150, 75, 0, 255);
        SDL_RenderDrawLine(renderer, 0, WINDOW_HEIGHT - 1, WINDOW_WIDTH, WINDOW_HEIGHT - 1);
        
        // Draw preview if dragging in box creation mode.
        if (dragging && previewBoxMode) {
            // Compute rectangle: top-left and dimensions.
            int rectX = std::min(dragStartX, currentDragX);
            int rectY = std::min(dragStartY, currentDragY);
            int rectW = std::abs(currentDragX - dragStartX);
            int rectH = std::abs(currentDragY - dragStartY);
            SDL_Rect previewRect = { rectX, rectY, rectW, rectH };
            
            // Semi-transparent fill
            SDL_SetRenderDrawColor(renderer, 0, 255, 0, 100);
            SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
            SDL_RenderFillRect(renderer, &previewRect);
            
            // Solid border
            SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
            SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);
            SDL_RenderDrawRect(renderer, &previewRect);
        }
        // If dragging and not in box mode, draw the drag vector for ball creation.
        else if (dragging && !previewBoxMode) {
            SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
            // Use SDL2_gfx for anti-aliased lines
            aalineRGBA(renderer, dragStartX, dragStartY, currentDragX, currentDragY, 0, 255, 0, 255);
        }
        
        // Render all objects.
        {
            std::lock_guard<std::mutex> lock(objectsMutex);
            for (const auto obj : objects) {
                if (obj->type == ObjectType::BALL)
                    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
                else
                    SDL_SetRenderDrawColor(renderer, 180, 180, 180, 255);
                obj->render(renderer);
                
                // Show velocity info if enabled
                if (showVelocityInfo && obj->type == ObjectType::BALL)
                    obj->renderVelocityInfo(renderer, font);
                
                // Show debug info if debug mode is enabled
                if (debugMode)
                    renderDebugInfo(renderer, font, obj);
            }
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

        // FPS text rendering with proper texture caching
        std::string fpsTextStr = fpsText.str();
        SDL_Texture* fpsTexture = nullptr;
        int textWidth = 0, textHeight = 0;

        // Check if we already have this text cached
        if (textCache.find(fpsTextStr) != textCache.end()) {
            fpsTexture = textCache[fpsTextStr];
            // Query the texture to get its width and height
            SDL_QueryTexture(fpsTexture, nullptr, nullptr, &textWidth, &textHeight);
        } else {
            SDL_Surface* fpsSurface = TTF_RenderText_Blended(font, fpsTextStr.c_str(), white);
            if (fpsSurface) {
                textWidth = fpsSurface->w;
                textHeight = fpsSurface->h;
                fpsTexture = SDL_CreateTextureFromSurface(renderer, fpsSurface);
                textCache[fpsTextStr] = fpsTexture;
                SDL_FreeSurface(fpsSurface);
            }
        }

        if (fpsTexture) {
            // First draw shadow
            SDL_SetTextureColorMod(fpsTexture, 0, 0, 0);
            SDL_SetTextureAlphaMod(fpsTexture, 128);
            SDL_Rect shadowRect = {12, 12, textWidth, textHeight};
            SDL_RenderCopy(renderer, fpsTexture, nullptr, &shadowRect);
            
            // Then draw text
            SDL_SetTextureColorMod(fpsTexture, 255, 255, 255);
            SDL_SetTextureAlphaMod(fpsTexture, 255);
            SDL_Rect dstRect = {10, 10, textWidth, textHeight};
            SDL_RenderCopy(renderer, fpsTexture, nullptr, &dstRect);
        }
        SDL_RenderPresent(renderer);

        Uint32 frameTime = SDL_GetTicks() - frameStart;
        if (frameTime < 16)
            SDL_Delay(16 - frameTime);
    }
    simulationRunning = false;
    physicsThread.join();

    // Clean up dynamically allocated objects.
    {
        std::lock_guard<std::mutex> lock(objectsMutex);
        for (auto obj : objects)
            delete obj;
        objects.clear();
    }

    // Clean up texture cache
    for (auto& pair : textCache) {
        SDL_DestroyTexture(pair.second);
    }
    textCache.clear();

    TTF_CloseFont(font);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    TTF_Quit();
    SDL_Quit();
    return 0;
}