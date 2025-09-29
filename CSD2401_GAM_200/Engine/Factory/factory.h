#pragma once
#include "GameObject.h"
#include <memory>
#include <vector>
#include <unordered_map>
#include <glm/glm.hpp>
#include "RenderComponent.h"
#include <string>

class GameObjectFactory {
public:
    static GameObjectFactory& Instance() {
        static GameObjectFactory instance;
        return instance;
    }

    // Create GameObject
    GameObject* CreateGameObject(const std::string& name = "GameObject");

    // Create predefined 2D types
    GameObject* CreatePlayer(const std::string& name, RenderComponent::ShapeType shape, float x, float y , float w, float h,const glm::vec4 color);
    GameObject* CreatePlayer(const std::string& name, RenderComponent::ShapeType shape, float x, float y, float w , float h, const char* sprite);
    GameObject* CreateShape(const std::string& name, RenderComponent::ShapeType shape, float x, float y, float w, float h, const glm::vec4& color);
    GameObject* CloneGameObject(EntityID id, const std::string& newName = "");


    // Management
    GameObject* GetGameObject(EntityID id);
    GameObject* FindGameObject(const std::string& name);
    const std::vector<std::unique_ptr<GameObject>>& GetAllGameObjects() const;
    unsigned int GetCloneCounter() { return cloneCounter; }
    void DestroyGameObject(EntityID id);
    void Clear();

private:
    GameObjectFactory() : nextEntityID(1), cloneCounter(0) {}
    EntityID nextEntityID;
    std::vector<std::unique_ptr<GameObject>> gameObjects;
    std::unordered_map<EntityID, size_t> entityToIndex;
    unsigned int cloneCounter;
};