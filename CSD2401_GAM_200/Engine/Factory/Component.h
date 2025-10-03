/* Start Header ************************************************************************/
/*!
\file       Component.h
\author     Jonathan Fam Qian Wei [100% - 19 lines]
\par        Fam.q@digipen.edu
\date       October, 2, 2025

Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*/
/* End Header **************************************************************************/

/*==================================================================================
/Brief
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
#include <memory>


class GameObject;

// Base Component class - contains both data and behavior
class IComponent {
public:
    /**
     * @brief Constructs the base component.
     * @param owner GameObject that owns this component.
     */
    IComponent(GameObject* owner) : owner(owner), enabled(true) {}

    /**
     * @brief Default virtual destructor for proper cleanup of derived component classes.
     */
    virtual ~IComponent() = default;


    /**
     * @brief Initializes the component's state. Called once upon component creation.
     */
    virtual void Initialize() {}

    /**
     * @brief Called every game frame to update the component's logic.
     * @param deltaTime The time elapsed since the last frame.
     */
    virtual void Update(float deltaTime) {}

    /**
     * @brief Cleans up the component's resources. Called once before component deletion.
     */
    virtual void Destroy() {}

    // Component info
    /**
     * @brief Gets the name of the component.
     * @return A string representing the component's name.
     */
    virtual std::string GetName() const = 0;

    //cloning virtual
    /**
     * @brief Creates a deep copy of the component for a new GameObject clone.
     * @param NewClone The GameObject that will own the new component.
     * @return A unique pointer to the cloned component.
     */
    virtual std::unique_ptr<IComponent>Clone(GameObject* NewClone) const = 0;

    // Enable/disable component
    /**
     * @brief Sets the enabled state of the component.
     * @param enable If true, the component is enabled; otherwise, it's disabled.
     */
    void SetEnabled(bool enable) { enabled = enable; }

    /**
     * @brief Checks if the component is currently enabled.
     * @return true if the component is enabled, false otherwise.
     */
    bool IsEnabled() const { return enabled; }

protected:
    GameObject* owner;
    bool enabled;
};