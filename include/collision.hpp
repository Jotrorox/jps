#ifndef COLLISION_HPP
#define COLLISION_HPP

class Object;

// Resolve a collision between two objects.
// - If both objects are dynamic (Ball), they share separation and exchange momentum.
// - If one object is static (Box) and the other is dynamic, only the dynamic object is moved.
// - If both are static, nothing happens.
void resolveCollision(Object* a, Object* b);

#endif // COLLISION_HPP