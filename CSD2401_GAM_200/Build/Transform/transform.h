#pragma once
#include "Component.h"

class TransformComponent : public IComponent {
public:
    TransformComponent(GameObject* owner, float x = 0, float y = 0, float z = 0);

    void Update(float deltaTime) override;
    std::string GetName() const override { return "TransformComponent"; }

    // Position
    void SetPosition(float x, float y, float z);
    void GetPosition(float& x, float& y, float& z) const;

    // Rotation
    void SetRotation(float x, float y, float z);
    void GetRotation(float& x, float& y, float& z) const;

    // Scale
    void SetScale(float x, float y, float z);
    void GetScale(float& x, float& y, float& z) const;

    // Movement
    void Move(float deltaX, float deltaY, float deltaZ);
    void Rotate(float deltaX, float deltaY, float deltaZ);

public:
    float posX, posY, posZ;
    float rotX, rotY, rotZ;
    float scaleX, scaleY, scaleZ;
};