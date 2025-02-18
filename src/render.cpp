#include "render.hpp"
#include "object.hpp"

void renderObject(SDL_Renderer* renderer, const Object* obj) {
    if (obj)
        obj->render(renderer);
}