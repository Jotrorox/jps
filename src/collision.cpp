#include "collision.hpp"
#include "object.hpp"
#include "ball.hpp"
#include "box.hpp"
#include <cmath>
#include <algorithm>
#include <utility>

constexpr float BOUNCE_DAMPING = 0.7f;

// Helper: get the Axis-Aligned Bounding Box (AABB) for an object.
static void getAABB(const Object* obj, float &left, float &right, float &top, float &bottom) {
    if (obj->type == ObjectType::BALL) {
        const Ball* ball = static_cast<const Ball*>(obj);
        left   = ball->x - ball->radius;
        right  = ball->x + ball->radius;
        top    = ball->y - ball->radius;
        bottom = ball->y + ball->radius;
    } else { // BOX
        const Box* box = static_cast<const Box*>(obj);
        left   = box->x - box->width * 0.5f;
        right  = box->x + box->width * 0.5f;
        top    = box->y - box->height * 0.5f;
        bottom = box->y + box->height * 0.5f;
    }
}

void resolveCollision(Object* a, Object* b) {
    bool aStatic = (a->type == ObjectType::BOX);
    bool bStatic = (b->type == ObjectType::BOX);
    
    // If both are static, do nothing.
    if (aStatic && bStatic)
        return;
    
    float leftA, rightA, topA, bottomA;
    float leftB, rightB, topB, bottomB;
    getAABB(a, leftA, rightA, topA, bottomA);
    getAABB(b, leftB, rightB, topB, bottomB);
    
    // If there is no overlap, return.
    if (rightA < leftB || rightB < leftA || bottomA < topB || bottomB < topA)
        return;
    
    // Calculate overlap along x and y.
    float overlapX = std::min(rightA, rightB) - std::max(leftA, leftB);
    float overlapY = std::min(bottomA, bottomB) - std::max(topA, topB);
    
    // Resolve along the least-penetrated axis.
    if (overlapX < overlapY) {
        if (!aStatic && !bStatic) {
            float separation = overlapX / 2.0f;
            if (a->x < b->x) {
                a->x -= separation;
                b->x += separation;
            } else {
                a->x += separation;
                b->x -= separation;
            }
            std::swap(a->vx, b->vx);
            a->vx *= BOUNCE_DAMPING;
            b->vx *= BOUNCE_DAMPING;
        } else if (!aStatic) {
            if (a->x < b->x)
                a->x -= overlapX;
            else
                a->x += overlapX;
            a->vx = -a->vx * BOUNCE_DAMPING;
        } else if (!bStatic) {
            if (b->x < a->x)
                b->x -= overlapX;
            else
                b->x += overlapX;
            b->vx = -b->vx * BOUNCE_DAMPING;
        }
    } else {
        if (!aStatic && !bStatic) {
            float separation = overlapY / 2.0f;
            if (a->y < b->y) {
                a->y -= separation;
                b->y += separation;
            } else {
                a->y += separation;
                b->y -= separation;
            }
            std::swap(a->vy, b->vy);
            a->vy *= BOUNCE_DAMPING;
            b->vy *= BOUNCE_DAMPING;
        } else if (!aStatic) {
            if (a->y < b->y)
                a->y -= overlapY;
            else
                a->y += overlapY;
            a->vy = -a->vy * BOUNCE_DAMPING;
        } else if (!bStatic) {
            if (b->y < a->y)
                b->y -= overlapY;
            else
                b->y += overlapY;
            b->vy = -b->vy * BOUNCE_DAMPING;
        }
    }
}