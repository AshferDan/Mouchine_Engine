#include "CollisionSystem.h"
#include "GameObject.h"
#include "Factory.h"
#include "Physics.h"
#include "Component.h"
#include "ColliderComponent.h"
#include "Transform.h"
#include "Collision.h"
#include <iostream>
#include <cmath>

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

    // Special case: Check our test circles
    GameObject* circle1 = factory.FindGameObject("Circle");
    GameObject* circle2 = factory.FindGameObject("Circle2");
    if (circle1 && circle2) {
        CheckAndBounceCircles(circle1, circle2);
    }

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

            // Handle collision
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

                // Only resolve collision on NEW collision (not every frame)
                if (!wasActive) {
                    std::cout << "Collision detected: " << gameObjects[i]->GetName()
                        << " <-> " << gameObjects[j]->GetName() << std::endl;
                    activeCollisions.push_back({ gameObjects[i].get(), gameObjects[j].get() });

                    // Apply collision response only once when collision first detected
                    ResolveCollision(gameObjects[i].get(), gameObjects[j].get(), col1, col2);
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

// Public function to check and bounce two specific circles
void CollisionSystem::CheckAndBounceCircles(GameObject* obj1, GameObject* obj2) {
    if (!obj1 || !obj2) return;

    // Get colliders
    auto* col1 = obj1->GetComponent<ColliderComponent>();
    auto* col2 = obj2->GetComponent<ColliderComponent>();
    if (!col1 || !col2) return;

    // Only works for circles
    if (col1->GetType() != ColliderComponent::Type::Circle ||
        col2->GetType() != ColliderComponent::Type::Circle) {
        return;
    }

    // Get circles
    Circle c1 = col1->GetCircle();
    Circle c2 = col2->GetCircle();

    // Check collision
    float dx = c2.center.x - c1.center.x;
    float dy = c2.center.y - c1.center.y;
    float distance = std::sqrt(dx * dx + dy * dy);
    float radiusSum = c1.radius + c2.radius;

    bool isColliding = (distance <= radiusSum);

    // Debug output
    static int debugCounter = 0;
    if (debugCounter % 60 == 0) {
        std::cout << "Distance: " << distance << ", RadiusSum: " << radiusSum
            << ", Colliding: " << (isColliding ? "YES" : "NO") << std::endl;
    }
    debugCounter++;

    // Check if this is a new collision
    bool wasActive = false;
    for (auto& pair : activeCollisions) {
        if ((pair.first == obj1 && pair.second == obj2) ||
            (pair.first == obj2 && pair.second == obj1)) {
            wasActive = true;
            break;
        }
    }

    if (isColliding && !wasActive) {
        std::cout << "\n=== BOUNCE DETECTED ===" << std::endl;
        std::cout << obj1->GetName() << " <-> " << obj2->GetName() << std::endl;
        std::cout << "Distance: " << distance << ", Radius sum: " << radiusSum << std::endl;

        // Get physics
        auto* physics1 = obj1->GetComponent<PhysicsComponent>();
        auto* physics2 = obj2->GetComponent<PhysicsComponent>();
        if (!physics1 || !physics2) {
            std::cout << "ERROR: Missing physics component!" << std::endl;
            return;
        }

        // Avoid division by zero
        if (distance < 0.0001f) {
            distance = 0.0001f;
        }

        // Normalize collision normal (direction from obj1 to obj2)
        float nx = dx / distance;
        float ny = dy / distance;

        // Get current velocities
        float vx1, vy1, vx2, vy2;
        physics1->GetVelocity(vx1, vy1);
        physics2->GetVelocity(vx2, vy2);

        std::cout << "Before - " << obj1->GetName() << " vel: (" << vx1 << ", " << vy1 << ")" << std::endl;
        std::cout << "Before - " << obj2->GetName() << " vel: (" << vx2 << ", " << vy2 << ")" << std::endl;

        // Calculate relative velocity
        float relVelX = vx1 - vx2;
        float relVelY = vy1 - vy2;

        // Calculate relative velocity along collision normal
        float velAlongNormal = relVelX * nx + relVelY * ny;

        std::cout << "velAlongNormal: " << velAlongNormal << std::endl;

        // REMOVED THE CHECK - we want to bounce even if they're separating
        // because fast-moving objects can pass through and already be separating

        // Calculate impulse (restitution = 1.0 for perfect elastic bounce)
        float restitution = 1.0f;
        float j = -(1.0f + restitution) * velAlongNormal;
        j /= (1.0f / physics1->GetMass() + 1.0f / physics2->GetMass());

        // Apply impulse along the collision normal
        float impulseX = j * nx;
        float impulseY = j * ny;

        std::cout << "Impulse: (" << impulseX << ", " << impulseY << ")" << std::endl;

        // Update velocities
        vx1 += impulseX / physics1->GetMass();
        vy1 += impulseY / physics1->GetMass();
        physics1->SetVelocity(vx1, vy1);

        vx2 -= impulseX / physics2->GetMass();
        vy2 -= impulseY / physics2->GetMass();
        physics2->SetVelocity(vx2, vy2);

        std::cout << "After  - " << obj1->GetName() << " vel: (" << vx1 << ", " << vy1 << ")" << std::endl;
        std::cout << "After  - " << obj2->GetName() << " vel: (" << vx2 << ", " << vy2 << ")" << std::endl;

        // Separate the circles to prevent overlap/sticking
        float overlap = radiusSum - distance;
        if (overlap > 0) {
            auto* transform1 = obj1->GetComponent<TransformComponent>();
            auto* transform2 = obj2->GetComponent<TransformComponent>();

            if (transform1 && transform2) {
                // Push circles apart equally (each gets half the separation)
                float separationX = nx * overlap * 0.5f;
                float separationY = ny * overlap * 0.5f;

                transform1->Move(-separationX, -separationY);
                transform2->Move(separationX, separationY);

                std::cout << "Separated by: (" << separationX << ", " << separationY << ")" << std::endl;
            }
        }

        // Mark as active collision
        activeCollisions.push_back({ obj1, obj2 });
    }
    else if (!isColliding && wasActive) {
        // Remove from active collisions
        activeCollisions.erase(
            std::remove_if(activeCollisions.begin(), activeCollisions.end(),
                [&](const auto& pair) {
                    return (pair.first == obj1 && pair.second == obj2) ||
                        (pair.first == obj2 && pair.second == obj1);
                }),
            activeCollisions.end()
        );
    }
}

void CollisionSystem::ResolveCollision(GameObject* obj1, GameObject* obj2,
    ColliderComponent* col1, ColliderComponent* col2) {
    // Get physics components
    auto* physics1 = obj1->GetComponent<PhysicsComponent>();
    auto* physics2 = obj2->GetComponent<PhysicsComponent>();

    // Skip if either object doesn't have physics
    if (!physics1 || !physics2) return;

    // Skip static objects
    if (physics1->IsStatic() && physics2->IsStatic()) return;

    // Circle vs Circle collision response
    if (col1->GetType() == ColliderComponent::Type::Circle &&
        col2->GetType() == ColliderComponent::Type::Circle) {

        Circle c1 = col1->GetCircle();
        Circle c2 = col2->GetCircle();

        // Calculate collision normal (direction from obj1 to obj2)
        float dx = c2.center.x - c1.center.x;
        float dy = c2.center.y - c1.center.y;
        float distance = std::sqrt(dx * dx + dy * dy);

        // Avoid division by zero
        if (distance < 0.0001f) return;

        // Normalize the collision normal
        float nx = dx / distance;
        float ny = dy / distance;

        // Separate the circles first (push them apart)
        float overlap = (c1.radius + c2.radius) - distance;
        if (overlap > 0) {
            auto* transform1 = obj1->GetComponent<TransformComponent>();
            auto* transform2 = obj2->GetComponent<TransformComponent>();

            if (transform1 && transform2) {
                // Push each circle half the overlap distance
                float separationX = nx * overlap * 0.5f;
                float separationY = ny * overlap * 0.5f;

                if (!physics1->IsStatic()) {
                    transform1->Move(-separationX, -separationY);
                }
                if (!physics2->IsStatic()) {
                    transform2->Move(separationX, separationY);
                }
            }
        }

        // Get velocities
        float vx1, vy1, vx2, vy2;
        physics1->GetVelocity(vx1, vy1);
        physics2->GetVelocity(vx2, vy2);

        std::cout << "  Before bounce - " << obj1->GetName() << " vel: ("
            << vx1 << ", " << vy1 << "), "
            << obj2->GetName() << " vel: ("
            << vx2 << ", " << vy2 << ")" << std::endl;

        // Calculate relative velocity
        float relVelX = vx1 - vx2;
        float relVelY = vy1 - vy2;

        // Calculate relative velocity in terms of the normal direction
        float velAlongNormal = relVelX * nx + relVelY * ny;

        std::cout << "  velAlongNormal: " << velAlongNormal << std::endl;

        // Don't resolve if velocities are separating
        if (velAlongNormal > 0) {
            std::cout << "  Objects already separating, skipping bounce" << std::endl;
            return;
        }

        // Calculate restitution (bounciness) - 1.0 means perfectly elastic collision
        float restitution = 1.0f;

        // Calculate impulse scalar
        float impulse = -(1 + restitution) * velAlongNormal;
        impulse /= (1.0f / physics1->GetMass() + 1.0f / physics2->GetMass());

        // Apply impulse
        float impulseX = impulse * nx;
        float impulseY = impulse * ny;

        std::cout << "  Impulse: (" << impulseX << ", " << impulseY << ")" << std::endl;

        // Update velocities
        if (!physics1->IsStatic()) {
            vx1 += impulseX / physics1->GetMass();
            vy1 += impulseY / physics1->GetMass();
            physics1->SetVelocity(vx1, vy1);
        }

        if (!physics2->IsStatic()) {
            vx2 -= impulseX / physics2->GetMass();
            vy2 -= impulseY / physics2->GetMass();
            physics2->SetVelocity(vx2, vy2);
        }

        std::cout << "  After bounce - " << obj1->GetName() << " vel: ("
            << vx1 << ", " << vy1 << "), "
            << obj2->GetName() << " vel: ("
            << vx2 << ", " << vy2 << ")" << std::endl;
    }
}