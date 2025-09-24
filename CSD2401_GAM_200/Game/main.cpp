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

     GameObject* player = factory.CreatePlayer("Hero", 0, 0);
     GameObject* enemy = factory.CreateEnemy("Goblin", 3, 0);
     GameObject* platform = factory.CreatePlatform("Ground", 0, -3, 6, 1);
     GameObject* bullet = factory.CreateProjectile("Bullet", -2, 1, 0.785f, 0); // velocity 0
     //Circle Object -> name, object type (circle), x coord, y coord, width, height (if you want uneven circle), 
     // RGBA color (0 is black , 1 is white, you probably need go online to convert if you want exact color of your liking)
     GameObject* CircleShape = factory.CreateShape("Circle", RenderComponent::ShapeType::Circle, 1, 1, 10, 10, glm::vec4(1.f, 0.f, 0.f, 1.f));

     //Square Object -> name, object type (square), x coord, y coord, width, height (if you want rectangle), 
     // RGBA color (0 is black , 1 is white, you probably need go online to convert if you want exact color of your liking)
     GameObject* SquareShape = factory.CreateShape("Square", RenderComponent::ShapeType::Square, 5, 5, 5, 5, glm::vec4(0.f, 0.f, 1.f, 1.f));

     //Line Object -> name, object type (Line), x1, y1 (start points), x2, y2 (end points),
     // RGBA color (0 is black , 1 is white, you probably need go online to convert if you want exact color of your liking)
     GameObject* Line = factory.CreateShape("Line", RenderComponent::ShapeType::Line, 1, 1, 10, 10, { 0,1,0,1 });

     //Point Object -> name, object type (Point), x coord, y coord, size scaling, 6TH ARGUMENT IS REDANDUNT JUST PUT 0 IT DOES NOT CHANGE ANYTHING, 
     // RGBA color (0 is black , 1 is white, you probably need go online to convert if you want exact color of your liking)
     GameObject* Point = factory.CreateShape("Point", RenderComponent::ShapeType::Point, 5, 5, 10, 0, { 1,0,0,1 });

     // Stop physics from moving them
     if (auto* phys = player->GetComponent<PhysicsComponent>())
         phys->SetGravity(false);

     // Scale for visibility
     player->GetComponent<TransformComponent>()->SetScale(10.0f, 10.0f);
     enemy->GetComponent<TransformComponent>()->SetScale(1.0f, 1.0f);
     platform->GetComponent<TransformComponent>()->SetScale(20.0f, 5.0f);
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