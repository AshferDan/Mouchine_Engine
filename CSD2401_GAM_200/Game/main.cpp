 #include "Core.h"
 #include "Factory.h"
 #include "GameObjectSystem.h"
 #include "GraphicsSystem.h"
 #include "ImGuiSystem.h"
 #include "Transform.h"
 #include "Physics.h"
 #include "RenderComponent.h"
 #include "audio.h"
 #include "ResourceManager.h"
 #include <iostream>
 #include <cmath>

 int main() {
     std::cout << "=== 2D Component System Test ===" << std::endl;

     CoreEngine engine;
     AudioEngine audio;

     // Add the GameObject system

     
     // Initialize
     std::cout << "\n--- Initializing Engine ---" << std::endl;
     engine.Init();
     
     // Initialise Audio System
     std::cout << "\n--- Initialising Audio System ---" << std::endl;
     if (!audio.initialiseAudio()) {
         std::cout << "audio system failed to initialise, continuing without audio" << std::endl;
     }
     else {
         std::cout << "Audio system initialised successfully!" << std::endl;
     }

     // === Initialize ResourceManager ===
     std::cout << "\n--- Initializing Resource Manager ---" << std::endl;
     auto& resourceMgr = ResourceManager::Instance();

     // link audio engine to resource manager for audio loading
     resourceMgr.SetAudioEngine(&audio);

     // load resources
     std::cout << "\n--- Loading Resources ---" << std::endl;

     // load textures
     resourceMgr.LoadTexture("player_sprite", "../Assets/protosprite.png");

     // load audio files
     resourceMgr.LoadAudio("bgm", "../Assets/audio/awesomeness.wav", true); // looping
     // resourceMgr.LoadAudio("wind", "../Assets/Audio/wind1.wav", false); // one-shot

     // print loaded resources
     resourceMgr.PrintLoadedResources();

     // play bgm
     std::cout << "\n--- Starting Background Music ---" << std::endl;
     // register the sound with audio engine to play it
     AudioResource* bgMusic = resourceMgr.GetAudio("bgm");
     if (bgMusic && bgMusic->IsLoaded()) {
         audio.registerSound("bgm", bgMusic->GetSound());

         // Now play it
         audio.playSound("bgm", "bgm_channel");
         audio.setVolume("bgm_channel", 1.0f);
     }
     // Create 2D game objects
     std::cout << "\n--- Creating 2D GameObjects ---" << std::endl;
     auto& factory = GameObjectFactory::Instance();

     GameObject* player = factory.CreatePlayer("Hero",RenderComponent::ShapeType::Circle,0,0, 1, 1,"../Assets/protosprite.png");
     float x, y;
     player->GetComponent<TransformComponent>()->GetPosition(x, y);
     

     //Circle Object -> name, object type (circle), x coord, y coord, width, height (if you want uneven circle), 
     // RGBA color (0 is black , 1 is white, you probably need go online to convert if you want exact color of your liking)
     GameObject* CircleShape = factory.CreateShape("Circle", RenderComponent::ShapeType::Circle, 20, 0, 5, 5, glm::vec4(1.f, 0.f, 0.f, 1.f));

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