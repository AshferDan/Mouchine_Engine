#pragma once
#include "Component.h"
#include <vector>
#include <memory>
#include <typeindex>
#include <unordered_map>
#include <string>

using EntityID = std::uint32_t;

class GameObject {
public:
    GameObject(EntityID id, const std::string& name = "GameObject");
    ~GameObject();

    // Component management
    template<typename T, typename... Args>
    T* AddComponent(Args&&... args) {
        // Check if component already exists
        if (HasComponent<T>()) {
            return GetComponent<T>();
        }

        auto component = std::make_unique<T>(this, std::forward<Args>(args)...);
        T* componentPtr = component.get();

        std::type_index typeIndex(typeid(T));
        components[typeIndex] = std::move(component);

        // Initialize the component
        componentPtr->Initialize();

        return componentPtr;
    }

    template<typename T>
    T* GetComponent() {
        std::type_index typeIndex(typeid(T));
        auto it = components.find(typeIndex);
        if (it != components.end()) {
            return static_cast<T*>(it->second.get());
        }
        return nullptr;
    }

    template<typename T>
    bool HasComponent() {
        std::type_index typeIndex(typeid(T));
        return components.find(typeIndex) != components.end();
    }

    template<typename T>
    void RemoveComponent() {
        std::type_index typeIndex(typeid(T));
        auto it = components.find(typeIndex);
        if (it != components.end()) {
            it->second->Destroy();
            components.erase(it);
        }
    }

    // GameObject lifecycle
    void Initialize();
    void Update(float deltaTime);
    void Destroy();

    // Getters
    EntityID GetID() const { return entityID; }
    const std::string& GetName() const { return name; }

    // Enable/disable
    void SetActive(bool active) { isActive = active; }
    bool IsActive() const { return isActive; }

private:
    EntityID entityID;
    std::string name;
    bool isActive;
    std::unordered_map<std::type_index, std::unique_ptr<IComponent>> components;
};