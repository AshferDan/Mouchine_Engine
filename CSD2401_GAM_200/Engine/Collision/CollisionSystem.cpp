#include "CollisionSystem.h"
#include "PhysicsSystem.h"
#include "GameObject.h"
#include "Factory.h"
#include "ColliderComponent.h"
#include "Collision.h"
#include <iostream>
#include <algorithm>

CollisionSystem::CollisionSystem() : physicsSystem(nullptr) {
}

CollisionSystem::~CollisionSystem() {
}

void CollisionSystem::Initialize() {
    std::cout << "Initializing system: CollisionSystem" << std::endl;
}

bool CollisionSystem::ShouldCollide(CollisionLayer layer1, CollisionLayer layer2) {
    if (layer1 == CollisionLayer::Default || layer2 == CollisionLayer::Default) {
        return true;
    }

    if (layer1 == CollisionLayer::Player || layer2 == CollisionLayer::Player) {
        CollisionLayer other = (layer1 == CollisionLayer::Player) ? layer2 : layer1;
        return (other == CollisionLayer::Ground ||
            other == CollisionLayer::Wall ||
            other == CollisionLayer::Enemy ||
            other == CollisionLayer::Platform ||
            other == CollisionLayer::Player);
    }

    if (layer1 == CollisionLayer::Enemy || layer2 == CollisionLayer::Enemy) {
        CollisionLayer other = (layer1 == CollisionLayer::Enemy) ? layer2 : layer1;
        return (other == CollisionLayer::Player ||
            other == CollisionLayer::Ground ||
            other == CollisionLayer::Wall ||
            other == CollisionLayer::Platform ||
            other == CollisionLayer::Enemy);
    }

    if (layer1 == CollisionLayer::Ground && layer2 == CollisionLayer::Ground) {
        return false;
    }

    if (layer1 == CollisionLayer::Wall && layer2 == CollisionLayer::Wall) {
        return false;
    }
    if ((layer1 == CollisionLayer::Wall && layer2 == CollisionLayer::Ground) ||
        (layer1 == CollisionLayer::Ground && layer2 == CollisionLayer::Wall)) {
        return false;
    }

    if (layer1 == CollisionLayer::Platform || layer2 == CollisionLayer::Platform) {
        CollisionLayer other = (layer1 == CollisionLayer::Platform) ? layer2 : layer1;
        return (other == CollisionLayer::Player || other == CollisionLayer::Enemy);
    }

    return true;
}

void CollisionSystem::Update(float) {
    auto& factory = GameObjectFactory::Instance();
    const auto& gameObjects = factory.GetAllGameObjects();

    // ONLY collision detection now - no resolution
    for (size_t i = 0; i < gameObjects.size(); ++i) {
        auto* col1 = gameObjects[i]->GetComponent<ColliderComponent>();
        if (!col1) continue;

        for (size_t j = i + 1; j < gameObjects.size(); ++j) {
            auto* col2 = gameObjects[j]->GetComponent<ColliderComponent>();
            if (!col2) continue;

            bool isColliding = false;

            // Detect collision based on shape types
            if (col1->GetType() == ColliderComponent::Type::Circle &&
                col2->GetType() == ColliderComponent::Type::Circle) {
                isColliding = CollisionIntersection_CircleCircle_Static(
                    col1->GetCircle(), col2->GetCircle());
            }
            else if (col1->GetType() == ColliderComponent::Type::Square &&
                col2->GetType() == ColliderComponent::Type::Square) {
                isColliding = CollisionIntersection_RectRect_Static(
                    col1->GetAABB(), col2->GetAABB());
            }
            else {
                if (col1->GetType() == ColliderComponent::Type::Square) {
                    isColliding = CollisionIntersection_RectCircle_Static(
                        col1->GetAABB(), col2->GetCircle());
                }
                else {
                    isColliding = CollisionIntersection_RectCircle_Static(
                        col2->GetAABB(), col1->GetCircle());
                }
            }

            // Check layer collision rules
            if (isColliding && !ShouldCollide(col1->GetLayer(), col2->GetLayer())) {
                isColliding = false;
            }

            if (isColliding) {
                bool wasActive = false;
                for (auto& pair : activeCollisions) {
                    if ((pair.first == gameObjects[i].get() && pair.second == gameObjects[j].get()) ||
                        (pair.first == gameObjects[j].get() && pair.second == gameObjects[i].get())) {
                        wasActive = true;
                        break;
                    }
                }

                if (!wasActive) {
                    std::cout << "Collision: " << gameObjects[i]->GetName() << " <-> "
                        << gameObjects[j]->GetName() << std::endl;
                    activeCollisions.push_back({ gameObjects[i].get(), gameObjects[j].get() });
                }

                // Report collision to PhysicsSystem for resolution
                if (physicsSystem) {
                    physicsSystem->ReportCollision(gameObjects[i].get(), gameObjects[j].get(), col1, col2);
                }
            }
            else {
                // Remove from active collisions
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