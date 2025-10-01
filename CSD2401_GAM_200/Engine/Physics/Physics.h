#pragma once
#include "Component.h"

class TransformComponent; // Forward declaration

class PhysicsComponent : public IComponent {
public:
    PhysicsComponent(GameObject* owner, float mass = 1.0f, bool isStatic = false);

    std::string GetName() const override { return "PhysicsComponent"; }

    // Velocity
    void SetVelocity(float x, float y);
    void GetVelocity(float& x, float& y) const;
    void AddVelocity(float x, float y);

    // ===== Jump System =====
    void Jump(float jumpForce);
    void SetGrounded(bool grounded) { isGrounded = grounded; }
    bool IsGrounded() const { return isGrounded; }
    bool IsFalling() const { return velocityY < 0 && !isGrounded; }
    bool IsRising() const { return velocityY > 0; }

    // Properties
    void SetMass(float m) { mass = m; }
    float GetMass() const { return mass; }
    void SetStatic(bool static_) { isStatic = static_; }
    bool IsStatic() const { return isStatic; }

    // 2D Physics settings
    void SetGravity(bool useGrav) { useGravity = useGrav; }

    // 2D specific helpers
    float GetSpeed() const;
    float GetDirection() const; // Direction of movement in radians
    void SetDirection(float angle, float speed);

private:
    void ApplyGravity(float deltaTime);
    void UpdatePosition(float deltaTime);

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