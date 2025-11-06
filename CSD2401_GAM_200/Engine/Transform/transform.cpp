/* Start Header ************************************************************************/
/*!
\file       tranform.cpp
\author     Leck Xin Yee [53% - 48 lines]
\co-author	Lee Hwee Min [47% - 43 lines]
\par        xinyee.leck@digipen.edu, l.hweemin@digipen.edu
\date       October, 3, 2025

Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*/
/* End Header **************************************************************************/

#include "transform.h"
#include "GameObject.h"
#include "factory.h"
#include <iostream>
#include <cmath>

/**
 * @brief Constructs a TransformComponent with initial position and default rotation/scale
 */
TransformComponent::TransformComponent(GameObject *owner, float x, float y)
    : IComponent(owner), posX(x), posY(y), rotation(0), scaleX(1), scaleY(1)
{
}

/**
 * @brief Creates a deep copy of this TransformComponent for a new GameObject
 */
std::unique_ptr<IComponent> TransformComponent::Clone(GameObject *NewClone) const
{
    auto clonedComponents = std::make_unique<TransformComponent>(NewClone, this->posX, this->posY);
    clonedComponents->rotation = this->rotation;
    clonedComponents->scaleX = this->scaleX;
    clonedComponents->scaleY = this->scaleY;

    clonedComponents->SetEnabled(this->IsEnabled());
    return clonedComponents;
}

/**
 * @brief Sets the absolute position of the transform
 */
void TransformComponent::SetPosition(float x, float y)
{
    float diffX{ posX - x }, diffY{ posY - y };
    posX = x;
    posY = y;
    if (!owner->GetAllChildren().empty())
    {
        GameObjectFactory& factory = GameObjectFactory::Instance();
        GameObject* GO = factory.GetGameObject(owner->GetID());
        std::vector<EntityID> familyTree{};
        GetFamilyTree(GO, familyTree);

        std::cout << "GetFamilyTree() has: \n";
        for (auto const& r : familyTree)
        {
            std::cout << r << std::endl;
        }

        // incredibly scuffed but it kinda works
        // better to add a local (WRT parent) and global (scene space) vec2 for positions, then just adjust accordingly
        for (auto& childID : familyTree)
        {
            GameObject* childGO = factory.GetGameObject(childID);
            vec2 newPos{};
            childGO->GetComponent<TransformComponent>()->GetPosition(newPos.x, newPos.y);
            childGO->GetComponent<TransformComponent>()->SetPosition(newPos.x + diffX, newPos.y + diffY);
        }
    }
}

/**
 * @brief Gets the current position coordinates
 */
void TransformComponent::GetPosition(float &x, float &y) const
{
    x = posX;
    y = posY;
}

/**
 * @brief Sets the absolute rotation angle in radians
 */
void TransformComponent::SetRotation(float angle)
{
    rotation = angle;
}

/**
 * @brief Gets the current rotation angle in radians
 */
float TransformComponent::GetRotation() const
{
    return rotation;
}

/**
 * @brief Sets separate scale factors for X and Y axes
 */
void TransformComponent::SetScale(float x, float y)
{
    scaleX = x;
    scaleY = y;
}

/**
 * @brief Sets uniform scale factor for both axes
 */
void TransformComponent::SetScale(float uniformScale)
{
    scaleX = scaleY = uniformScale;
}

/**
 * @brief Gets the current scale factors for both axes
 */
void TransformComponent::GetScale(float &x, float &y) const
{
    x = scaleX;
    y = scaleY;
}

/**
 * @brief Moves the transform by relative delta values
 */
void TransformComponent::Move(float deltaX, float deltaY)
{
    posX += deltaX;
    posY += deltaY;
}

/**
 * @brief Rotates the transform by a relative delta angle
 */
void TransformComponent::Rotate(float deltaAngle)
{
    rotation += deltaAngle;
}

/**
 * @brief Calculates and sets rotation to face a target position using atan2
 */
void TransformComponent::LookAt(float targetX, float targetY)
{
    float deltaX = targetX - posX;
    float deltaY = targetY - posY;
    rotation = std::atan2(deltaY, deltaX);
}

/**
 * @brief Moves forward in the direction of current rotation
 */
void TransformComponent::MoveForward(float distance)
{
    posX += std::cos(rotation) * distance;
    posY += std::sin(rotation) * distance;
}

/**
 * @brief Moves right perpendicular to current rotation direction
 */
void TransformComponent::MoveRight(float distance)
{
    posX += std::cos(rotation + 1.5708f) * distance; // +90 degrees
    posY += std::sin(rotation + 1.5708f) * distance;
}