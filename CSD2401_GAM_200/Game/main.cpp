 #include "Core.h"
 #include "Factory.h"
 #include "GameObjectSystem.h"
 #include "GraphicsSystem.h"
 #include "Transform.h"
 #include "Physics.h"
 #include "RenderComponent.h"
 #include <iostream>
 #include <cmath>

 int main() {
     std::cout << "=== 2D Component System Test ===" << std::endl;

     CoreEngine engine;
     
     // Initialize
     std::cout << "\n--- Initializing Engine ---" << std::endl;
     engine.Init();

     // Create 2D game objects
     std::cout << "\n--- Creating 2D GameObjects ---" << std::endl;
     auto& factory = GameObjectFactory::Instance();

     GLuint heroTex = GraphicsSystem::LoadTexture("../Assets/protosprite.png");
     if (heroTex == 0) {
         std::cerr << "Failed to load hero texture!\n";
     }
     for (int i = 0; i <= 1; i++) {
         GameObject* prefab = factory.CreateDefault(5, 7);
         prefab->AddComponent<RenderComponent>(heroTex);
     }

     //// Stop physics from moving them
     //if (auto* phys = player->GetComponent<PhysicsComponent>())
     //    phys->SetGravity(false);

     //// Scale for visibility
     //player->GetComponent<TransformComponent>()->SetScale(10.0f, 10.0f);
     //enemy->GetComponent<TransformComponent>()->SetScale(1.0f, 1.0f);

    //player->AddComponent<RenderComponent>(heroTex);
     //enemy->AddComponent<RenderComponent>(heroTex);
     

     //Start engine main loop
     std::cout << "\n--- Starting Engine Gameloop ---" << std::endl;
     engine.GameLoop();


     return 0;
 }