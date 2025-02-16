#include "physics.hpp"
#include <chrono>
#include <thread>
#include <cmath>
#include <algorithm>

// Simulation constants
constexpr int WINDOW_WIDTH  = 800;
constexpr int WINDOW_HEIGHT = 600;
constexpr float GRAVITY     = 980.0f;   // gravitational acceleration (pixels/s^2)
constexpr float AIR_DRAG    = 0.1f;     // linear air resistance coefficient
constexpr float GROUND_FRICTION = 500.0f; // ground friction deceleration (pixels/s^2)
constexpr float TIME_STEP   = 0.001f;   // fixed physics integration timestep (seconds)
constexpr float BOUNCE_DAMPING = 0.7f;  // restitution coefficient for collisions

// Runge-Kutta 4th order integration helper.
// 'acceleration' is a function pointer that computes the acceleration given position and velocity.
static void RK4Step(float &pos, float &vel, float dt, float (*acceleration)(const float, const float)) {
    float k1_v = acceleration(pos, vel);
    float k1_x = vel;

    float v_temp = vel + 0.5f * dt * k1_v;
    float k2_v = acceleration(pos, v_temp);
    float k2_x = v_temp;

    v_temp = vel + 0.5f * dt * k2_v;
    float k3_v = acceleration(pos, v_temp);
    float k3_x = v_temp;

    v_temp = vel + dt * k3_v;
    float k4_v = acceleration(pos, v_temp);
    float k4_x = v_temp;

    pos += dt / 6.0f * (k1_x + 2.0f * k2_x + 2.0f * k3_x + k4_x);
    vel += dt / 6.0f * (k1_v + 2.0f * k2_v + 2.0f * k3_v + k4_v);
}

// Acceleration functions including air drag.
// For x-direction, only air drag applies.
static float accelerationX(const float /*pos*/, const float v) {
    return -AIR_DRAG * v;
}

// For y-direction, gravity and air drag apply.
static float accelerationY(const float /*pos*/, const float v) {
    return GRAVITY - AIR_DRAG * v;
}

// Update the ball state using RK4 integration over dt.
// Also applies collision response and, when the ball is on the ground, applies extra ground friction.
void updatePhysics(Ball &ball, float dt) {
    // Integrate the motion separately in x and y directions using RK4.
    RK4Step(ball.x, ball.vx, dt, accelerationX);
    RK4Step(ball.y, ball.vy, dt, accelerationY);

    // Collision detection and response
    // Floor collision
    if (ball.y + ball.radius > WINDOW_HEIGHT) {
        ball.y = WINDOW_HEIGHT - ball.radius;
        ball.vy = -ball.vy * BOUNCE_DAMPING;
        // Apply ground friction to horizontal velocity if on the floor.
        float frictionDelta = GROUND_FRICTION * dt;
        if (std::fabs(ball.vx) < frictionDelta)
            ball.vx = 0;
        else
            ball.vx -= (ball.vx > 0 ? frictionDelta : -frictionDelta);
    }
    // Ceiling collision
    if (ball.y - ball.radius < 0) {
        ball.y = ball.radius;
        ball.vy = -ball.vy * BOUNCE_DAMPING;
    }
    // Left wall collision
    if (ball.x - ball.radius < 0) {
        ball.x = ball.radius;
        ball.vx = -ball.vx * BOUNCE_DAMPING;
    }
    // Right wall collision
    if (ball.x + ball.radius > WINDOW_WIDTH) {
        ball.x = WINDOW_WIDTH - ball.radius;
        ball.vx = -ball.vx * BOUNCE_DAMPING;
    }
}

// Thread function to update physics using a fixed timestep for improved accuracy.
void physicsThreadFunction(bool &running, Ball &ball, std::mutex &ballMutex) {
    auto previous = std::chrono::high_resolution_clock::now();
    while (running) {
        auto current = std::chrono::high_resolution_clock::now();
        std::chrono::duration<float> elapsed = current - previous;
        previous = current;
        float accumulator = elapsed.count();

        // Break the elapsed time into fixed sized integration steps.
        while (accumulator >= TIME_STEP) {
            {
                std::lock_guard<std::mutex> lock(ballMutex);
                updatePhysics(ball, TIME_STEP);
            }
            accumulator -= TIME_STEP;
        }
        // Process any remaining time as a partial step.
        if (accumulator > 0.0f) {
            std::lock_guard<std::mutex> lock(ballMutex);
            updatePhysics(ball, accumulator);
        }
        // Sleep briefly to prevent busy waiting.
        std::this_thread::sleep_for(std::chrono::microseconds(100));
    }
}