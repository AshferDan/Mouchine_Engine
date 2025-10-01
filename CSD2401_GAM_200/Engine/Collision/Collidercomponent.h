#pragma once
#include "Component.h"
#include "collision.h"   // for AABB, Circle, Vec2
#include "transform.h"

class TransformComponent;

// Collision layers enum
enum class CollisionLayer {
    Player,
    Ground,
    Wall,
    Enemy,
    Platform,
    Default  // For objects without a specific layer
};

class ColliderComponent : public IComponent {
public:
    enum class Type { Square, Circle };

    ColliderComponent(GameObject* owner, Type type, float w, float h = 0.0f,
        CollisionLayer layer = CollisionLayer::Default);

    std::string GetName() const override { return "ColliderComponent"; }

    // Getters for collision shapes
    AABB   GetAABB() const;
    Circle GetCircle() const;

    // Layer management
    CollisionLayer GetLayer() const { return layer; }
    void SetLayer(CollisionLayer newLayer) { layer = newLayer; }

    std::unique_ptr<IComponent> Clone(GameObject* newOwner) const override;
    void Update(float deltaTime) override;

    // Debug info
    Type GetType() const { return type; }

private:
    Type type;
    float width;
    float height;
    float radius;
    CollisionLayer layer;
};