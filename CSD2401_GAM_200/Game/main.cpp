#include "main.h"

void EnableMemoryLeakChecking(int breakAlloc = -1)
{
    // Set the leak checking flag
    int tmpDbgFlag = _CrtSetDbgFlag(_CRTDBG_REPORT_FLAG);
    tmpDbgFlag |= _CRTDBG_LEAK_CHECK_DF;
    _CrtSetDbgFlag(tmpDbgFlag);

    // If a valid break alloc provided set the breakAlloc
    if (breakAlloc != -1)
        _CrtSetBreakAlloc(breakAlloc);
}


int main()
{
    EnableMemoryLeakChecking();
    std::cout << "=== 2D Component System Test ===" << std::endl;
    CoreEngine engine;
    AudioEngine audio;
    engine.SetAudioEngine(&audio);
    double g_totalLoopTime = 0.0;
    std::map<std::string, double> g_systemTimings; // ms per system

    std::cout << "\n--- Initializing Engine ---" << std::endl;
    engine.Init();

     // Initialise Audio System
     std::cout << "\n--- Initialising Audio System ---" << std::endl;
     if (!audio.initialiseAudio())
     {
         std::cout << "Audio system failed to initialise, continuing without audio" << std::endl;
     }
     else
     {
         std::cout << "Audio system initialised successfully" << std::endl;
     }

     // Initialise ResourceManager
     std::cout << "\n--- Initialising Resource Manager ---" << std::endl;
     auto& resourceMgr = ResourceManager::Instance();
     resourceMgr.SetAudioEngine(&audio);

     // load textures
     resourceMgr.LoadTexture("player_sprite", "../Assets/protosprite.png");

     // load audio files
     resourceMgr.LoadAudio("bgm", "../Assets/audio/level_1.wav", true); // looping
     resourceMgr.LoadAudio("jump_sfx", "../Assets/audio/jump.wav", false); // when jumping only
     // resourceMgr.LoadAudio("wind", "../Assets/Audio/wind1.wav", false); // one-shot

     // print loaded resources
     resourceMgr.PrintLoadedResources();

     // register the sound with audio engine
     std::cout << "\n--- Starting Background Music ---" << std::endl;
     audio.playSound("bgm", "bgm_channel"); 
     audio.setVolume("bgm_channel", 0.5f);

    std::cout << "\n--- Creating 2D GameObjects ---" << std::endl;
    auto &factory = GameObjectFactory::Instance();

    GameObject *Background = factory.CreateImage("bgImage", RenderComponent::ShapeType::Square, 0.f, 0.f, 2400.f, 1380.f, "../Assets/wallpaper_transparent.png");
    
    GameObject* animatedMouch = factory.CreateAnimatedImage(
        "Hero",                 // name
        0.0f, 0.0f,                // x, y position
        300.0f, 300.0f,                // width, height (scale)
        "../Assets/Mouch_idle.png", // filename
        1, 2,                          //row,columns
        0, 7,                          // start frame , end frame
        2.0f,                          // 12 frames per second
        CollisionLayer::Player                           
    );
    auto* playerPhysics = animatedMouch->AddComponent<PhysicsComponent>(1.0f, true);
    playerPhysics->SetGravity(true);
    playerPhysics->SetStatic(false);

    GameObject* animatedMouchidle = factory.CreateAnimatedImage(
        "AniImage",                 // name
        -700.0f, 100.0f,                // x, y position
        300.0f, 300.0f,                // width, height (scale)
        "../Assets/Mouch_idle.png",     // filename
        1, 2,                          //row,columns
        0, 7,                          // start frame , end frame
        2.0f                         // 12 frames per second
    );

    GameObject* animatedMouchRun = factory.CreateAnimatedImage(
        "AniImage",                 // name
        700.0f, 100.0f,                // x, y position
        300.0f, 300.0f,                // width, height (scale)
        "../Assets/Mouch_Running.png", // filename
        1, 2,                          //row,columns
        0, 7,                          // start frame , end frame
        2.0f                          // 12 frames per second
    );

    // Create ground at the bottom of the screen (black rectangle)
    GameObject *ground = factory.CreateShape("Ground", RenderComponent::ShapeType::Square, 0, -500, 3000, 500, glm::vec4(0.0f, 0.0f, 0.0f, 1.0f), CollisionLayer::Ground);
    // Make ground static so it doesn't fall
    auto *groundPhysics = ground->AddComponent<PhysicsComponent>(1.0f, true);
    groundPhysics->SetGravity(false);

    // Create ground at the bottom of the screen (black rectangle)
    GameObject *wall = factory.CreateShape("Wall", RenderComponent::ShapeType::Square, 1500, -500, 1000, 3000, glm::vec4(0.0f, 0.0f, 0.0f, 1.0f), CollisionLayer::Ground);
    auto* wallPhysics = wall->AddComponent<PhysicsComponent>(1.0f, true);
    wallPhysics->SetGravity(false);

    GameObject* wall2 = factory.CreateShape("Wall2", RenderComponent::ShapeType::Square, -1500, -500, 1000, 3000, glm::vec4(0.0f, 0.0f, 0.0f, 1.0f), CollisionLayer::Ground);

    // Make ground static so it doesn't fall
    auto *wall2Physics = wall2->AddComponent<PhysicsComponent>(1.0f, true);
    wall2Physics->SetGravity(false);

    // Circle Object -> name, object type (circle), x coord, y coord, width, height (if you want uneven circle),
    //  RGBA color (0 is black , 1 is white, you probably need go online to convert if you want exact color of your liking)
    GameObject *CircleShape = factory.CreateShape("Circle", RenderComponent::ShapeType::Circle, 150, 300, 100, 100, glm::vec4(1.f, 0.f, 0.f, 1.f), CollisionLayer::Enemy);
    auto *physics1 = CircleShape->AddComponent<PhysicsComponent>(1.0f, false);
    physics1->SetVelocity(-250.0f, 0.0f); // Moving right
    physics1->SetGravity(true);           // Disable gravity

    
 
    GameObject *CircleShape2 = factory.CreateShape("Circle2", RenderComponent::ShapeType::Circle, -150, 300, 100, 100, glm::vec4(1.f, 0.f, 0.f, 1.f), CollisionLayer::Enemy);
    auto *physics2 = CircleShape2->AddComponent<PhysicsComponent>(1.0f, false);
    physics2->SetVelocity(250.0f, 0.0f); // Moving left
    physics2->SetGravity(true);          // Disable gravity
    // Button that has the 2.5k GOs spawning/ despawn
    GameObject* EvilButton = factory.CreateGameObject("Evil Button");


    std::cout << "\n--- Starting Engine Gameloop ---" << std::endl;
    engine.GameLoop();

    return 0;
}