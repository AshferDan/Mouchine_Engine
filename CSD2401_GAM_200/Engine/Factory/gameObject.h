/*====================================================================
Jonathan
im just putting my thought here for referance, 

here is where the gameobjects are being stored where the each gameobject
will have a entityID, name , and active-ness and as well as the componment array

was looking around ecs for inspiraion but with about little entities we going to have
so i decided to use the traditional Component system where we have a
core and factory.

as we know the component have many different systems and data and we 
and we need to add componment and remove accordingly, went to look thru the 
 and as well as checking the gameobject what does it has

as i was reseaching i saw the the unqineness of unordermaps and unique_ptr and how was it used
with abit of AI i was able to see create the templates below
for the unordered maps was becuase of its for faster lookup might look into std::maps when 
i need things to be ordered. i feel that i could have used vector but the find() is useful since i have the 
GameObjects named and the components easier.

As for the unique ptrs during my research most of the ecs have these ptrs looking, i realise it useful for creating these system to prevent
memory leaks and auto the parameters and since each gameobject has their own unique typeID and component array.

as for the template that has args... or basically Variadic Templates since different system have different parameters that they take in of course
i didnt know about only after using abit of AI to look at examples


======================================================================*/


#pragma once
#include "Component.h"
#include <vector>
#include <memory>
#include <typeindex>
#include <unordered_map>
#include <string>

using GameObjectID = std::uint32_t;

class GameObject {
public:
    GameObject(GameObjectID id, const std::string& name = "GameObject");
    ~GameObject();

    // Component management
     /*======================================================================================

    ========================================================================================*/
    template<typename T, typename... Args>
    T* AddComponent(Args&&... args) {
        // Check if component already exists
        if (HasComponent<T>()) {
            return GetComponent<T>();
        }
        //if not then with the new created object add a component or more
        //args... is a way to get the variable number of contstrutor arguments
        //with std::forward it helps to forward  the other args to the component construtor
        auto component = std::make_unique<T>(this, std::forward<Args>(args)...);
        T* componentPtr = component.get();

        //std::type index is to get each of the unique ID for each component type
        std::type_index typeIndex(typeid(T));
        components[typeIndex] = std::move(component);

        // Initialize the component
        componentPtr->Initialize();

        return componentPtr;
    }
    /*======================================================================================
    
    ========================================================================================*/
    template<typename T>
    T* GetComponent() {
        std::type_index typeIndex(typeid(T));
        auto it = components.find(typeIndex);
        if (it != components.end()) {
            return static_cast<T*>(it->second.get());
        }
        return nullptr;
    }

    /*======================================================================================

   ========================================================================================*/
    template<typename T>
    bool HasComponent() {
        std::type_index typeIndex(typeid(T));
        return components.find(typeIndex) != components.end();
    }
    /*======================================================================================

   ========================================================================================*/

    template<typename T>
    void RemoveComponent() {
        std::type_index typeIndex(typeid(T));
        auto it = components.find(typeIndex);
        if (it != components.end()) {
            it->second->Destroy();
            components.erase(it);
        }
    }
    /*======================================================================================

   ========================================================================================*/
    // GameObject lifecycle
    void Initialize();
    void Update(float deltaTime);
    void Destroy();

    // Getters
    GameObjectID GetID() const { return GObjID; }
    const std::string& GetName() const { return name; }

    // Enable/disable
    void SetActive(bool active) { isActive = active; }
    bool IsActive() const { return isActive; }

private:
    GameObjectID GObjID;
    std::string name;
    bool isActive;
    std::unordered_map<std::type_index, std::unique_ptr<IComponent>> components;
};