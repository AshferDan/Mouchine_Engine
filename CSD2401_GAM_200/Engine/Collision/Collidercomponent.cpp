#include "ColliderComponent.h"
#include "GameObject.h"

ColliderComponent::ColliderComponent(GameObject* owner, Type type, float w, float h, CollisionLayer layer)
    : IComponent(owner), type(type), width(w), height(h), radius(w * 0.5f), layer(layer) {
}

// Convert Transform position to AABB
AABB ColliderComponent::GetAABB() const {
    auto* transform = owner->GetComponent<TransformComponent>();
    if (!transform) return { {0,0}, {0,0} };

    Vec2 pos{ transform->posX, transform->posY };
    Vec2 half{ width * 0.5f, height * 0.5f };

    return { {pos.x - half.x, pos.y - half.y}, {pos.x + half.x, pos.y + half.y} };
}



// Convert Transform position to Circle
Circle ColliderComponent::GetCircle() const {
    auto* transform = owner->GetComponent<TransformComponent>();
    if (!transform) return { {0,0}, radius };

    Vec2 pos{ transform->posX, transform->posY };
    return { pos, radius };
}

std::unique_ptr<IComponent> ColliderComponent::Clone(GameObject* newOwner) const {
    auto cloned = std::make_unique<ColliderComponent>(
        newOwner,
        this->type,
        this->width,
        this->height,
        this->layer
    );
    cloned->SetEnabled(this->IsEnabled());
    return cloned;
}

void ColliderComponent::Update(float) {
    // Colliders typically don't need per-frame updates
    // They're queried by the collision system
    // Leave empty or add any specific update logic you need
}