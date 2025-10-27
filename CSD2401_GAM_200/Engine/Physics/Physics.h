/* Start Header ************************************************************************/
/*!
\file       Physics.h
\author     Khalil Belabadia [100% - 42 lines]
\par        khalil.b@digipen.edu
\date       October, 2, 2025
Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*/
/* End Header **************************************************************************/

#pragma once
#include "Component.h"
#include <memory>
#include "transform.h"
#include "GameObject.h"
#include <iostream>
#include <cmath>

class TransformComponent; // Forward declaration

class PhysicsComponent : public IComponent {
public:
    /**
     * @brief Constructs a physics component with mass and static properties
     * @param owner GameObject that owns this component
     * @param mass Object mass for physics calculations (default 1.0)
     * @param isStatic Whether object is immovable (default false)
     */
    PhysicsComponent(GameObject* owner, float mass = 1.0f, bool isStatic = false);

    /**
     * @brief Gets the component name
     * @return String identifier for this component type
     */
    std::string GetName() const override { return "PhysicsComponent"; }

    /**
     * @brief Creates a deep copy of this component
     * @param NewClone GameObject that will own the clone
     * @return Unique pointer to the cloned component
     */
    std::unique_ptr<IComponent>Clone(GameObject* NewClone) const override;

    /**
     * @brief Sets the velocity of the object
     * @param x Horizontal velocity
     * @param y Vertical velocity
     */
    void SetVelocity(float x, float y);

    /**
     * @brief Gets the current velocity
     * @param x Output horizontal velocity
     * @param y Output vertical velocity
     */
    void GetVelocity(float& x, float& y) const;

    /**
     * @brief Adds to the current velocity
     * @param x Horizontal velocity to add
     * @param y Vertical velocity to add
     */
    void AddVelocity(float x, float y);

    // ===== Jump System =====
    /**
     * @brief Initiates a jump if the object is grounded
     * @param jumpForce Upward force to apply
     */
    void Jump(float jumpForce);

    /**
     * @brief Sets the grounded state
     * @param grounded Whether object is on ground
     */
    void SetGrounded(bool grounded) { isGrounded = grounded; }

    /**
     * @brief Checks if object is on ground
     * @return True if grounded
     */
    bool IsGrounded() const { return isGrounded; }

    /**
     * @brief Checks if object is falling
     * @return True if moving downward and not grounded
     */
    bool IsFalling() const { return velocityY < 0 && !isGrounded; }

    /**
     * @brief Checks if object is rising
     * @return True if moving upward
     */
    bool IsRising() const { return velocityY > 0; }

    // Properties
    /**
     * @brief Sets the object mass
     * @param m New mass value
     */
    void SetMass(float m) { mass = m; }

    /**
     * @brief Gets the object mass
     * @return Current mass
     */
    float GetMass() const { return mass; }

    /**
     * @brief Sets whether object is static
     * @param static_ Static state
     */
    void SetStatic(bool static_) { isStatic = static_; }

    /**
     * @brief Checks if object is static
     * @return True if static
     */
    bool IsStatic() const { return isStatic; }

    /**
     * @brief Enables or disables gravity
     * @param useGrav Whether to apply gravity
     */
    void SetGravity(bool useGrav) { useGravity = useGrav; }
    /**
 * @brief sets a constant gravity to x or y to the object
 * @param float values of x and y
 */
    void SetGravityVal(float x, float y) {
        gravityX = x;
        gravityY = y;
    }
    // 2D specific helpers
    /**
     * @brief Calculates the speed (magnitude of velocity)
     * @return Current speed
     */
    float GetSpeed() const;

    /**
     * @brief Gets direction of movement
     * @return Direction in radians
     */
    float GetDirection() const;
    /**
     * @brief Sets velocity from angle and speed
     * @param angle Direction in radians
     * @param speed Movement speed
     */
    void SetDirection(float angle, float speed);

private:
    /**
     * @brief Applies gravity to velocity
     * @param deltaTime Frame time
     */
    //void ApplyGravity(float deltaTime);

    /**
     * @brief Updates object position based on velocity
     * @param deltaTime Frame time
     */
   // void UpdatePosition(float deltaTime);

public:
    // Linear motion
    float velocityX, velocityY;

    // Properties
    float mass;               
    bool isStatic;
    bool useGravity;

    // State Flages
    bool isGrounded;
    bool wasGrounded;
    bool isJumping;

    // 2D gravity (can be changed for different effects)
    float gravityX, gravityY;
};