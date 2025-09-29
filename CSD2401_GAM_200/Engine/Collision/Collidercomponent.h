#pragma once
#include "Component.h"
#include "collision.h"   // for AABB, Circle, Vec2
#include "transform.h"

class TransformComponent;

class ColliderComponent : public IComponent {
public:
    enum class Type { Square, Circle };

    ColliderComponent(GameObject* owner, Type type, float w, float h = 0.0f);

    std::string GetName() const override { return "ColliderComponent"; }

    // Getters for collision shapes
    AABB   GetAABB() const;
    Circle GetCircle() const;

    // Debug info
    Type GetType() const { return type; }

private:
    Type type;
    float width;
    float height;
    float radius;
};
