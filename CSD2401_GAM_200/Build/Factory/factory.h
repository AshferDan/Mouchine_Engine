#pragma once
#include "GameObject.h"
#include <memory>
#include <vector>
#include <unordered_map>

class GameObjectFactory {
public:
    static GameObjectFactory& Instance() {
        static GameObjectFactory instance;
        return instance;
    }

    // Create GameObject
    GameObject* CreateGameObject(const std::string& name = "GameObject");

    // Create predefined types
    GameObject* CreatePlayer(const std::string& name, float x, float y, float z);
    GameObject* CreateEnemy(const std::string& name, float x, float y, float z);
    GameObject* CreateStaticProp(const std::string& name, float x, float y, float z, const std::string& meshName);

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
