/* Start Header ************************************************************************/
/*!
\file       main.cpp
\author     Lee Hwee Min [100% - 137 lines]
\par        l.hweemin@digipen.edu
\date       October, 2, 2025
Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*/
/* End Header **************************************************************************/

#include "main.h"
PickupSystem pickupSys;

namespace CrashHandler
{
    std::string GetStackTrace();
    void LogCrash(const char *type, const std::exception *e = nullptr);
}

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
    try
    {

        EnableMemoryLeakChecking();
        AudioEngine audio;
        CoreEngine engine;

        API::Init_Engine(engine, audio);

        // ── Register game-specific systems & callbacks ────────────────────────
        // These systems are compiled in the Game project, not the engine.
        // They are registered here so the engine can update and notify them
        // through the I
        // interface without depending on their types.

        struct GameApp : public IGameCallbacksSystem
        {
            MenuLogicSystem*      menuLogic      = nullptr;
            PauseMenuLogicSystem* pauseMenuLogic = nullptr;
            WinLoseLogicSystem*   winLoseLogic   = nullptr;
            GameTimerSystem*      gameTimer      = nullptr;
            PickupSystem          pickupSystem;

            void FixedUpdatePlaying(float dt) override {
                if (CORE->GetGhostSystem()->IsReplayMode()) return;
                if (menuLogic) menuLogic->Update(dt);
            }
            void FixedUpdateAlways(float dt) override {
                if (CORE->GetGhostSystem()->IsReplayMode()) return;
                if (pauseMenuLogic) pauseMenuLogic->Update(dt);
                if (winLoseLogic)   winLoseLogic->Update(dt);
            }
            void VariableUpdate(float dt) override {
                if (CORE->GetGhostSystem()->IsReplayMode()) return;
                if (gameTimer) gameTimer->Update(dt);
                GameManager::Instance().GameStateLoop(dt);
            }
            void OnEditorStop() override {
                if (menuLogic) menuLogic->Reset();
            }
            void OnLevelLoaded(const std::string& path) override {
                if (pauseMenuLogic) pauseMenuLogic->SetCurrentScene(path);
                if (winLoseLogic)   winLoseLogic->SetCurrentScene(path);
                pickupSystem = PickupSystem();
            }
            void OnSceneLoadReinit() override {
                if (pauseMenuLogic) pauseMenuLogic->Initialize();
                if (gameTimer)      gameTimer->Initialize();
                if (winLoseLogic)   winLoseLogic->Initialize();
                pickupSystem = PickupSystem();
            }
            bool IsPauseMenuActive()  const override { return pauseMenuLogic && pauseMenuLogic->IsPauseMenuActive(); }
            bool IsWinScreenActive()  const override { return winLoseLogic   && winLoseLogic->IsWinScreenActive(); }
            bool IsLoseScreenActive() const override { return winLoseLogic   && winLoseLogic->IsLoseScreenActive(); }
            bool IsHowToPlayOpen()    const override { return pauseMenuLogic && pauseMenuLogic->IsHowToPlayOpen(); }
            bool IsPauseSettingsOpen() const override { return pauseMenuLogic && pauseMenuLogic->IsPauseSettingsOpen(); }
            bool IsGhostMenuOpen()    const override { return pauseMenuLogic && pauseMenuLogic->IsGhostMenuOpen(); }
            bool IsOnMainMenu()       const override {
                return pauseMenuLogic &&
                       pauseMenuLogic->GetCurrentScene().find("MainMenu") != std::string::npos;
            }
            std::string GetCurrentScenePath() const override {
                return pauseMenuLogic ? pauseMenuLogic->GetCurrentScene() : "";
            }
            void TogglePauseMenu()     override { if (pauseMenuLogic) pauseMenuLogic->TogglePauseMenu(); }
            void OpenHowToPlay()       override { if (pauseMenuLogic) pauseMenuLogic->OpenHowToPlay(); }
            void CloseHowToPlay()      override { if (pauseMenuLogic) pauseMenuLogic->CloseHowToPlay(); }
            void ClosePauseSettings()  override { if (pauseMenuLogic) pauseMenuLogic->ClosePauseSettings(); }
            void CloseGhostMenu()      override { if (pauseMenuLogic) pauseMenuLogic->CloseGhostMenu(); }
            void TriggerLose()         override { if (winLoseLogic)   winLoseLogic->TriggerLose(); }

            void CollectCoin()          override { pickupSystem.CollectCoin(); }
            void CollectShell()         override { pickupSystem.CollectShell(); }
            int  GetCurrentCoins()  const override { return pickupSystem.GetCurrentCoins(); }
            int  GetCurrentShells() const override { return pickupSystem.GetCurrentShells(); }
            int  GetTotalCoins()    const override { return pickupSystem.GetTotalCoins(); }
            int  GetTotalShells()   const override { return pickupSystem.GetTotalShells(); }
            void ReinitPickups()        override { pickupSystem = PickupSystem(); }

            float GetLevelTimerLength() const override { return gameTimer ? gameTimer->GetLevelTimerLength() : 60.0f; }
            void  SetLevelTimerLength(float length) override { if (gameTimer) gameTimer->SetLevelTimerLength(length); }
        };

        static GameApp gameApp;
        gameApp.menuLogic      = CORE->AddSystem<MenuLogicSystem>();
        gameApp.pauseMenuLogic = CORE->AddSystem<PauseMenuLogicSystem>();
        gameApp.winLoseLogic   = CORE->AddSystem<WinLoseLogicSystem>();
        gameApp.gameTimer      = CORE->AddSystem<GameTimerSystem>();

        gameApp.menuLogic->Initialize();
        gameApp.pauseMenuLogic->Initialize();
        gameApp.winLoseLogic->Initialize();
        gameApp.gameTimer->Initialize();

        CORE->SetGameCallbacks(&gameApp);
        // ─────────────────────────────────────────────────────────────────────

        // CORE->RequestSceneLoad("Prefab_TestScene");
#ifndef _DEBUG
        CORE->RequestSceneLoad("../Assets/Scenes/MainMenu.json", true);
		CORE->GetGraphicsSystem()->SetEditorMode(false);
#else
        GameObject *mainCamera = GameObjectFactory::Instance().CreateGameObject("MainCamera");
        auto *cam = mainCamera->AddComponent<CameraComponent>();

        GameObject *player = GameObjectFactory::Instance().FindGameObject("Hero");
        if (player)
        {
            cam->SetMode(CameraComponent::CameraMode::FOLLOW_TARGET);
            // WHEN CAMERA MODE IS FIXED
            // cam->SetPosition(0.f, 0.f);
            cam->SetFollowTarget(player);
            cam->SetFollowOffset(vec2(0.f, 0.f));
            cam->SetSmoothing(true, 2.f);
            cam->SetZoom(0.5f);         
        }

        CameraManager::Instance().SetActiveCamera(mainCamera);


        GameObject* Hero = GameObjectFactory::Instance().CreateAnimatedImage(
            "Hero",
            0, 0,
            192, 192,
            "../Assets/Mouch_idle.png", 1, 2, 0, 7, 2.0f, CollisionLayer::Player);

        if (Hero)
        {
            ColliderComponent* colComp = Hero->AddComponent<ColliderComponent>();
            PhysicsComponent* phyComp = Hero->AddComponent<PhysicsComponent>();
            //TransformComponent* TransComp = Hero->GetComponent<TransformComponent>();

            if (colComp && phyComp)
            {
                colComp->SetSize(168.0f, 192.0f);
            }

            ScriptComponent* scriptComp = Hero->AddComponent<ScriptComponent>();

            if (scriptComp)
            {
                scriptComp->AddScript("PlayerMovement");
                scriptComp->AddScript("PlayerMovementStats");
                scriptComp->AddScript("PickupManager");
                

                std::cout << "[Main] Created 'PlayerMovement' and attached 'Hero_cs'.\n";
            }
            // --- Ghost Player Setup ---
            // Load the best recording saved from any previous run (no-op if no file exists yet)
            CORE->GetGhostSystem()->LoadRecording("DebugLevel");

            // Start recording the current run using the Hero object
            CORE->GetGhostSystem()->StartRecording("Hero");

            // If a previous ghost recording was loaded, clone Hero as the ghost object.
            // The clone inherits Hero's Spritesheet and RenderComponent automatically —
            // no manual animation setup needed. Physics and collision are stripped so
            // the ghost is purely visual and passes through walls.
            if (CORE->GetGhostSystem()->IsGhostAvailable())
            {
                GameObject* GhostHero = GameObjectFactory::Instance().CloneGameObject(
                    Hero->GetID(), "GhostHero");

                if (GhostHero)
                {
                    GhostHero->RemoveComponent<ColliderComponent>();
                    GhostHero->RemoveComponent<PhysicsComponent>();
                    GhostHero->RemoveComponent<ScriptComponent>();

                    // Clone the VFX companion so ReplayPlayer can drive it
                    GameObject* HeroVFX = GameObjectFactory::Instance().FindGameObject("Hero_vfx");
                    if (HeroVFX)
                    {
                        GameObject* GhostVFX = GameObjectFactory::Instance().CloneGameObject(
                            HeroVFX->GetID(), "GhostHero_vfx");
                        if (GhostVFX)
                            GhostVFX->RemoveComponent<ScriptComponent>();
                    }

                    CORE->GetGhostSystem()->StartPlayback("GhostHero");
                }
            }
        }

        GameObject* Platform = GameObjectFactory::Instance().CreateShape(
            "Ground",                  // <--- NAME MUST MATCH C#
            RenderComponent::ShapeType::Square,
            0.0f, -200.0f,                      // Center of screen
            100000.0f, 200.0f,                  // Size
            false,
            vec4(1.0f, 1.0f, 1.0f, 0.6f)     // White (Script will change this to Yellow)
        );

        if (Platform)
        {
            ColliderComponent* colComp = Platform->AddComponent<ColliderComponent>();
            PhysicsComponent* phyComp = Platform->AddComponent<PhysicsComponent>();

            if (colComp && phyComp)
            {
                phyComp->SetStatic(true);
                colComp->SetSize(100000.0f, 200.0f);
                colComp->SetLayer(CollisionLayer::Ground);
            }
        }

        GameObject* Wall = GameObjectFactory::Instance().CreateShape(
            "Platform",                  // <--- NAME MUST MATCH C#
            RenderComponent::ShapeType::Square,
            -200.0f, 0.0f,                      // Center of screen
            100.0f, 1000.0f,                  // Size
            false,
            vec4(1.0f, 1.0f, 1.0f, 0.6f)     // White (Script will change this to Yellow)
        );

        if (Wall)
        {
            ColliderComponent* colComp = Wall->AddComponent<ColliderComponent>();
            PhysicsComponent* phyComp = Wall->AddComponent<PhysicsComponent>();

            if (colComp && phyComp)
            {
                phyComp->SetStatic(true);
                colComp->SetSize(100.0f, 1000.0f);
                colComp->SetLayer(CollisionLayer::Ground);
            }
        }


        GameObject* Wall2 = GameObjectFactory::Instance().CreateShape(
            "NPC_Lobster",                  // <--- NAME MUST MATCH C#
            RenderComponent::ShapeType::Square,
            200.0f, 0.0f,                      // Center of screen
            100.0f, 1000.0f,                  // Size
            false,
            vec4(1.0f, 1.0f, 1.0f, 0.6f)     // White (Script will change this to Yellow)
        );

        if (Wall2)
        {
            ColliderComponent* colComp = Wall2->AddComponent<ColliderComponent>();
            PhysicsComponent* phyComp = Wall2->AddComponent<PhysicsComponent>();

            if (colComp && phyComp)
            {
                phyComp->SetStatic(true);
                colComp->SetSize(100.0f, 1000.0f);
                colComp->SetLayer(CollisionLayer::Ground);
            }
        }

#endif
        // Initialise Game State Manager (Level1)
        GameManager::Instance().ChangeState(std::make_unique<Level1>());

        API::StartEngineLoop(engine);
    }
    catch (const std::exception &e)
    {
        std::cerr << "[TEST] Exception caught: " << e.what() << "\n";
        CrashHandler::LogCrash("EXCEPTION", &e);
    }
    catch (...)
    {
        CrashHandler::LogCrash("UNKNOWN EXCEPTION");
    }
    return 0;
}

namespace CrashHandler
{

#if defined(_WIN32)
#include <windows.h>
#include <dbghelp.h>
#pragma comment(lib, "dbghelp.lib")
#endif
    /**
     * @brief Captures and returns the current stack trace as a formatted string.
     * @return A string containing the call stack (Windows only).
     */
    std::string GetStackTrace()
    {
#if defined(_WIN32)
        void *stack[62];
        HANDLE process = GetCurrentProcess();
        SymInitialize(process, NULL, TRUE);
        WORD frames = CaptureStackBackTrace(0, 62, stack, NULL);

        std::ostringstream oss;
        for (WORD i = 0; i < frames; ++i)
        {
            DWORD64 address = (DWORD64)(stack[i]);
            char buffer[sizeof(SYMBOL_INFO) + MAX_SYM_NAME * sizeof(TCHAR)];
            PSYMBOL_INFO symbol = (PSYMBOL_INFO)buffer;
            symbol->SizeOfStruct = sizeof(SYMBOL_INFO);
            symbol->MaxNameLen = MAX_SYM_NAME;

            DWORD64 displacement = 0;
            if (SymFromAddr(process, address, &displacement, symbol))
            {
                oss << "[" << i << "] " << symbol->Name
                    << " - 0x" << std::hex << symbol->Address << std::dec << "\n";
            }
        }
        return oss.str();
#else
        return "Stacktrace not supported on this platform.";
#endif
    }
    /**
     * @brief Logs crash details and stack trace to a file.
     * @param type Type or category of the crash (e.g., "EXCEPTION").
     * @param e Optional exception pointer for additional error details.
     */
    void LogCrash(const char *type, const std::exception *e)
    {
        std::filesystem::create_directories("../Logs");
        std::ofstream log("../Logs/CrashLog.txt", std::ios::app);
        auto now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
        log << "\n[" << type << "] " << std::put_time(std::localtime(&now), "%F %T") << "\n";
        if (e)
            log << e->what() << "\n";
        log << GetStackTrace() << "\n";
        log.close();
    }
} // namespace CrashHandler
