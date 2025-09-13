#pragma once
#include "Component.h"


class PhysicsComponent : public IComponent {
public:
    PhysicsComponent(GameObject* owner, float mass = 1.0f, bool isStatic = false);

    void Update(float deltaTime) override;
    std::string GetName() const override { return "PhysicsComponent"; }

    void SetVelocity(float x, float y, float z);
    void AddForce(float x, float y, float z);
    void SetMass(float m) { mass = m; }
    void SetStatic(bool static_) { isStatic = static_; }

private:
    void ApplyGravity(float deltaTime);
    void UpdatePosition(float deltaTime);

public:
    float velocityX, velocityY, velocityZ;
    float forceX, forceY, forceZ;
    float mass;
    bool isStatic;
    bool useGravity;
};