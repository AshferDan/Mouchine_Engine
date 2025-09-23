/*==================================================================================
Jonathan
Brief
this file is the blueprint for all components where it has the class icomponent
with this file it gives all gameobject where the component are being store 
must haves like the component must haves
-Initialize();
-Update();
-Destroy();
-GetName() - the component must have a name
-enable and disable 
===================================================================================*/

#pragma once
#include <string>


class GameObject;

// Base Component class - contains both data and behavior
class IComponent {
public:
    //Set up the component so to start one is to use any of them
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