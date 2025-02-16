#ifndef PHYSICS_HPP
#define PHYSICS_HPP

#include "ball.hpp"
#include <mutex>

// Update the physics of the ball over a given timestep using a 4th order Runge–Kutta method.
// This function applies air drag and, when the ball is on the ground, ground friction.
void updatePhysics(Ball &ball, float dt);

// Thread function that continuously updates the ball's physics using delta time.
void physicsThreadFunction(bool &running, Ball &ball, std::mutex &ballMutex);

#endif // PHYSICS_HPP