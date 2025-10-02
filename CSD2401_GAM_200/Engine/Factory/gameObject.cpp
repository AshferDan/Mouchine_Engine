/* Start Header ************************************************************************/
/*!
\file       gameObject.cpp
\author     Jonathan Fam Qian Wei [100% - 42 lines]
\par        Fam.q@digipen.edu
\date       October, 2, 2025

Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*/
/* End Header **************************************************************************/

#include "GameObject.h"
#include "factory.h"
#include <iostream>
#include <sstream>


GameObject::GameObject(EntityID id, const std::string& name)
    : entityID(id), name(name), isActive(true) {
}

GameObject* GameObject::Clone(const std::string& newName) const {
    GameObjectFactory& factory = GameObjectFactory::Instance();

    // Ensure we use the unique name passed by the factory, with a fallback
    std::string finalName = newName.empty() ? (this->name + " (FallbackClone)") : newName;

    // 1. Create the new GameObject using the unique name
    GameObject* clone = factory.CreateGameObject(finalName);

    // 2. Iterate over all components and clone them
    for (const auto& pair : components) {
        const std::unique_ptr<IComponent>& originalComponent = pair.second;

        // Call the component's virtual Clone method
        std::unique_ptr<IComponent> clonedComponent = originalComponent->Clone(clone);

        // Move the cloned component into the new GameObject's map
        clone->components[pair.first] = std::move(clonedComponent);
    }

    // 3. Copy state and initialize
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