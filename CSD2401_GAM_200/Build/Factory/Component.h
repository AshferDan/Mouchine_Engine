#pragma once
#include <string>

// Forward declaration
class GameObject;

// Base Component class - contains both data and behavior
class IComponent {
public:
    IComponent(GameObject* owner) : owner(owner), enabled(true) {}
    virtual ~IComponent() = default;

    // Component lifecycle
    virtual void Initialize() {}
    virtual void Update(float deltaTime) {}
    virtual void Destroy() {}

    // Component info
    virtual std::string GetName() const = 0;

    // Enable/disable component
    void SetEnabled(bool enable) { enabled = enable; }
    bool IsEnabled() const { return enabled; }

protected:
    GameObject* owner;
    bool enabled;
};