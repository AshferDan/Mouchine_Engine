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
        "./Assets/wallpaper_transparent.png");

    std::cout << "\n--- Creating Hero ---" << std::endl;
    API::CreateAnimatedImage("Hero", 0.0f, 0.0f, 150.0f, 150.0f,
        "./Assets/Mouch_idle.png",
        1, 2, 0, 7, 2.0f,
        CollisionLayer::Player,
        true, false, true);

    // Maybe some logic here...
    std::cout << "\n--- Creating Other Sprites ---" << std::endl;
    API::CreateAnimatedImage("AniImage", -700.0f, 100.0f, 300.0f, 300.0f,
        "./Assets/Mouch_idle.png",
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

    GameObject* textObj = GameObjectFactory::Instance().CreateGameObject("ScoreText");
    textObj->AddComponent<TransformComponent>(100.0f, 100.0f);  // Position on screen
    auto* text = textObj->AddComponent<TextComponent>(
        "./Assets/FrederickatheGreat-Regular.ttf",
        48,  // Font size
        "Score: 0",
        vec4(1.0f, 1.0f, 1.0f, 1.0f),
        true
    );
    text->SetScale(1.0f);

    // Later, update the text:
    text->SetText("Score: 100");

    GameObject* mainCamera = GameObjectFactory::Instance().CreateGameObject("MainCamera");
    auto* cam = mainCamera->AddComponent<CameraComponent>();

    GameObject* player = GameObjectFactory::Instance().FindGameObject("Hero");
    if (player) {
        cam->SetMode(CameraComponent::CameraMode::FOLLOW_TARGET);
        cam->SetFollowTarget(player);
        cam->SetFollowOffset(vec2(0.f, 0.f));
        cam->SetSmoothing(true, 0.5f);
    }
    CameraManager::Instance().SetActiveCamera(mainCamera);

    //auto* bg = GameObjectFactory::Instance().FindGameObject("bgImage");
    //if (bg && bg->GetComponent<RenderComponent>()) {
    //    bg->GetComponent<RenderComponent>()->SetLayer(-10);
    //}

    // You can even add conditional creation
    bool createButton = true;
    if (createButton)
    {
        //API::CreateGameObject("Evil Button");
    }

    // Start the game loop when ready
    API::StartEngineLoop(engine);

    return 0;
}