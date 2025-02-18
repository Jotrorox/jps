#include "physics.hpp"
#include "object.hpp"
#include "collision.hpp"
#include <chrono>
#include <thread>
#include <mutex>
#include <vector>

constexpr float TIME_STEP = 0.001f;

void physicsThreadFunction(bool &running, std::vector<Object*> &objects, std::mutex &objectsMutex) {
    auto previous = std::chrono::high_resolution_clock::now();
    while (running) {
        auto current = std::chrono::high_resolution_clock::now();
        std::chrono::duration<float> elapsed = current - previous;
        previous = current;
        float accumulator = elapsed.count();
        while (accumulator >= TIME_STEP) {
            {
                std::lock_guard<std::mutex> lock(objectsMutex);
                // Update physics for each object. Only dynamic objects (Ball) perform updates.
                for (auto obj : objects) {
                    obj->updatePhysics(TIME_STEP);
                }
                // Resolve collisions in a pairwise manner.
                for (size_t i = 0; i < objects.size(); ++i) {
                    for (size_t j = i + 1; j < objects.size(); ++j) {
                        resolveCollision(objects[i], objects[j]);
                    }
                }
            }
            accumulator -= TIME_STEP;
        }
        if (accumulator > 0.0f) {
            std::lock_guard<std::mutex> lock(objectsMutex);
            for (auto obj : objects) {
                obj->updatePhysics(accumulator);
            }
            for (size_t i = 0; i < objects.size(); ++i) {
                for (size_t j = i + 1; j < objects.size(); ++j) {
                    resolveCollision(objects[i], objects[j]);
                }
            }
        }
        std::this_thread::sleep_for(std::chrono::microseconds(100));
    }
}