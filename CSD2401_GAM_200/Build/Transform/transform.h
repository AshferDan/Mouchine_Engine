#pragma once
#include "Component.h"

class TransformComponent : public IComponent {
public:
    TransformComponent(GameObject* owner, float x = 0, float y = 0);

    void Update(float deltaTime) override;
    std::string GetName() const override { return "TransformComponent"; }

    // Position
    void SetPosition(float x, float y);
    void GetPosition(float& x, float& y) const;

    // Rotation (single angle in 2D)
    void SetRotation(float angle);
    float GetRotation() const;

    // Scale
    void SetScale(float x, float y);
    void SetScale(float uniformScale); // Uniform scaling
    void GetScale(float& x, float& y) const;

    // Movement
    void Move(float deltaX, float deltaY);
    void Rotate(float deltaAngle);

    // 2D specific helpers
    void LookAt(float targetX, float targetY);
    void MoveForward(float distance);
    void MoveRight(float distance);

public:
    float posX, posY;        // Position
    float rotation;          // Rotation angle in radians
    float scaleX, scaleY;    // Scale factors
};
