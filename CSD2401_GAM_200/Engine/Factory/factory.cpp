#include "Factory.h"
#include "transform.h"
#include "physics.h"
#include "RenderComponent.h"
#include <iostream>
#include <cmath>
GameObject* GameObjectFactory::CreateGameObject(const std::string& name) {
    GameObjectID id = nextGameObjectID++;
    auto gameObject = std::make_unique<GameObject>(id, name);
    GameObject* ptr = gameObject.get();

    entityToIndex[id] = gameObjects.size();
    gameObjects.push_back(std::move(gameObject));

    return ptr;
}
GameObject* GameObjectFactory::CreateDefault(float x, float y) {
    GameObject* prefab = CreateGameObject("Default");

    prefab->AddComponent<TransformComponent>(x, y);
    prefab->AddComponent<TransformComponent>()->SetScale(5.0f,5.0f);
    std::cout << "created prefab" << prefab->GetName() << " at (" << x << ", " << y << ")" << std::endl;
    return prefab;

}
GameObject* GameObjectFactory::CreatePlayer(const std::string& name, float x, float y) {
    GameObject* player = CreateGameObject(name);

    // Add components for 2D player
    player->AddComponent<TransformComponent>(x, y);
    auto* physics = player->AddComponent<PhysicsComponent>(70.0f, false);
    physics->SetDrag(0.9f); // Player has more control, less slide

    std::cout << "Created Player: " << name << " at (" << x << ", " << y << ")" << std::endl;

    return player;
}

GameObject* GameObjectFactory::CreateEnemy(const std::string& name, float x, float y) {
    GameObject* enemy = CreateGameObject(name);

    enemy->AddComponent<TransformComponent>(x, y);
    auto* physics = enemy->AddComponent<PhysicsComponent>(50.0f, false);
    physics->SetDrag(0.95f);

    std::cout << "Created Enemy: " << name << " at (" << x << ", " << y << ")" << std::endl;

    return enemy;
}



GameObject* GameObjectFactory::GetGameObject(GameObjectID id) {
    auto it = entityToIndex.find(id);
    if (it != entityToIndex.end() && it->second < gameObjects.size()) {
        return gameObjects[it->second].get();
    }
    return nullptr;
}

GameObject* GameObjectFactory::FindGameObject(const std::string& name) {
    for (auto& obj : gameObjects) {
        if (obj->GetName() == name) {
            return obj.get();
        }
    }
    return nullptr;
}

const std::vector<std::unique_ptr<GameObject>>& GameObjectFactory::GetAllGameObjects() const {
    return gameObjects;
}

void GameObjectFactory::DestroyGameObject(GameObjectID id) {
    auto it = entityToIndex.find(id);
    if (it != entityToIndex.end()) {
        size_t index = it->second;

        if (index < gameObjects.size() - 1) {
            std::swap(gameObjects[index], gameObjects.back());
            entityToIndex[gameObjects[index]->GetID()] = index;
        }

        gameObjects.pop_back();
        entityToIndex.erase(it);
    }
}

void GameObjectFactory::Clear() {
    gameObjects.clear();
    entityToIndex.clear();
    nextGameObjectID = 1;
}
