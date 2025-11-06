/* Start Header ************************************************************************/
/*!
\file       CollisionSystem.h
\author     Khalil Belabadia [100% - 21 lines]
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
#include "SpatialGrid.h"
#include <vector>
#include <utility>

class GameObject;
class ColliderComponent;
class PhysicsSystem;

class CollisionSystem : public ISystem {
public:
    /**
     * @brief Default constructor
     */
    CollisionSystem();

    /**
     * @brief Destructor
     */
    ~CollisionSystem() override;

    /**
     * @brief Initializes the collision system
     */
    void Initialize() override;

    /**
     * @brief Updates collision detection each frame
     * @param deltaTime Time elapsed since last update
     */
    void Update(float deltaTime) override;

    /**
     * @brief Gets the system name
     * @return String identifier for this system
     */
    std::string GetName() override { return "CollisionSystem"; }

    /**
     * @brief Sets the physics system for collision resolution
     * @param physSys Pointer to the physics system
     */
    void SetPhysicsSystem(PhysicsSystem* physSys) { physicsSystem = physSys; }

    /**
     * @brief Checks if two collision layers should interact
     * @param layer1 First collision layer
     * @param layer2 Second collision layer
     * @return True if the layers should collide
     */
    static bool ShouldCollide(CollisionLayer layer1, CollisionLayer layer2);

private:
    // Track collision pairs to avoid spam
    std::vector<std::pair<GameObject*, GameObject*>> activeCollisions;

    // Reference to physics system for reporting collisions
    PhysicsSystem* physicsSystem;

    // Spatial grid for broad-phase collision detection
    SpatialGrid spatialGrid;
};