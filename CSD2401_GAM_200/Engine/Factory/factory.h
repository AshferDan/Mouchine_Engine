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

    // Create predefined 2D types
    GameObject* CreatePlayer(const std::string& name, float x, float y);
    GameObject* CreateEnemy(const std::string& name, float x, float y);
    GameObject* CreateDefault(float x, float y);


    // Management
    GameObject* GetGameObject(GameObjectID);
    GameObject* FindGameObject(const std::string& name);
    const std::vector<std::unique_ptr<GameObject>>& GetAllGameObjects() const;

    void DestroyGameObject(GameObjectID id);
    void Clear();

private:
    GameObjectFactory() : nextGameObjectID(1) {}
    GameObjectID nextGameObjectID;
    std::vector<std::unique_ptr<GameObject>> gameObjects;
    std::unordered_map<GameObjectID, size_t> entityToIndex;
};