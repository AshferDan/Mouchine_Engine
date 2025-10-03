#pragma once
#include "System.h"
#include "ColliderComponent.h"
#include <vector>
#include <utility>

class GameObject;
class ColliderComponent;
class PhysicsSystem;

class CollisionSystem : public ISystem {
public:
    CollisionSystem();
    ~CollisionSystem() override;

    void Initialize() override;
    void Update(float) override;
    std::string GetName() override { return "CollisionSystem"; }

    // Set the physics system to report collisions to
    void SetPhysicsSystem(PhysicsSystem* physSys) { physicsSystem = physSys; }

    // Check if two layers should collide
    static bool ShouldCollide(CollisionLayer layer1, CollisionLayer layer2);

private:
    // Track collision pairs to avoid spam
    std::vector<std::pair<GameObject*, GameObject*>> activeCollisions;

    // Reference to physics system for reporting collisions
    PhysicsSystem* physicsSystem;
};