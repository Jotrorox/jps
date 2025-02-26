#ifndef BALL_HPP
#define BALL_HPP

#include "object.hpp"
#include <sstream>

class Ball : public Object {
public:
    float radius;

    Ball(float x, float y, float vx, float vy, float radius)
        : Object(ObjectType::BALL, x, y, vx, vy), radius(radius)
    {}

    virtual void updatePhysics(float dt) override;
    virtual void render(SDL_Renderer* renderer) const override;
    virtual void renderVelocityInfo(SDL_Renderer* renderer, TTF_Font* font) const override;
    SDL_Rect getBoundingBox() const;
};

#endif // BALL_HPP