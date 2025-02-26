#ifndef BOX_HPP
#define BOX_HPP

#include "object.hpp"

class Box : public Object {
public:
    float width, height;

    // Boxes are static so velocity is always 0.
    Box(float x, float y, float width, float height)
        : Object(ObjectType::BOX, x, y, 0.0f, 0.0f), width(width), height(height)
    {}

    // Boxes are static; no physics update.
    virtual void updatePhysics(float dt) override;
    virtual void render(SDL_Renderer* renderer) const override;
    SDL_Rect getBoundingBox() const;
};

#endif // BOX_HPP