#include "Factory.h"
#include "GraphicsSystem.h"
#include "transform.h"
#include "physics.h"
#include <iostream>
#include <cmath>
#include <sstream>

GameObject* GameObjectFactory::CreateGameObject(const std::string& name) {
    EntityID id = nextEntityID++;
    auto gameObject = std::make_unique<GameObject>(id, name);
    GameObject* ptr = gameObject.get();

    entityToIndex[id] = gameObjects.size();
    gameObjects.push_back(std::move(gameObject));

    return ptr;
}

GameObject* GameObjectFactory::CreatePlayer(const std::string& name, RenderComponent::ShapeType shape, float x, float y,float w,float h, const glm::vec4 color) {
    GameObject* player = CreateGameObject(name);

    auto* transform = player->AddComponent<TransformComponent>(x, y);
    auto* physics = player->AddComponent<PhysicsComponent>(0,0);
    auto render = player->AddComponent<RenderComponent>(shape,color);
    transform->SetScale(w, h);
    physics->SetVelocity(0, 0);
    physics->SetGravity(false);
    
    std::cout << "Created Player: " << name << " at (" << x << ", " << y << ")" << std::endl;

    return player;
}

GameObject* GameObjectFactory::CreatePlayer(const std::string& name, RenderComponent::ShapeType shape, float x, float y, float w, float h, const char* filename) {
    GameObject* player = CreateGameObject(name);
    auto* transform = player->AddComponent<TransformComponent>(x, y);
    auto* physics = player->AddComponent<PhysicsComponent>(30, 0);
    GLuint texID = GraphicsSystem::LoadTexture(filename);
    auto render = player->AddComponent<RenderComponent>(texID);
    transform->SetScale(w, h);
    physics->SetVelocity(0, 0);
    physics->SetGravity(false);

    std::cout << "Created Player: " << name << " at (" << x << ", " << y << ")" << std::endl;

    return player;
}

GameObject* GameObjectFactory::CreateShape(const std::string& name, RenderComponent::ShapeType shape, float x, float y, float w, float h, const glm::vec4& color) {
    GameObject* obj = CreateGameObject(name);

    auto* transform = obj->AddComponent<TransformComponent>(x, y);

    auto render = obj->AddComponent<RenderComponent>(shape, color);

    if (shape == RenderComponent::ShapeType::Line) {
        render->SetLine(glm::vec2(x, y), glm::vec2(w, h));
    }
    else if (shape == RenderComponent::ShapeType::Point) {
        render->SetPoint(glm::vec2(x, y), w);
    }
    else {
        transform->SetScale(w, h);
    }
    //testing TO BE REMOVED
    std::cout << "Created "
        << (shape == RenderComponent::ShapeType::Circle ? "Circle" :
            shape == RenderComponent::ShapeType::Square ? "Square" :
            shape == RenderComponent::ShapeType::Line ? "Line" : "Point")
        << ": " << name << " at (" << x << "," << y << ")\n";

    return obj;
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
