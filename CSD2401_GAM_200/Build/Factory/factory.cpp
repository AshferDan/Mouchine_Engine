#include "factory.h"
 #include "transform.h"
 #include "physics.h"
 #include <iostream>

 GameObject* GameObjectFactory::CreateGameObject(const std::string& name) {
     EntityID id = nextEntityID++;
     auto gameObject = std::make_unique<GameObject>(id, name);
     GameObject* ptr = gameObject.get();

     entityToIndex[id] = gameObjects.size();
     gameObjects.push_back(std::move(gameObject));

     return ptr;
 }

 GameObject* GameObjectFactory::CreatePlayer(const std::string& name, float x, float y, float z) {
     GameObject* player = CreateGameObject(name);

     // Add components (no render component for now)
     player->AddComponent<TransformComponent>(x, y, z);
     player->AddComponent<PhysicsComponent>(70.0f, false);

     std::cout << "Created Player: " << name << " at (" << x << ", " << y << ", " << z << ")" << std::endl;

     return player;
 }

 GameObject* GameObjectFactory::CreateEnemy(const std::string& name, float x, float y, float z) {
     GameObject* enemy = CreateGameObject(name);

     enemy->AddComponent<TransformComponent>(x, y, z);
     enemy->AddComponent<PhysicsComponent>(50.0f, false);

     std::cout << "Created Enemy: " << name << " at (" << x << ", " << y << ", " << z << ")" << std::endl;

     return enemy;
 }

 GameObject* GameObjectFactory::CreateStaticProp(const std::string& name, float x, float y, float z, const std::string& meshName) {
     GameObject* prop = CreateGameObject(name);

     prop->AddComponent<TransformComponent>(x, y, z);
     // No physics component for static props

     std::cout << "Created Static Prop: " << name << " (" << meshName << ") at (" 
               << x << ", " << y << ", " << z << ")" << std::endl;

     return prop;
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