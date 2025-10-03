/* Start Header ************************************************************************/
/*!
\file       PhysicsSystem.h
\author     Khalil Belabadia [100% - 33 lines]
\par        khalil.b@digipen.edu
\date       October, 2, 2025
Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*/
/* End Header **************************************************************************/

#pragma once
#include "System.h"
#include "ColliderComponent.h"
#include <vector>

class GameObject;
class PhysicsComponent;
class TransformComponent;

/**
 * @brief Structure to hold collision data for deferred resolution
 */
struct CollisionData {
    GameObject* obj1;
    GameObject* obj2;
    ColliderComponent* col1;
    ColliderComponent* col2;
};

class PhysicsSystem : public ISystem {
public:
    /**
     * @brief Default constructor
     */
    PhysicsSystem();

    /**
     * @brief Destructor
     */
    ~PhysicsSystem() override;

    /**
     * @brief Initializes the physics system
     */
    void Initialize() override;

    /**
     * @brief Updates physics simulation each frame
     * @param deltaTime Time elapsed since last update
     */
    void Update(float deltaTime) override;

    /**
     * @brief Gets the system name
     * @return String identifier for this system
     */
    std::string GetName() override { return "PhysicsSystem"; }

    /**
     * @brief Called by CollisionSystem to report detected collisions
     * @param obj1 First colliding object
     * @param obj2 Second colliding object
     * @param col1 First object's collider
     * @param col2 Second object's collider
     */
    void ReportCollision(GameObject* obj1, GameObject* obj2,
        ColliderComponent* col1, ColliderComponent* col2);

private:
    /**
    * @brief Resolves collision between two circles
    * @param obj1 First circle object
    * @param obj2 Second circle object
    * @param col1 First circle's collider
    * @param col2 Second circle's collider
    */
    void ResolveCircleCircle(GameObject* obj1, GameObject* obj2,
        ColliderComponent* col1, ColliderComponent* col2);

    /**
     * @brief Resolves collision between two squares
     * @param obj1 First square object
     * @param obj2 Second square object
     * @param col1 First square's collider
     * @param col2 Second square's collider
     */
    void ResolveSquareSquare(GameObject* obj1, GameObject* obj2,
        ColliderComponent* col1, ColliderComponent* col2);

    /**
     * @brief Resolves collision between mixed shape types
     * @param obj1 First object
     * @param obj2 Second object
     * @param col1 First object's collider
     * @param col2 Second object's collider
     */
    void ResolveMixed(GameObject* obj1, GameObject* obj2,
        ColliderComponent* col1, ColliderComponent* col2);

    /**
     * @brief Applies gravity acceleration to a physics component
     * @param physics Physics component to update
     * @param deltaTime Frame time
     */
    void ApplyGravity(PhysicsComponent* physics, float deltaTime);

    /**
     * @brief Integrates velocity into position
     * @param physics Physics component with velocity
     * @param transform Transform to update
     * @param deltaTime Frame time
     */
    void IntegrateVelocity(PhysicsComponent* physics, TransformComponent* transform, float deltaTime);

    // Store collisions reported this frame for batch resolution
    std::vector<CollisionData> pendingCollisions;


};
