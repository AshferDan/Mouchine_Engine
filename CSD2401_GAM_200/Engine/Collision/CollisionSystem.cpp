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
    // Get the factory singleton that manages all game objects
    auto& factory = GameObjectFactory::Instance();
    const auto& gameObjects = factory.GetAllGameObjects();

    // Double loop to check every possible pair of objects
    for (size_t i = 0; i < gameObjects.size(); ++i) {
        // Get ColliderComponent from first object
        auto* col1 = gameObjects[i]->GetComponent<ColliderComponent>();
        if (!col1) continue; // Skip if no collider

        // Start j at i+1 to avoid checking pairs twice (A-B and B-A)
        for (size_t j = i + 1; j < gameObjects.size(); ++j) {
            // Get ColliderComponent from second object
            auto* col2 = gameObjects[j]->GetComponent<ColliderComponent>();
            if (!col2) continue; // Skip if no collider

            bool isColliding = false;

            // Check collision based on shape types
            if (col1->GetType() == ColliderComponent::Type::Circle &&
                col2->GetType() == ColliderComponent::Type::Circle) {
                // Circle vs Circle - uses circle vs circle function in collision.h
                isColliding = CollisionIntersection_CircleCircle_Static(
                    col1->GetCircle(), col2->GetCircle());
            }
            else if (col1->GetType() == ColliderComponent::Type::Square &&
                col2->GetType() == ColliderComponent::Type::Square) {
                // AABB vs AABB - uses rect vs rect function in collision.h
                isColliding = CollisionIntersection_RectRect_Static(
                    col1->GetAABB(), col2->GetAABB());
            }
            else {
                // Mixed: AABB vs Circle - uses circle vs rect function in collision.h
                if (col1->GetType() == ColliderComponent::Type::Square) {
                    isColliding = CollisionIntersection_RectCircle_Static(
                        col1->GetAABB(), col2->GetCircle());
                }
                else {
                    isColliding = CollisionIntersection_RectCircle_Static(
                        col2->GetAABB(), col1->GetCircle());
                }
            }

            // Track collision state to avoid spam on console
            if (isColliding) {
                // Check if this collision was already active
                bool wasActive = false;
                for (auto& pair : activeCollisions) {
                    // See if this pair is already tracked
                    if ((pair.first == gameObjects[i].get() && pair.second == gameObjects[j].get()) ||
                        (pair.first == gameObjects[j].get() && pair.second == gameObjects[i].get())) {
                        wasActive = true;
                        break;
                    }
                }

                // Only print on NEW collision (not every frame)
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