
#include "GameObject.h"
#include <iostream>

GameObject::GameObject(GameObjectID id, const std::string& name) 
    :GObjID(id), name(name), isActive(true) {
}

GameObject::~GameObject() {
    Destroy();
}

void GameObject::Initialize() {
    for (auto& [type, component] : components) {
        component->Initialize();
    }
}

void GameObject::Update(float deltaTime) {
    if (!isActive) return;

    for (auto& [type, component] : components) {
        if (component->IsEnabled()) {
            component->Update(deltaTime);
        }
    }
}

void GameObject::Destroy() {
    for (auto& [type, component] : components) {
        component->Destroy();
    }
    components.clear();
}