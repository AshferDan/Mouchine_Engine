#include "GameObjectSystem.h"
#include "factory.h"
#include <iostream>

GameObjectSystem::GameObjectSystem() : factory(GameObjectFactory::Instance()) {}

void GameObjectSystem::Initialize() {
    std::cout << "GameObjectSystem initialized" << std::endl;
}

void GameObjectSystem::Update(float timeslice) {
    // Update all GameObjects (which will update their components)
    auto& gameObjects = factory.GetAllGameObjects();

    for (auto& obj : gameObjects) {
        if (obj->IsActive()) {
            obj->Update(timeslice);
        }
    }
}