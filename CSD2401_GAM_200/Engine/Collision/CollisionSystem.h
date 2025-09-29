#pragma once
#include "System.h"
#include <vector>
#include <utility>

class GameObject;

class CollisionSystem : public ISystem {
public:
    CollisionSystem();
    ~CollisionSystem() override;

    void Initialize() override;
    void Update(float deltaTime) override;
    std::string GetName() override { return "CollisionSystem"; }

private:
    // Track collision pairs to avoid spam
    std::vector<std::pair<GameObject*, GameObject*>> activeCollisions;
};