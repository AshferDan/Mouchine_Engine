#include "CollisionSystem.h"
#include "GameObject.h"
#include "Factory.h"
#include "ColliderComponent.h"
#include "Collision.h"
#include <iostream>

CollisionSystem::CollisionSystem() {
}

CollisionSystem::~CollisionSystem() {
}

void CollisionSystem::Initialize() {
    std::cout << "Initializing system: CollisionSystem" << std::endl;
}

void CollisionSystem::Update(float deltaTime) {
    auto& factory = GameObjectFactory::Instance();
    const auto& gameObjects = factory.GetAllGameObjects();

    // Check all pairs of objects
    for (size_t i = 0; i < gameObjects.size(); ++i) {
        auto* col1 = gameObjects[i]->GetComponent<ColliderComponent>();
        if (!col1) continue;

        for (size_t j = i + 1; j < gameObjects.size(); ++j) {
            auto* col2 = gameObjects[j]->GetComponent<ColliderComponent>();
            if (!col2) continue;

            bool isColliding = false;

            // Check collision based on types using your existing functions
            if (col1->GetType() == ColliderComponent::Type::Circle &&
                col2->GetType() == ColliderComponent::Type::Circle) {
                // Circle vs Circle
                isColliding = CollisionIntersection_CircleCircle_Static(
                    col1->GetCircle(), col2->GetCircle());
            }
            else if (col1->GetType() == ColliderComponent::Type::Square &&
                col2->GetType() == ColliderComponent::Type::Square) {
                // AABB vs AABB
                isColliding = CollisionIntersection_RectRect_Static(
                    col1->GetAABB(), col2->GetAABB());
            }
            else {
                // Mixed: AABB vs Circle
                if (col1->GetType() == ColliderComponent::Type::Square) {
                    isColliding = CollisionIntersection_RectCircle_Static(
                        col1->GetAABB(), col2->GetCircle());
                }
                else {
                    isColliding = CollisionIntersection_RectCircle_Static(
                        col2->GetAABB(), col1->GetCircle());
                }
            }

            if (isColliding) {
                // Check if this collision was already active
                bool wasActive = false;
                for (auto& pair : activeCollisions) {
                    if ((pair.first == gameObjects[i].get() && pair.second == gameObjects[j].get()) ||
                        (pair.first == gameObjects[j].get() && pair.second == gameObjects[i].get())) {
                        wasActive = true;
                        break;
                    }
                }

                // Only print on new collision
                if (!wasActive) {
                    std::cout << "Collision detected: " << gameObjects[i]->GetName()
                        << " <-> " << gameObjects[j]->GetName() << std::endl;
                    activeCollisions.push_back({ gameObjects[i].get(), gameObjects[j].get() });
                }
            }
            else {
                // Remove from active collisions if it was there
                activeCollisions.erase(
                    std::remove_if(activeCollisions.begin(), activeCollisions.end(),
                        [&](const auto& pair) {
                            return (pair.first == gameObjects[i].get() && pair.second == gameObjects[j].get()) ||
                                (pair.first == gameObjects[j].get() && pair.second == gameObjects[i].get());
                        }),
                    activeCollisions.end()
                );
            }
        }
    }
}