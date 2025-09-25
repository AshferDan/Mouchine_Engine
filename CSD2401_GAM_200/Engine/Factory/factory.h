#pragma once
#include "GameObject.h"
#include <memory>
#include <vector>
#include <unordered_map>
#include <glm/glm.hpp>
#include "RenderComponent.h"

class GameObjectFactory {
public:
    static GameObjectFactory& Instance() {
        static GameObjectFactory instance;
        return instance;
    }

    // Create GameObject
    GameObject* CreateGameObject(const std::string& name = "GameObject");

    // Create predefined 2D types
    GameObject* CreatePlayer(const std::string& name, float x, float y);
    GameObject* CreateEnemy(const std::string& name, float x, float y);
    GameObject* CreateStaticProp(const std::string& name, float x, float y, const std::string& spriteName = "");
    GameObject* CreateProjectile(const std::string& name, float x, float y, float directionAngle, float speed);
    GameObject* CreatePlatform(const std::string& name, float x, float y, float width, float height);
    GameObject* CreateShape(const std::string& name, RenderComponent::ShapeType shape, float x, float y, float w, float h, const glm::vec4& color);

    // Management
    GameObject* GetGameObject(EntityID id);
    GameObject* FindGameObject(const std::string& name);
    const std::vector<std::unique_ptr<GameObject>>& GetAllGameObjects() const;

    void DestroyGameObject(EntityID id);
    void Clear();

private:
    GameObjectFactory() : nextEntityID(1) {}
    EntityID nextEntityID;
    std::vector<std::unique_ptr<GameObject>> gameObjects;
    std::unordered_map<EntityID, size_t> entityToIndex;
};