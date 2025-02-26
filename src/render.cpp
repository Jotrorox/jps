#include "render.hpp"
#include "object.hpp"
#include <sstream>
#include <iomanip>

void renderObject(SDL_Renderer* renderer, const Object* obj) {
    if (obj)
        obj->render(renderer);
}

void renderDebugInfo(SDL_Renderer* renderer, TTF_Font* font, const Object* obj) {
    if (!obj || !font) return;

    // Draw collision box in red
    SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
    
    // Get object's bounding box
    SDL_Rect boundingBox = obj->getBoundingBox();
    SDL_RenderDrawRect(renderer, &boundingBox);
    
    // Format debug text with position and velocity
    std::stringstream debugText;
    debugText << std::fixed << std::setprecision(1);
    debugText << "Pos:(" << obj->x << "," << obj->y << ")";
    debugText << " Vel:(" << obj->vx << "," << obj->vy << ")";
    
    // Add type-specific information
    if (obj->type == ObjectType::BALL) {
        debugText << " Type:Ball";
    } else {
        debugText << " Type:Box";
    }
    
    // Render text with shadow for better visibility
    SDL_Color textColor = {255, 255, 0, 255}; // Yellow is more visible
    SDL_Surface* textSurface = TTF_RenderText_Solid(font, debugText.str().c_str(), textColor);
    if (textSurface) {
        SDL_Texture* textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
        if (textTexture) {
            // Position text above the object
            SDL_Rect textRect = {
                boundingBox.x,
                boundingBox.y - textSurface->h - 5,
                textSurface->w,
                textSurface->h
            };
            
            // Draw shadow first
            SDL_SetTextureColorMod(textTexture, 0, 0, 0);
            SDL_Rect shadowRect = {textRect.x + 1, textRect.y + 1, textRect.w, textRect.h};
            SDL_RenderCopy(renderer, textTexture, nullptr, &shadowRect);
            
            // Then draw text
            SDL_SetTextureColorMod(textTexture, 255, 255, 0);
            SDL_RenderCopy(renderer, textTexture, nullptr, &textRect);
            
            SDL_DestroyTexture(textTexture);
        }
        SDL_FreeSurface(textSurface);
    }
}