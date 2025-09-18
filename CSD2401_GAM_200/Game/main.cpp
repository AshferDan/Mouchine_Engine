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

     // Add the GameObject system
     engine.AddSystem<GameObjectSystem>();
     engine.AddSystem<GraphicsSystem>();

     // Initialize
     std::cout << "\n--- Initializing Engine ---" << std::endl;
     engine.Init();

     // Create 2D game objects
     std::cout << "\n--- Creating 2D GameObjects ---" << std::endl;
     auto& factory = GameObjectFactory::Instance();

     GameObject* player = factory.CreatePlayer("Hero", 0, 0);
     GameObject* enemy = factory.CreateEnemy("Goblin", 3, 0);
     GameObject* platform = factory.CreatePlatform("Ground", 0, -3, 6, 1);
     GameObject* bullet = factory.CreateProjectile("Bullet", -2, 1, 0.785f, 0); // velocity 0

     // Stop physics from moving them
     if (auto* phys = player->GetComponent<PhysicsComponent>())
         phys->SetGravity(true);

     // Scale for visibility
     player->GetComponent<TransformComponent>()->SetScale(10.0f, 10.0f);
     enemy->GetComponent<TransformComponent>()->SetScale(1.0f, 1.0f);
     platform->GetComponent<TransformComponent>()->SetScale(30.0f, 5.0f);
     bullet->GetComponent<TransformComponent>()->SetScale(0.5f, 0.5f);

     GLuint heroTex = GraphicsSystem::LoadTexture("../Assets/protosprite.png");
     if (heroTex == 0) {
         std::cerr << "Failed to load hero texture!\n";
     }
     player->AddComponent<RenderComponent>(heroTex);
     enemy->AddComponent<RenderComponent>(heroTex);
     platform->AddComponent<RenderComponent>(heroTex);
     bullet->AddComponent<RenderComponent>(heroTex);

     //Start engine main loop
     std::cout << "\n--- Starting Engine Gameloop ---" << std::endl;
     engine.GameLoop();


     return 0;
 }