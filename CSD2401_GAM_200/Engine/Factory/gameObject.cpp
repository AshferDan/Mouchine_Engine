#include "GameObject.h"
#include "factory.h"
#include <iostream>
#include <sstream>

GameObject::GameObject(EntityID id, const std::string& name)
    : entityID(id), name(name), isActive(true) {
}

GameObject* GameObject::Clone(const std::string& newName) const
{
    GameObjectFactory& factory = GameObjectFactory::Instance();

    std::string name = !newName.empty() ? (this->name + " (Clone)") : newName;
    std::cout << name;
    GameObject* clone = factory.CreateGameObject(name);
    for (const auto& componentPair : components) {
        // Get the values of the componments first then clone the components into the new cloned gameobject.
        // the unordered map is the <std::type_index, std::unique_ptr<IComponent>>; so will be the second one.
        //                                 ^^First         ^^second
        //as for the std::move is because the type_index is a unique_ptr so need to move a new copy to the cloneComponent;
        const std::unique_ptr<IComponent>& ogComponent = componentPair.second;
        std::unique_ptr<IComponent> cloneComponent = ogComponent->Clone(clone);
        clone->components[componentPair.first] = std::move(cloneComponent);
    }
    clone->isActive = this->isActive;
    clone->Initialize();
    return clone;
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