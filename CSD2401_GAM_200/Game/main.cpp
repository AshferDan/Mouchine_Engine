#include "Core.h"
#include "Factory.h"
#include "GameObjectSystem.h"
#include "GraphicsSystem.h"
#include "ImGuiSystem.h"
#include "DebugSystem.h"
#include "Transform.h"
#include "Collision.h"
#include "Collidercomponent.h"
#include "Physics.h"
#include "RenderComponent.h"
#include <iostream>
#include <cmath>
#include "Animator.hpp"
#include "ResourceManager.h"
#include "audio.h"

int main()
{
    std::cout << "=== 2D Component System Test ===" << std::endl;
    CoreEngine engine;
    AudioEngine audio;
    std::cout << "\n--- Initializing Engine ---" << std::endl;
    engine.Init();

    // Initialise Audio System
    std::cout << "\n--- Initialising Audio System ---" << std::endl;
    if (!audio.initialiseAudio())
    {
        std::cout << "audio system failed to initialise, continuing without audio" << std::endl;
    }
    else
    {
        std::cout << "audio system initialised successfully!" << std::endl;
    }

    // Initialising ResourceManager
    std::cout << "\n--- Initializing Resource Manager ---" << std::endl;
    auto &resourceMgr = ResourceManager::Instance();

    // link audio engine to resource manager for audio loading
    resourceMgr.SetAudioEngine(&audio);
    std::cout << "\n--- Loading Resources ---" << std::endl;

    // load textures
    resourceMgr.LoadTexture("player_sprite", "../Assets/protosprite.png");

    // load audio files
    resourceMgr.LoadAudio("bgm", "../Assets/audio/awesomeness.wav", true); // looping
    // resourceMgr.LoadAudio("wind", "../Assets/Audio/wind1.wav", false); // one-shot

    // print loaded resources
    resourceMgr.PrintLoadedResources();

    // register the sound with audio engine to play it
    std::cout << "\n--- Starting Background Music ---" << std::endl;
    AudioResource *bgMusic = resourceMgr.GetAudio("bgm");
    if (bgMusic && bgMusic->IsLoaded())
    {
        audio.registerSound("bgm", bgMusic->GetSound());
        audio.playSound("bgm", "bgm_channel");
        audio.setVolume("bgm_channel", 1.0f);
    }

    std::cout << "\n--- Creating 2D GameObjects ---" << std::endl;
    auto &factory = GameObjectFactory::Instance();

    GameObject *Background = factory.CreateImage("bgImage", RenderComponent::ShapeType::Square, 0.f, 0.f, 1920.f, 1080.f, "../Assets/wallpaper_transparent.png");
    GameObject *player = factory.CreatePlayer("Hero", RenderComponent::ShapeType::Square, -7, 0, 100, 100, "../Assets/protosprite.png", CollisionLayer::Player);
    float x, y;
    player->GetComponent<TransformComponent>()->GetPosition(x, y);
    auto *playerPhysics = player->GetComponent<PhysicsComponent>();
    playerPhysics->SetGravity(true); // Enable/Disable gravity
    playerPhysics->SetStatic(false);

    // Create ground at the bottom of the screen (black rectangle)
    GameObject *ground = factory.CreateShape("Ground", RenderComponent::ShapeType::Square, 0, -20, 1000, 500, glm::vec4(0.0f, 0.0f, 0.0f, 1.0f), CollisionLayer::Ground);
    // Make ground static so it doesn't fall
    auto *groundPhysics = ground->AddComponent<PhysicsComponent>(1.0f, true);
    groundPhysics->SetGravity(false);

    // Circle Object -> name, object type (circle), x coord, y coord, width, height (if you want uneven circle),
    //  RGBA color (0 is black , 1 is white, you probably need go online to convert if you want exact color of your liking)
    GameObject *CircleShape = factory.CreateShape("Circle", RenderComponent::ShapeType::Circle, 5, 0, 200, 200, glm::vec4(1.f, 0.f, 0.f, 1.f), CollisionLayer::Enemy);
    auto *physics1 = CircleShape->AddComponent<PhysicsComponent>(1.0f, false);
    physics1->SetVelocity(-3.0f, 0.0f); // Moving right
    physics1->SetGravity(true);         // Disable gravity

    GameObject *CircleShape2 = factory.CreateShape("Circle2", RenderComponent::ShapeType::Circle, -5, 0, 200, 200, glm::vec4(1.f, 0.f, 0.f, 1.f), CollisionLayer::Enemy);
    auto *physics2 = CircleShape2->AddComponent<PhysicsComponent>(1.0f, false);
    physics2->SetVelocity(3.0f, 0.0f); // Moving left
    physics2->SetGravity(true);        // Disable gravity


    //GameObject *animatedMouch = factory.CreateAnimatedImage(
    //    "MouchRunner",                 // name
    //    100.0f, 100.0f,                // x, y position
    //    500.0f, 500.0f,                // width, height (scale)
    //    "../Assets/Mouch_Running.png", // filename
    //    1, 2,                          // 1 row, 8 columns
    //    0, 7,                          // start frame 0, end frame 7
    //    9.0f                           // 12 frames per second
    //);

    CORE->GetDebugSystem()->DrawLine({0.f, 0.f}, {960, 540}, {1, 0, 0, 1});
    std::cout << "\n--- Starting Engine Gameloop ---" << std::endl;
    engine.GameLoop();

    return 0;
}