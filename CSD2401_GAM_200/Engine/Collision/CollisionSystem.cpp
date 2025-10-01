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

void CollisionSystem::Update(float deltaTime) {
    auto& factory = GameObjectFactory::Instance();
    const auto& gameObjects = factory.GetAllGameObjects();

    for (size_t i = 0; i < gameObjects.size(); ++i) {
        auto* col1 = gameObjects[i]->GetComponent<ColliderComponent>();
        if (!col1) continue;

        for (size_t j = i + 1; j < gameObjects.size(); ++j) {
            auto* col2 = gameObjects[j]->GetComponent<ColliderComponent>();
            if (!col2) continue;

            bool isColliding = false;

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

                ResolveCollision(gameObjects[i].get(), gameObjects[j].get(), col1, col2);
            }
            else {
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

void CollisionSystem::ResolveCollision(GameObject* obj1, GameObject* obj2,
    ColliderComponent* col1, ColliderComponent* col2) {
    auto* physics1 = obj1->GetComponent<PhysicsComponent>();
    auto* physics2 = obj2->GetComponent<PhysicsComponent>();

    if (!physics1 || !physics2) return;
    if (physics1->IsStatic() && physics2->IsStatic()) return;

    // Circle vs Circle
    if (col1->GetType() == ColliderComponent::Type::Circle &&
        col2->GetType() == ColliderComponent::Type::Circle) {

        Circle c1 = col1->GetCircle();
        Circle c2 = col2->GetCircle();

        float dx = c2.center.x - c1.center.x;
        float dy = c2.center.y - c1.center.y;
        float distance = std::sqrt(dx * dx + dy * dy);

        if (distance < 0.0001f) {
            distance = 0.0001f;
        }

        float nx = dx / distance;
        float ny = dy / distance;

        float overlap = (c1.radius + c2.radius) - distance;
        if (overlap > 0) {
            auto* transform1 = obj1->GetComponent<TransformComponent>();
            auto* transform2 = obj2->GetComponent<TransformComponent>();

            if (transform1 && transform2) {
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

        float vx1, vy1, vx2, vy2;
        physics1->GetVelocity(vx1, vy1);
        physics2->GetVelocity(vx2, vy2);

        float relVelX = vx1 - vx2;
        float relVelY = vy1 - vy2;
        float velAlongNormal = relVelX * nx + relVelY * ny;

        float restitution = 0.8f;
        float impulse = -(1 + restitution) * velAlongNormal;
        impulse /= (1.0f / physics1->GetMass() + 1.0f / physics2->GetMass());

        float impulseX = impulse * nx;
        float impulseY = impulse * ny;

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
    }
    // Square vs Square
    else if (col1->GetType() == ColliderComponent::Type::Square &&
        col2->GetType() == ColliderComponent::Type::Square) {

        AABB aabb1 = col1->GetAABB();
        AABB aabb2 = col2->GetAABB();

        float overlapX = std::min(aabb1.max.x - aabb2.min.x, aabb2.max.x - aabb1.min.x);
        float overlapY = std::min(aabb1.max.y - aabb2.min.y, aabb2.max.y - aabb1.min.y);

        if (overlapX < overlapY) {
            float direction = (aabb1.min.x + aabb1.max.x) < (aabb2.min.x + aabb2.max.x) ? -1.0f : 1.0f;

            auto* transform1 = obj1->GetComponent<TransformComponent>();
            auto* transform2 = obj2->GetComponent<TransformComponent>();

            if (!physics1->IsStatic() && transform1) {
                transform1->Move(direction * overlapX * 0.5f, 0);
                physics1->SetVelocity(0, physics1->velocityY);
            }
            if (!physics2->IsStatic() && transform2) {
                transform2->Move(-direction * overlapX * 0.5f, 0);
                physics2->SetVelocity(0, physics2->velocityY);
            }
        }
        else {
            float direction = (aabb1.min.y + aabb1.max.y) < (aabb2.min.y + aabb2.max.y) ? -1.0f : 1.0f;

            auto* transform1 = obj1->GetComponent<TransformComponent>();
            auto* transform2 = obj2->GetComponent<TransformComponent>();

            if (!physics1->IsStatic() && transform1) {
                transform1->Move(0, direction * overlapY * 0.5f);
                physics1->SetVelocity(physics1->velocityX, 0);
                if (direction < 0 && physics1->velocityY <= 0) {
                    physics1->SetGrounded(true);
                }
            }
            if (!physics2->IsStatic() && transform2) {
                transform2->Move(0, -direction * overlapY * 0.5f);
                physics2->SetVelocity(physics2->velocityX, 0);
                if (direction > 0 && physics2->velocityY <= 0) {
                    physics2->SetGrounded(true);
                }
            }
        }
    }
    // Mixed: Square vs Circle
    else {
        ColliderComponent* squareCol = (col1->GetType() == ColliderComponent::Type::Square) ? col1 : col2;
        ColliderComponent* circleCol = (col1->GetType() == ColliderComponent::Type::Circle) ? col1 : col2;
        GameObject* squareObj = (col1->GetType() == ColliderComponent::Type::Square) ? obj1 : obj2;
        GameObject* circleObj = (col1->GetType() == ColliderComponent::Type::Circle) ? obj1 : obj2;
        auto* squarePhysics = squareObj->GetComponent<PhysicsComponent>();
        auto* circlePhysics = circleObj->GetComponent<PhysicsComponent>();

        if (!squarePhysics || !circlePhysics) return;

        AABB rect = squareCol->GetAABB();
        Circle circle = circleCol->GetCircle();

        float closestX = std::max(rect.min.x, std::min(circle.center.x, rect.max.x));
        float closestY = std::max(rect.min.y, std::min(circle.center.y, rect.max.y));

        float dx = circle.center.x - closestX;
        float dy = circle.center.y - closestY;
        float distance = std::sqrt(dx * dx + dy * dy);

        if (distance < 0.0001f) return;

        float nx = dx / distance;
        float ny = dy / distance;

        float penetration = circle.radius - distance;

        if (penetration > 0) {
            auto* squareTransform = squareObj->GetComponent<TransformComponent>();
            auto* circleTransform = circleObj->GetComponent<TransformComponent>();

            float cvx, cvy;
            circlePhysics->GetVelocity(cvx, cvy);

            if (!squarePhysics->IsStatic() && squareTransform) {
                squareTransform->Move(-nx * penetration * 0.5f, -ny * penetration * 0.5f);
                squarePhysics->SetVelocity(0, 0);
            }
            if (!circlePhysics->IsStatic() && circleTransform) {
                circleTransform->Move(nx * penetration, ny * penetration);

                float restitution = 0.6f;
                float dot = cvx * nx + cvy * ny;

                cvx = cvx - (1 + restitution) * dot * nx;
                cvy = cvy - (1 + restitution) * dot * ny;

                circlePhysics->SetVelocity(cvx, cvy);

                if (ny > 0.7f && std::abs(cvy) < 1.0f) {
                    circlePhysics->SetVelocity(cvx, 0);
                    circlePhysics->SetGrounded(true);
                }
            }
        }
    }
}