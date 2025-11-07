/* Start Header ************************************************************************/
/*!
\file       CollisionSystem.cpp
\author     Khalil Belabadia [100% - 156 lines]
\par        khalil.b@digipen.edu
\date       October, 2, 2025
Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*/
/* End Header **************************************************************************/

#include "CollisionSystem.h"
#include "PhysicsSystem.h"
#include "GameObject.h"
#include "Factory.h"
#include "ColliderComponent.h"
#include "Collision.h"
#include <iostream>
#include <algorithm>

/**
 * @brief Constructs collision system with null physics reference
 */
CollisionSystem::CollisionSystem() : physicsSystem(nullptr), spatialGrid()
{
}

/**
 * @brief Destructor
 */
CollisionSystem::~CollisionSystem()
{
}

/**
 * @brief Initializes the collision system
 */
void CollisionSystem::Initialize()
{
    std::cout << "Initializing system: CollisionSystem" << std::endl;
}

/**
 * @brief Determines if two collision layers should interact based on layer rules
 * Layer interaction matrix:
 * - Default collides with everything
 * - Player collides with Ground, Wall, Enemy, Platform
 * - Enemy collides with Player, Ground, Wall, Platform, other Enemies
 * - Ground doesn't collide with other Ground or Walls
 * - Walls don't collide with other Walls or Ground
 * - Platforms only collide with Player and Enemy
 */
bool CollisionSystem::ShouldCollide(CollisionLayer layer1, CollisionLayer layer2)
{
    if (layer1 == CollisionLayer::Default || layer2 == CollisionLayer::Default)
    {
        return true;
    }

    if (layer1 == CollisionLayer::Player || layer2 == CollisionLayer::Player)
    {
        CollisionLayer other = (layer1 == CollisionLayer::Player) ? layer2 : layer1;
        return (other == CollisionLayer::Ground ||
                other == CollisionLayer::Wall ||
                other == CollisionLayer::Enemy ||
                other == CollisionLayer::Platform ||
                other == CollisionLayer::Player);
    }

    if (layer1 == CollisionLayer::Enemy || layer2 == CollisionLayer::Enemy)
    {
        CollisionLayer other = (layer1 == CollisionLayer::Enemy) ? layer2 : layer1;
        return (other == CollisionLayer::Player ||
                other == CollisionLayer::Ground ||
                other == CollisionLayer::Wall ||
                other == CollisionLayer::Platform ||
                other == CollisionLayer::Enemy);
    }

    if (layer1 == CollisionLayer::Ground && layer2 == CollisionLayer::Ground)
    {
        return false;
    }

    if (layer1 == CollisionLayer::Wall && layer2 == CollisionLayer::Wall)
    {
        return false;
    }
    if ((layer1 == CollisionLayer::Wall && layer2 == CollisionLayer::Ground) ||
        (layer1 == CollisionLayer::Ground && layer2 == CollisionLayer::Wall))
    {
        return false;
    }

    if (layer1 == CollisionLayer::Platform || layer2 == CollisionLayer::Platform)
    {
        CollisionLayer other = (layer1 == CollisionLayer::Platform) ? layer2 : layer1;
        return (other == CollisionLayer::Player || other == CollisionLayer::Enemy);
    }

    return true;
}

/**
 * @brief Main update loop - detects collisions and reports them to physics system
 * Performs broad-phase collision detection for all game objects with colliders,
 * then narrow-phase testing based on shape types
 */
void CollisionSystem::Update(float deltaTime)
{
    (void)deltaTime;
    auto &factory = GameObjectFactory::Instance();
    const auto &gameObjects = factory.GetAllGameObjects();

    // Clear and rebuild spatial grid each frame
    spatialGrid.Clear();

    // Insert all objects with colliders into the grid
    for (const auto &obj : gameObjects)
    {
        auto *collider = obj->GetComponent<ColliderComponent>();
        if (collider)
        {
            spatialGrid.Insert(obj.get(), collider);
        }
    }

    // Use spatial grid for broad-phase collision detection
    for (const auto &obj1 : gameObjects)
    {
        auto *col1 = obj1->GetComponent<ColliderComponent>();
        if (!col1)
            continue;

        // Query nearby objects from spatial grid (MUCH FASTER than checking all)
        std::vector<GameObject *> nearbyObjects = spatialGrid.Query(obj1.get(), col1);

        // Check collisions only with nearby objects
        for (GameObject *obj2 : nearbyObjects)
        {
            auto *col2 = obj2->GetComponent<ColliderComponent>();
            if (!col2)
                continue;

            bool isColliding = false;

            // Narrow-phase: Detect collision based on shape types
            if (col1->GetType() == ColliderComponent::Type::Circle &&
                col2->GetType() == ColliderComponent::Type::Circle)
            {
                isColliding = CollisionIntersection_CircleCircle_Static(
                    col1->GetCircle(), col2->GetCircle());
            }
            else if (col1->GetType() == ColliderComponent::Type::Square &&
                     col2->GetType() == ColliderComponent::Type::Square)
            {
                isColliding = CollisionIntersection_RectRect_Static(
                    col1->GetAABB(), col2->GetAABB());
            }
            else
            {
                if (col1->GetType() == ColliderComponent::Type::Square)
                {
                    isColliding = CollisionIntersection_RectCircle_Static(
                        col1->GetAABB(), col2->GetCircle());
                }
                else
                {
                    isColliding = CollisionIntersection_RectCircle_Static(
                        col2->GetAABB(), col1->GetCircle());
                }
            }

            // Check layer collision rules
            if (isColliding && !ShouldCollide(col1->GetLayer(), col2->GetLayer()))
            {
                isColliding = false;
            }

            if (isColliding)
            {
                bool wasActive = false;
                for (auto &pair : activeCollisions)
                {
                    if ((pair.first == obj1.get() && pair.second == obj2) ||
                        (pair.first == obj2 && pair.second == obj1.get()))
                    {
                        wasActive = true;
                        break;
                    }
                }

                if (!wasActive)
                {
                    CollisionMessage msg(obj1.get(), obj2);
                    CORE->GetMessageBus().Publish(msg);
                    std::cout << "Collision: " << obj1->GetName() << " <-> "
                              << obj2->GetName() << std::endl;
                    activeCollisions.push_back({obj1.get(), obj2});
                }

                // Report collision to PhysicsSystem for resolution
                if (physicsSystem)
                {
                    physicsSystem->ReportCollision(obj1.get(), obj2, col1, col2);
                }
            }
            else
            {
                // Remove from active collisions
                activeCollisions.erase(
                    std::remove_if(activeCollisions.begin(), activeCollisions.end(),
                                   [&](const auto &pair)
                                   {
                                       return (pair.first == obj1.get() && pair.second == obj2) ||
                                              (pair.first == obj2 && pair.second == obj1.get());
                                   }),
                    activeCollisions.end());
            }
        }
    }
}