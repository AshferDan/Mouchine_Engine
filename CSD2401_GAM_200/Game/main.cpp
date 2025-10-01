 #include "Core.h"
 #include "Factory.h"
 #include "GameObjectSystem.h"
 #include "GraphicsSystem.h"
 #include "ImGuiSystem.h"
 #include "Transform.h"
 #include "Collision.h"
 #include "Collidercomponent.h"
 #include "Physics.h"
 #include "RenderComponent.h"
 #include <iostream>
 #include <cmath>

 int main() {
     std::cout << "=== 2D Component System Test ===" << std::endl;

     CoreEngine engine;

     // Add the GameObject system
  

     // Initialize
     std::cout << "\n--- Initializing Engine ---" << std::endl;
     engine.Init();

     // Create 2D game objects
     std::cout << "\n--- Creating 2D GameObjects ---" << std::endl;
     auto& factory = GameObjectFactory::Instance();

     GameObject* player = factory.CreatePlayer("Hero",RenderComponent::ShapeType::Square,-7,0, 1, 1,"../Assets/protosprite.png", CollisionLayer::Player);
     float x, y;
     player->GetComponent<TransformComponent>()->GetPosition(x, y);
     auto* playerPhysics = player->GetComponent<PhysicsComponent>();
         playerPhysics->SetGravity(true);  // Enable/Disable gravity
         playerPhysics->SetStatic(false);

     // Create ground at the bottom of the screen (black rectangle)
     GameObject* ground = factory.CreateShape("Ground", RenderComponent::ShapeType::Square,0,-20, 60, 5,glm::vec4(0.0f, 0.0f, 0.0f, 1.0f),CollisionLayer::Ground);
     // Make ground static so it doesn't fall
     auto* groundPhysics = ground->AddComponent<PhysicsComponent>(1.0f, true);
     groundPhysics->SetGravity(false);

     //Circle Object -> name, object type (circle), x coord, y coord, width, height (if you want uneven circle), 
     // RGBA color (0 is black , 1 is white, you probably need go online to convert if you want exact color of your liking)
     GameObject* CircleShape = factory.CreateShape("Circle", RenderComponent::ShapeType::Circle, 5, 0, 2, 2, glm::vec4(1.f, 0.f, 0.f, 1.f),CollisionLayer::Enemy);
     auto* physics1 = CircleShape->AddComponent<PhysicsComponent>(1.0f, false);
     physics1->SetVelocity(-3.0f, 0.0f);  // Moving right
     physics1->SetGravity(true);  // Disable gravity

     GameObject* CircleShape2 = factory.CreateShape("Circle2", RenderComponent::ShapeType::Circle, -5, 0, 2, 2, glm::vec4(1.f, 0.f, 0.f, 1.f), CollisionLayer::Enemy);
     auto* physics2 = CircleShape2->AddComponent<PhysicsComponent>(1.0f, false);
     physics2->SetVelocity(3.0f, 0.0f);  // Moving left
     physics2->SetGravity(true);  // Disable gravity

     //Square Object -> name, object type (square), x coord, y coord, width, height (if you want rectangle), 
     // RGBA color (0 is black , 1 is white, you probably need go online to convert if you want exact color of your liking)
     //GameObject* SquareShape = factory.CreateShape("Square", RenderComponent::ShapeType::Square, 5, 5, 5, 5, glm::vec4(0.f, 0.f, 1.f, 1.f));

     //Line Object -> name, object type (Line), x1, y1 (start points), x2, y2 (end points),
     // RGBA color (0 is black , 1 is white, you probably need go online to convert if you want exact color of your liking)
    // GameObject* Line = factory.CreateShape("Line", RenderComponent::ShapeType::Line, 1, 1, 10, 10, { 0,1,0,1 });

     //Point Object -> name, object type (Point), x coord, y coord, size scaling, 6TH ARGUMENT IS REDANDUNT JUST PUT 0 IT DOES NOT CHANGE ANYTHING, 
     // RGBA color (0 is black , 1 is white, you probably need go online to convert if you want exact color of your liking)
    // GameObject* Point = factory.CreateShape("Point", RenderComponent::ShapeType::Point, 5, 5, 10, 0, { 1,0,0,1 });

     //Start engine main loop
     std::cout << "\n--- Starting Engine Gameloop ---" << std::endl;
     engine.GameLoop();


     return 0;
 }