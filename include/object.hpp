#ifndef OBJECT_HPP
#define OBJECT_HPP

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

enum class ObjectType {
    BALL,
    BOX
};

class Object {
public:
    ObjectType type;
    float x, y;   // Position
    float vx, vy; // Velocity

    Object(ObjectType t, float x, float y, float vx, float vy)
        : type(t), x(x), y(y), vx(vx), vy(vy)
    {}

    virtual ~Object() {}

    // Update the physics state for delta time dt.
    virtual void updatePhysics(float dt) = 0;
    // Render the object.
    virtual void render(SDL_Renderer* renderer) const = 0;
    // Optionally, render additional info (e.g. velocity text). Default does nothing.
    virtual void renderVelocityInfo(SDL_Renderer* renderer, TTF_Font* font) const {}

    // Add this method if not already present
    virtual SDL_Rect getBoundingBox() const = 0;
};

#endif // OBJECT_HPP