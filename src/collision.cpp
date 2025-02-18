#include "collision.hpp"
#include "object.hpp"
#include "ball.hpp"
#include "box.hpp"
#include <cmath>
#include <algorithm>
#include <utility>

constexpr float BOUNCE_DAMPING = 0.7f;
constexpr float FRICTION_COEFFICIENT = 0.2f; // coefficient for tangential friction

// Helper clamp function.
static float clamp(float value, float min, float max) {
    return std::max(min, std::min(value, max));
}

// Resolve collision between two balls using circle collision resolution with friction.
static void resolveBallBallCollision(Object* a, Object* b) {
    // Cast objects to Ball type.
    Ball* ballA = static_cast<Ball*>(a);
    Ball* ballB = static_cast<Ball*>(b);

    // Compute vector between ball centers.
    float dx = ballB->x - ballA->x;
    float dy = ballB->y - ballA->y;
    float distance = std::hypot(dx, dy);
    float combinedRadius = ballA->radius + ballB->radius;

    // If balls are not overlapping or exactly overlapping, no need to resolve.
    if (distance >= combinedRadius || distance == 0.0f)
        return;

    // Collision normal.
    float invDist = 1.0f / distance;
    float nx = dx * invDist;
    float ny = dy * invDist;

    // Penetration depth.
    float penetration = combinedRadius - distance;

    // Separate the balls proportionally (assuming equal mass).
    ballA->x -= nx * penetration * 0.5f;
    ballA->y -= ny * penetration * 0.5f;
    ballB->x += nx * penetration * 0.5f;
    ballB->y += ny * penetration * 0.5f;

    // Relative velocity.
    float rvx = ballB->vx - ballA->vx;
    float rvy = ballB->vy - ballA->vy;
    float velAlongNormal = rvx * nx + rvy * ny;

    // If the balls are separating already, no impulse is needed.
    if (velAlongNormal > 0)
        return;

    // Calculate impulse scalar (assuming unit mass).
    float impulseScalar = -(1.0f + BOUNCE_DAMPING) * velAlongNormal / 2.0f;
    float impulseX = impulseScalar * nx;
    float impulseY = impulseScalar * ny;

    // Apply impulse.
    ballA->vx -= impulseX;
    ballA->vy -= impulseY;
    ballB->vx += impulseX;
    ballB->vy += impulseY;

    // Apply friction impulse to simulate tangential resistance.
    float tangentX = rvx - (rvx * nx + rvy * ny) * nx;
    float tangentY = rvy - (rvx * nx + rvy * ny) * ny;
    float tangentMag = std::hypot(tangentX, tangentY);
    if (tangentMag > 1e-4f) {
        tangentX /= tangentMag;
        tangentY /= tangentMag;
        float vt = rvx * tangentX + rvy * tangentY;
        // Friction impulse proportional to normal impulse.
        float frictionImpulse = -FRICTION_COEFFICIENT * impulseScalar;
        // Limit friction impulse to prevent over-correction.
        if (std::fabs(frictionImpulse) > std::fabs(vt) / 2.0f)
            frictionImpulse = (vt < 0 ? 1 : -1) * std::fabs(vt) / 2.0f;
        ballA->vx -= frictionImpulse * tangentX;
        ballA->vy -= frictionImpulse * tangentY;
        ballB->vx += frictionImpulse * tangentX;
        ballB->vy += frictionImpulse * tangentY;
    }
}

// Resolve collision between a ball and a box using circle-AABB collision detection.
// This provides more accurate contact resolution such that the ball can rotate or "roll off" the edges.
static void resolveBallBoxCollision(Ball* ball, Box* box) {
    // Box parameters.
    float boxHalfWidth = box->width * 0.5f;
    float boxHalfHeight = box->height * 0.5f;
    // Find the closest point on the box to the circle center.
    float closestX = clamp(ball->x, box->x - boxHalfWidth, box->x + boxHalfWidth);
    float closestY = clamp(ball->y, box->y - boxHalfHeight, box->y + boxHalfHeight);

    // Vector from closest point to ball center.
    float diffX = ball->x - closestX;
    float diffY = ball->y - closestY;
    float distance = std::hypot(diffX, diffY);

    // If no collision, exit.
    if (distance >= ball->radius || distance == 0.0f)
        return;

    // Collision normal.
    float invDistance = 1.0f / distance;
    float nx = diffX * invDistance;
    float ny = diffY * invDistance;
    float penetration = ball->radius - distance;

    // Move ball out of collision.
    ball->x += nx * penetration;
    ball->y += ny * penetration;

    // Relative velocity (box is static, so it's just the ball’s velocity).
    float velAlongNormal = ball->vx * nx + ball->vy * ny;

    // Do not resolve if motion is separating.
    if (velAlongNormal > 0)
        return;

    // Compute impulse scalar.
    float impulseScalar = -(1.0f + BOUNCE_DAMPING) * velAlongNormal;
    float impulseX = impulseScalar * nx;
    float impulseY = impulseScalar * ny;

    // Apply impulse.
    ball->vx += impulseX;
    ball->vy += impulseY;

    // Friction component for tangential impulse.
    float dot = ball->vx * nx + ball->vy * ny;
    float tangentX = ball->vx - dot * nx;
    float tangentY = ball->vy - dot * ny;
    float tangentMag = std::hypot(tangentX, tangentY);
    if (tangentMag > 1e-4f) {
        tangentX /= tangentMag;
        tangentY /= tangentMag;
        float vt = ball->vx * tangentX + ball->vy * tangentY;
        float frictionImpulse = -FRICTION_COEFFICIENT * impulseScalar;
        if (std::fabs(frictionImpulse) > std::fabs(vt))
            frictionImpulse = (vt < 0 ? 1 : -1) * std::fabs(vt);
        ball->vx += frictionImpulse * tangentX;
        ball->vy += frictionImpulse * tangentY;
    }
}

// For generic AABB collisions (e.g., between two boxes, or non-circle cases).
static void resolveAABBCollision(Object* a, Object* b) {
    float leftA, rightA, topA, bottomA;
    float leftB, rightB, topB, bottomB;
    
    // For a ball, use its circle AABB; for box, use its bounds.
    if (a->type == ObjectType::BALL) {
        Ball* ball = static_cast<Ball*>(a);
        leftA   = ball->x - ball->radius;
        rightA  = ball->x + ball->radius;
        topA    = ball->y - ball->radius;
        bottomA = ball->y + ball->radius;
    } else {
        Box* box = static_cast<Box*>(a);
        leftA   = box->x - box->width * 0.5f;
        rightA  = box->x + box->width * 0.5f;
        topA    = box->y - box->height * 0.5f;
        bottomA = box->y + box->height * 0.5f;
    }
    
    if (b->type == ObjectType::BALL) {
        Ball* ball = static_cast<Ball*>(b);
        leftB   = ball->x - ball->radius;
        rightB  = ball->x + ball->radius;
        topB    = ball->y - ball->radius;
        bottomB = ball->y + ball->radius;
    } else {
        Box* box = static_cast<Box*>(b);
        leftB   = box->x - box->width * 0.5f;
        rightB  = box->x + box->width * 0.5f;
        topB    = box->y - box->height * 0.5f;
        bottomB = box->y + box->height * 0.5f;
    }
    
    // No collision.
    if (rightA < leftB || rightB < leftA || bottomA < topB || bottomB < topA)
        return;
    
    // Calculate overlap.
    float overlapX = std::min(rightA, rightB) - std::max(leftA, leftB);
    float overlapY = std::min(bottomA, bottomB) - std::max(topA, topB);
    
    bool aStatic = (a->type == ObjectType::BOX);
    bool bStatic = (b->type == ObjectType::BOX);
    
    // Resolve along the axis of least penetration.
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

void resolveCollision(Object* a, Object* b) {
    // If both objects are balls, use circle collision resolution.
    if (a->type == ObjectType::BALL && b->type == ObjectType::BALL) {
        resolveBallBallCollision(a, b);
    } 
    // If one is a ball and the other a box, use the more accurate circle-AABB collision.
    else if (a->type == ObjectType::BALL && b->type == ObjectType::BOX) {
        resolveBallBoxCollision(static_cast<Ball*>(a), static_cast<Box*>(b));
    } 
    else if (a->type == ObjectType::BOX && b->type == ObjectType::BALL) {
        resolveBallBoxCollision(static_cast<Ball*>(b), static_cast<Box*>(a));
    }
    // Otherwise, use generic AABB resolution.
    else {
        resolveAABBCollision(a, b);
    }
}