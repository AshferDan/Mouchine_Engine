/* Start Header ************************************************************************/
/*!
\file       Physics.cpp
\author     Khalil Belabadia [100% - 80 lines]
\par        khalil.b@digipen.edu
\date       October, 2, 2025
Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*/
/* End Header **************************************************************************/

#include "Physics.h"
#include "Transform.h"
#include "GameObject.h"
#include <iostream>
#include <cmath>

/**
 * @brief Constructs physics component with initial settings
 */
PhysicsComponent::PhysicsComponent(GameObject *owner, float m, bool static_)
    : IComponent(owner),
      velocityX(0), velocityY(0),
      mass(m), isStatic(static_), useGravity(true),
      isGrounded(true), isJumping(false), wasGrounded(true),
      gravityX(0), gravityY(-381.f)
{
}

/**
 * @brief Sets the velocity directly
 */
void PhysicsComponent::SetVelocity(float x, float y)
{
    velocityX = x;
    velocityY = y;
}

/**
 * @brief Creates a deep copy of this physics component
 */
std::unique_ptr<IComponent> PhysicsComponent::Clone(GameObject *NewClone) const
{

    auto clonedComponent = std::make_unique<PhysicsComponent>(
        NewClone,
        this->mass,    // Get mass
        this->isStatic // Get isStatic status
    );

    clonedComponent->velocityX = this->velocityX;
    clonedComponent->velocityY = this->velocityY;
    clonedComponent->useGravity = this->useGravity;
    clonedComponent->gravityX = this->gravityX;
    clonedComponent->gravityY = this->gravityY;

    clonedComponent->SetEnabled(this->IsEnabled());

    return clonedComponent;
}

    /**
     * @brief Gets the current velocity values
     */
    void PhysicsComponent::GetVelocity(float &x, float &y) const
    {
        x = velocityX;
        y = velocityY;
    }

    /**
     * @brief Adds to the current velocity
     */
    void PhysicsComponent::AddVelocity(float x, float y)
    {
        velocityX += x;
        velocityY += y;
    }

    /**
     * @brief Initiates a jump if grounded, sets upward velocity
     */
    void PhysicsComponent::Jump(float jumpForce)
    {
        if (isGrounded)
        {
            velocityY = jumpForce;
            isJumping = true;
            isGrounded = false;

            std::cout << owner->GetName() << " jumped with force " << jumpForce
                      << "! New VelY: " << velocityY << std::endl;
        }
    }

    /**
     * @brief Calculates the speed magnitude from velocity components
     */
    float PhysicsComponent::GetSpeed() const
    {
        return std::sqrt(velocityX * velocityX + velocityY * velocityY);
    }

    /**
     * @brief Calculates the direction angle from velocity components
     */
    float PhysicsComponent::GetDirection() const
    {
        return std::atan2(velocityY, velocityX);
    }

    /**
     * @brief Sets velocity from polar coordinates
     */
    void PhysicsComponent::SetDirection(float angle, float speed)
    {
        velocityX = std::cos(angle) * speed;
        velocityY = std::sin(angle) * speed;
    }

    //**
    // * @brief Applies gravity acceleration to velocity
    // */
    //void PhysicsComponent::ApplyGravity(float deltaTime)
    //{
    //    velocityX += gravityX * deltaTime;
    //    velocityY += gravityY * deltaTime;
    //}

    //**
    // * @brief Updates the transform position based on velocity
    // */
    //void PhysicsComponent::UpdatePosition(float deltaTime)
    //{
    //    auto *transform = owner->GetComponent<TransformComponent>();
    //    if (transform)
    //    {
    //        transform->Move(velocityX * deltaTime, velocityY * deltaTime);
    //    }
    //}