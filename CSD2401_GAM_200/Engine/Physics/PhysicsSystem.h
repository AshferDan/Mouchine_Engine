#pragma once
#include "System.h"
#include "ColliderComponent.h"
#include <vector>

class GameObject;
class PhysicsComponent;
class TransformComponent;

// Structure to hold collision data
struct CollisionData {
    GameObject* obj1;
    GameObject* obj2;
    ColliderComponent* col1;
    ColliderComponent* col2;
};

class PhysicsSystem : public ISystem {
public:
    PhysicsSystem();
    ~PhysicsSystem() override;

    void Initialize() override;
    void Update(float deltaTime) override;
    std::string GetName() override { return "PhysicsSystem"; }

    // Called by CollisionSystem to report collisions
    void ReportCollision(GameObject* obj1, GameObject* obj2,
        ColliderComponent* col1, ColliderComponent* col2);

private:
    // Collision resolution functions
    void ResolveCircleCircle(GameObject* obj1, GameObject* obj2,
        ColliderComponent* col1, ColliderComponent* col2);

    void ResolveSquareSquare(GameObject* obj1, GameObject* obj2,
        ColliderComponent* col1, ColliderComponent* col2);

    void ResolveMixed(GameObject* obj1, GameObject* obj2,
        ColliderComponent* col1, ColliderComponent* col2);

    // Physics helper functions
    void ApplyGravity(PhysicsComponent* physics, float deltaTime);
    void IntegrateVelocity(PhysicsComponent* physics, TransformComponent* transform, float deltaTime);

    // Store collisions reported this frame
    std::vector<CollisionData> pendingCollisions;


};
