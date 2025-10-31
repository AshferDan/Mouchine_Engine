/* Start Header ************************************************************************/
/*!
\file       main.cpp
\author     Lee Hwee Min [100% - 95 lines]
\par        l.hweemin@digipen.edu
\date       October, 2, 2025
Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*/
/* End Header **************************************************************************/

#include "main.h"

/**
 * @brief Enables memory leak checking in debug mode.
 * @param breakAlloc, Allocation number to break on for debugging.
 */
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

/**
 * @brief Main entry point of the program.
 *
 * Initializes the engine, resource manager, audio, and game objects.
 * Loads textures, audio files, sets up the scene, then enters the main loop.
 *
 * @return Program exit code (0 = success).
 */
int main()
{
    EnableMemoryLeakChecking();
    CoreEngine engine;
    AudioEngine audio;

    API::Init_Engine(engine, audio);

    // Create objects whenever you want, in any order
    std::cout << "\n--- Creating Background ---" << std::endl;
    API::CreateImage("bgImage", RenderComponent::ShapeType::Square,
        0.f, 0.f, 2400.f, 1380.f,
        "../Assets/wallpaper_transparent.png");

    std::cout << "\n--- Creating Hero ---" << std::endl;
    API::CreateAnimatedImage("Hero", 0.0f, 0.0f, 150.0f, 150.0f,
        "../Assets/Mouch_idle.png",
        1, 2, 0, 7, 2.0f,
        CollisionLayer::Player,
        true, false, true);

    // Maybe some logic here...
    std::cout << "\n--- Creating Other Sprites ---" << std::endl;
    API::CreateAnimatedImage("AniImage", -700.0f, 100.0f, 300.0f, 300.0f,
        "../Assets/Mouch_idle.png",
        1, 2, 0, 7, 2.0f, CollisionLayer::Ground, true, true, false);

    // Create ground
    std::cout << "\n--- Creating Environment ---" << std::endl;
    API::CreateShape("Ground", RenderComponent::ShapeType::Square,
        0, -500, 3000, 500,
        vec4(0.0f, 0.0f, 0.0f, 1.0f),
        CollisionLayer::Ground,
        true, true, false);

    API::CreateShape("Wall", RenderComponent::ShapeType::Square,
        1500, -500, 1000, 3000,
        vec4(0.0f, 0.0f, 0.0f, 1.0f),
        CollisionLayer::Ground,
        true, true, false);

    API::CreateShape("Wall2", RenderComponent::ShapeType::Square,
        -1500, -500, 1000, 3000,
        vec4(0.0f, 0.0f, 0.0f, 1.0f),
        CollisionLayer::Ground,
        true, true, false);

    // Create enemies
    std::cout << "\n--- Creating Enemies ---" << std::endl;
    API::CreateShape("Circle", RenderComponent::ShapeType::Circle,
        150, 300, 100, 100,
        vec4(1.f, 0.f, 0.f, 1.f),
        CollisionLayer::Enemy,
        true, false, true);
    API::SetVelocity("Circle", -250.0f, 0.0f);

    API::CreateShape("Circle2", RenderComponent::ShapeType::Circle,
        -150, 300, 100, 100,
        vec4(1.f, 0.f, 0.f, 1.f),
        CollisionLayer::Enemy,
        true, false, true);
    API::SetVelocity("Circle2", 250.0f, 0.0f);

    // You can even add conditional creation
    bool createButton = true;
    if (createButton)
    {
        //API::CreateGameObject("Evil Button");
    }

    // =======================================================
    // C# SCRIPTING - TWO SQUARES SPAWNER
    // =======================================================
    std::cout << "\n--- Attaching C# Scripts ---" << std::endl;

    ScriptSystem* scriptSys = engine.GetScriptSystem();

    // ===== ORIGINAL: Spawner script attached to Hero =====
    GameObject* hero = GameObjectFactory::Instance().FindGameObject("Hero");
    if (scriptSys && hero)
    {
        scriptSys->AttachScript(hero, "Spawner");
        std::cout << "Attached 'Spawner' script to Hero" << std::endl;
    }
    else
    {
        if (!scriptSys)
            std::cerr << "ERROR: Could not find ScriptingSystem!" << std::endl;
        if (!hero)
            std::cerr << "ERROR: Could not find 'Hero' GameObject!" << std::endl;
    }

    // ===== NEW: TestGameObject - Creates 2 static squares =====
    // Use the factory to create the GameObject properly
    GameObject* squareSpawner = GameObjectFactory::Instance().CreateGameObject("SquareSpawner");
    if (scriptSys && squareSpawner)
    {
        // Attach the TestGameObject script (creates red and blue squares)
        bool attached = scriptSys->AttachScript(squareSpawner, "TestGameObject");
        if (attached)
        {
            std::cout << "Attached 'TestGameObject' script to SquareSpawner" << std::endl;

            // Call OnStart to actually create the squares
            scriptSys->CallOnStart(squareSpawner);
            std::cout << "Called OnStart - Two squares should be created!" << std::endl;
        }
        else
        {
            std::cerr << "ERROR: Failed to attach 'TestGameObject' script!" << std::endl;
            std::cerr << "Make sure GameScripts.dll is in ./Scripts/ folder" << std::endl;
            std::cerr << "And that TestGameObject class exists in the DLL" << std::endl;
        }
    }
    else
    {
        if (!scriptSys)
            std::cerr << "ERROR: ScriptingSystem is null!" << std::endl;
        if (!squareSpawner)
            std::cerr << "ERROR: Could not create SquareSpawner GameObject!" << std::endl;
    }

    // ===== OPTIONAL: AnimatedSquares - Creates 2 animated squares =====
    // Uncomment this section to also spawn animated squares
    /*
    GameObject* animatedSpawner = new GameObject("AnimatedSpawner");
    if (scriptSys && animatedSpawner)
    {
        bool attached = scriptSys->AttachScript(animatedSpawner, "AnimatedSquares");
        if (attached)
        {
            std::cout << "Attached 'AnimatedSquares' script to AnimatedSpawner" << std::endl;
            scriptSys->CallOnStart(animatedSpawner);
            std::cout << "Called OnStart - Two animated squares should be created!" << std::endl;
        }
    }
    */

    // =======================================================

    // Start the game loop when ready
    API::StartEngineLoop(engine);

    return 0;
}