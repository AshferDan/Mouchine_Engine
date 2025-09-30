#pragma once
#include "Component.h"
#include <memory>
#include "Transform.h"
#include "GameObject.h"
#include <iostream>
#include <cmath>

class TransformComponent; // Forward declaration

class PhysicsComponent : public IComponent {
public:
    PhysicsComponent(GameObject* owner, float mass = 1.0f, bool isStatic = false);

    void Update(float deltaTime) override;
    std::string GetName() const override { return "PhysicsComponent"; }
    std::unique_ptr<IComponent>Clone(GameObject* NewClone) const override;
    // Velocity
    void SetVelocity(float x, float y);
    void GetVelocity(float& x, float& y) const;
    void AddVelocity(float x, float y);

    // Forces
    void AddForce(float x, float y);
    void AddImpulse(float x, float y); // Instant velocity change

    // Properties
    void SetMass(float m) { mass = m; }
    float GetMass() const { return mass; }
    void SetStatic(bool static_) { isStatic = static_; }
    bool IsStatic() const { return isStatic; }

    // 2D Physics settings
    void SetGravity(bool useGrav) { useGravity = useGrav; }
    void SetDrag(float dragCoeff) { drag = dragCoeff; }
    void SetAngularVelocity(float angVel) { angularVelocity = angVel; }
    void AddTorque(float torque);

    // 2D specific helpers
    float GetSpeed() const;
    float GetDirection() const; // Direction of movement in radians
    void SetDirection(float angle, float speed);

private:
    void ApplyGravity(float deltaTime);
    void ApplyDrag(float deltaTime);
    void UpdatePosition(float deltaTime);
    void UpdateRotation(float deltaTime);

public:
    // Linear motion
    float velocityX, velocityY;
    float forceX, forceY;

    // Angular motion (2D rotation)
    float angularVelocity;
    float torque;

    // Properties
    float mass;
    float drag;                // Air resistance
    bool isStatic;
    bool useGravity;

    // 2D gravity (can be changed for different effects)
    float gravityX, gravityY;
};