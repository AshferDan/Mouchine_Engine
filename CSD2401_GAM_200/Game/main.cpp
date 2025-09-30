 #include "Core.h"
 #include "Factory.h"
 #include "GameObjectSystem.h"
 #include "GraphicsSystem.h"
 #include "ImGuiSystem.h"
 #include "Transform.h"
 #include "Physics.h"
 #include "RenderComponent.h"
 #include <iostream>
 #include <cmath>
#include "Animator.hpp"

 int main() {
     std::cout << "=== 2D Component System Test ===" << std::endl;
     CoreEngine engine;
     std::cout << "\n--- Initializing Engine ---" << std::endl;
     engine.Init();



     std::cout << "\n--- Creating 2D GameObjects ---" << std::endl;
     auto& factory = GameObjectFactory::Instance();


     GameObject* Background = factory.CreateImage("bgImage", RenderComponent::ShapeType::Square, 0.f, 0.f, 1920.f, 1080.f, "../Assets/wallpaper_transparent.png");
       GameObject* player = factory.CreatePlayer("Hero", RenderComponent::ShapeType::Circle, 0.f, 0.f, 100.f, 100.f, "../Assets/protosprite.png");
       
       GameObject* animatedMouch = factory.CreateAnimatedImage(
           "MouchRunner",           // name
           100.0f, 100.0f,          // x, y position
           500.0f, 500.0f,            // width, height (scale)
           "../Assets/Mouch_Running.png", // filename
           1, 2,                    // 1 row, 8 columns
           0, 7,                    // start frame 0, end frame 7
           9.0f                    // 12 frames per second
       );

     //Circle Object -> name, object type (circle), x coord, y coord, width, height (if you want uneven circle), 
     // RGBA color (0 is black , 1 is white, you probably need go online to convert if you want exact color of your liking)
     GameObject* CircleShape = factory.CreateShape("Circle", RenderComponent::ShapeType::Circle, 20.f, 0.f, 5.f, 5.f, glm::vec4(1.f, 0.f, 0.f, 1.f));

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