#include "Factory.h"
#include "transform.h"
#include "physics.h"
#include <iostream>
#include <cmath>

GameObject* GameObjectFactory::CreateGameObject(const std::string& name) {
    EntityID id = nextEntityID++;
    auto gameObject = std::make_unique<GameObject>(id, name);
    GameObject* ptr = gameObject.get();

    entityToIndex[id] = gameObjects.size();
    gameObjects.push_back(std::move(gameObject));

    return ptr;
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

GameObject* GameObjectFactory::CreateStaticProp(const std::string& name, float x, float y, const std::string& spriteName) {
    GameObject* prop = CreateGameObject(name);

    prop->AddComponent<TransformComponent>(x, y);
    // No physics component for static props

    std::cout << "Created Static Prop: " << name;
    if (!spriteName.empty()) {
        std::cout << " (" << spriteName << ")";
    }
    std::cout << " at (" << x << ", " << y << ")" << std::endl;

    return prop;
}

GameObject* GameObjectFactory::CreateProjectile(const std::string& name, float x, float y, float directionAngle, float speed) {
    GameObject* projectile = CreateGameObject(name);

    auto* transform = projectile->AddComponent<TransformComponent>(x, y);
    transform->SetRotation(directionAngle);

    auto* physics = projectile->AddComponent<PhysicsComponent>(1.0f, false);
    physics->SetDirection(directionAngle, speed);
    physics->SetGravity(false); // Projectiles often don't use gravity
    physics->SetDrag(0.99f); // Minimal drag

    std::cout << "Created Projectile: " << name << " at (" << x << ", " << y 
              << ") moving at angle " << directionAngle << " with speed " << speed << std::endl;

    return projectile;
}

GameObject* GameObjectFactory::CreatePlatform(const std::string& name, float x, float y, float width, float height) {
    GameObject* platform = CreateGameObject(name);

    auto* transform = platform->AddComponent<TransformComponent>(x, y);
    transform->SetScale(width, height); // Use scale to represent size

    // Platforms are static
    std::cout << "Created Platform: " << name << " at (" << x << ", " << y 
              << ") size (" << width << "x" << height << ")" << std::endl;

    return platform;
}

GameObject* GameObjectFactory::GetGameObject(EntityID id) {
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

void GameObjectFactory::DestroyGameObject(EntityID id) {
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
    nextEntityID = 1;
}
