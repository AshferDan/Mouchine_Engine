#pragma once
#include "System.h"
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

    // FUNCTION THAT HANDLES COLLISION ONLY BETWEEN TWO CIRCLES
    void CheckAndBounceCircles(GameObject* obj1, GameObject* obj2);

private:
    // Track collision pairs to avoid spam
    std::vector<std::pair<GameObject*, GameObject*>> activeCollisions;

    // GENERAL COLLISION RESPONSE THAT FOR NOW HANDLES ONLY CIRCLE-CIRCLE (WE'LL ADD AABB-AABB AND MIXED COLLISIONS INSHALLAH)
    void ResolveCollision(GameObject* obj1, GameObject* obj2,
        ColliderComponent* col1, ColliderComponent* col2);
};