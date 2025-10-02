 #include "GameObjectSystem.h"
 #include "Factory.h"
 #include <iostream>

 GameObjectSystem::GameObjectSystem() : factory(GameObjectFactory::Instance()) {}

 void GameObjectSystem::Initialize() {
     std::cout << "GameObjectSystem initialized" << std::endl;
 }

 void GameObjectSystem::Update(float timeslice) {
     //// Update all GameObjects (which will update their components)
     auto& gameObjects = factory.GetAllGameObjects();

     for (auto& obj_unique_ptr : gameObjects) {
         GameObject* obj = obj_unique_ptr.get(); // Get the raw pointer

         // This check confirms if the unique_ptr became null unexpectedly
         if (obj == nullptr) {
             std::cerr << "ERROR: Null GameObject pointer found in factory list!" << std::endl;
             continue; // Skip the corrupted object
         }

         if (obj->IsActive()) { // Now it's safe to call IsActive()
             obj->Update(timeslice);
         }
     }
 }