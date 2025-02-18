#ifndef PHYSICS_HPP
#define PHYSICS_HPP

#include <vector>
#include <mutex>
#include "object.hpp"

// The physics thread function updates all objects and resolves collisions.
// The vector contains pointers to dynamically allocated Object (Ball or Box).
void physicsThreadFunction(bool &running, std::vector<Object*> &objects, std::mutex &objectsMutex);

#endif // PHYSICS_HPP