#pragma once
#include "System.h"
#include "ColliderComponent.h"  // for CollisionLayer enum
#include <vector>
#include <utility>

class GameObject;
class ColliderComponent;

class CollisionSystem : public ISystem {
public:
    CollisionSystem();
    ~CollisionSystem() override;

    void Initialize() override;
    void Update(float deltaTime) override;
    std::string GetName() override { return "CollisionSystem"; }

    // Check if two layers should collide
    static bool ShouldCollide(CollisionLayer layer1, CollisionLayer layer2);

private:
    // Track collision pairs to avoid spam
    std::vector<std::pair<GameObject*, GameObject*>> activeCollisions;

    // Collision response
    void ResolveCollision(GameObject* obj1, GameObject* obj2,
        ColliderComponent* col1, ColliderComponent* col2);
};