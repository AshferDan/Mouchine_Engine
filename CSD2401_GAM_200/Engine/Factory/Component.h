#pragma once
#include <string>

// Forward declaration
class GameObject;

// Base Component class - contains both data and behavior
class IComponent {
public:
    //Set up the component so to start one is to use any of them
    //its setting the gameobject to active.

    //GameObject* player = factory.CreatePlayer("Name_of_GameObject", x, y);
    //GameObject* Enemies = factorry.CreateEnemies("Name_of_Enemies",x,y);
    //GameObject* StaticProps = factory.CreateEnemies("Name_of_props,x,y);
    IComponent(GameObject* owner) : owner(owner), enabled(true) {}
    virtual ~IComponent() = default;

    // Component lifecycle
    // all of the virtual down here is where all the functions each object should have when called 
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