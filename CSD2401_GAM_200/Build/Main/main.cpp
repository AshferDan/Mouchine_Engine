 #include "Core.h"
 #include "Factory.h"
 #include "GameObjectSystem.h"
 #include "Transform.h"
 #include "Physics.h"
 #include <iostream>
 #include <cmath>

 int main() {
     std::cout << "=== 2D Component System Test ===" << std::endl;

     CoreEngine engine;

     // Add the GameObject system
     engine.AddSystem<GameObjectSystem>();

     // Initialize
     std::cout << "\n--- Initializing Engine ---" << std::endl;
     engine.Init();

     // Create 2D game objects
     std::cout << "\n--- Creating 2D GameObjects ---" << std::endl;
     auto& factory = GameObjectFactory::Instance();

     GameObject* player = factory.CreatePlayer("Hero", 0, 5);
     // Test 2D component interactions
     std::cout << "\n--- Testing 2D Component Interactions ---" << std::endl;
     // Test transform methods
     if (auto* transform = player->GetComponent<TransformComponent>()) {
         transform->SetRotation(0.785f); // 45 degrees
         std::cout << "Set player rotation to 45 degrees" << std::endl;
     }

     std::cout << "\n--- Starting 2D Simulation (8 frames) ---" << std::endl;

     // Simulate frames to see 2D physics in action
     float deltaTime = 0.016f; // ~60 FPS
     for (int frame = 1; frame <= 8; ++frame) {
         std::cout << "\n--- Frame " << frame << " ---" << std::endl;

         // Update all systems
         auto& gameObjects = factory.GetAllGameObjects();
         for (auto& obj : gameObjects) {
             if (obj->IsActive()) {
                 obj->Update(deltaTime);
             }
         }
     }

     std::cout << "\n=== 2D Test Complete ===" << std::endl;

     return 0;
 }