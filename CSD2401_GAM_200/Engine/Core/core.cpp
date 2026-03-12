/* Start Header ************************************************************************/
/*!
\file       Core.cpp
\author     Jonathan Fam Qian Wei [70% - 249 lines]
\co-author  Lee Hwee Min [30% - 45 lines]
\par        Fam.q@digipen.edu , l.hweemin@digipen.edu
\date       October, 2, 2025

Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*/
/* End Header **************************************************************************/

#include "core.h"
#include "LevelEditor.h"
#include "Message.h"
#include "IMessageHandler.h"
#include "logic.h"
#include "factory.h"
#include "GameCallbacks.h"

bool InputManager::KeyStateQuit = false;
CoreEngine *CORE;

double CoreEngine::fps = 0.0;
double CoreEngine::delta_time = 0.0;
double CoreEngine::fixed_dt = 1.0 / 60.0;
double g_totalLoopTime = 0.0;
std::map<std::string, double> g_systemTimings;

/* brief
   Constuctor

   */
CoreEngine::CoreEngine() : WindWidth(0), WindHeight(0), accumulator(0.0), stepMode(false), stepOnce(false)
{
    LastTime = 0;
    GameActive = true;
    CORE = this;
    std::cout << "[CoreEngine::CoreEngine] CORE set to " << this << std::endl;

    std::cout << "CoreEngine constructed" << std::endl;
}
/* brief
   deonstuctor
   */
CoreEngine::~CoreEngine()
{
    DestroySystems();
    std::cout << "CoreEngine destroyed" << std::endl;
}

/* brief
   function loads the config.txt file from the folder  directory and loads the
   window width and height to the graphics system

   */
void CoreEngine::ConfigFile(const char *configF)
{
    std::ifstream fp{configF};
    if (!fp)
    {
        std::cout << "Cannot find file" << std::endl;
        glfwTerminate();
    }

    int WindWid = 0, WindHei = 0;
    std::string temp;
    fp >> temp >> WindWid;
    fp >> temp >> WindHei;
    setWindsize(WindWid, WindHei);
}

/* brief
   This function init the systems and add the systems into the enigne

   */
void CoreEngine::InitSystems()
{
    CoreEngine::ConfigFile("../Logs/config.txt");
    std::cout << "Initializing default systems..." << std::endl;

    // 1. Graphics first (needed for window)
    graphicsSystem = AddSystem<GraphicsSystem>(WindWidth, WindHeight);

    // 2. ImGui (needs graphics window)
    imguiSystem = AddSystem<ImGuiSystem>(graphicsSystem->getWindow());

    // 3. Input Manager - MUST BE EARLY to capture input before physics updates
    inputManager = AddSystem<InputManager>(graphicsSystem->getWindow(), audioEngine);
    debugSystem = AddSystem<DebugSystem>();

    // 4. GameObject System
    gameObjectSystem = AddSystem<GameObjectSystem>();

    // Transform System
    transformSystem = AddSystem<TransformSystem>();

    // 5. Collision System - detects collisions
    collisionSystem = AddSystem<CollisionSystem>();

    // 6. Physics System - processes collisions and applies physics
    physicsSystem = AddSystem<PhysicsSystem>();

    // 7. Camera System - Controls the cameras
    cameraSystem = AddSystem<CameraSystem>();

    // 8. CutsceneSystem - Play/Stop videos
    cutsceneSystem = AddSystem<CutsceneSystem>();

    // 9. Level Editor - for editing levels
    levelEditor = AddSystem<LevelEditor>();

    // 10. Connect CollisionSystem to PhysicsSystem
    collisionSystem->SetPhysicsSystem(physicsSystem);

    // 11. ScriptSystem - C# Script System
    scriptSystem = AddSystem<ScriptSystem>();

    timerSystem      = AddSystem<TimerSystem>();
    ghostSystem      = AddSystem<GhostSystem>();
    textInputSystem  = AddSystem<TextInputSystem>();

    // Connect GraphicsSystem to LevelEditor for editor camera
    graphicsSystem->SetLevelEditor(levelEditor);

    // Subscribe systems to specific message types
    messageBus.Subscribe<PlaySoundMessage>(audioEngine);
    messageBus.Subscribe<PauseSoundMessage>(audioEngine);
    messageBus.Subscribe<CollisionMessage>(physicsSystem);
    messageBus.Subscribe<CreateObjectMessage>(gameObjectSystem);

    std::cout << "Default systems initialized: GameObjectSystem, GraphicsSystem" << std::endl;
}
/* brief
   This function init the engine and add the systems into the enigne and as well as initialze the added systems.

   */

void CoreEngine::Init()
{
    InitSystems();
    if (audioEngine)
    {
        std::cout << "[CoreEngine] Creating AudioManager internally..." << std::endl;
        // We use 'new' here because we store it in a raw pointer member variable.
        // We MUST delete this in DestroySystems() to avoid leaks.
        audioManager = new AudioManager(audioEngine);
    }
    else
    {
        std::cerr << "[CoreEngine] WARNING: AudioEngine not set before Init! AudioManager will be null." << std::endl;
    }

    std::cout << "Initializing CoreEngine with " << Systems.size() << " systems" << std::endl;

    for (size_t i = 0; i < Systems.size(); ++i)
    {
        std::cout << "Initializing system: " << Systems[i]->GetName() << std::endl;
        Systems[i]->Initialize();
    }

    std::cout << "CoreEngine initialization complete" << std::endl;
}

/* brief
   This function starts the core engine loop to start the game engine process
   there is the a exeception error checker too to get error thrown too
   Uses fixed timestep for physics and variable timestep for rendering

   */
void CoreEngine::GameLoop()
{
    double fps_interval = 1.0;
    const double max_frame_time = 0.25; // Cap frame time to prevent spiral of death
    accumulator = 0.0;

    while (!glfwWindowShouldClose(graphicsSystem->getWindow()))
    {

        if (m_hasPendingLoad)
        {
            // Wait for cutscene to finish before loading scene
            if (cutsceneSystem && cutsceneSystem->IsPlaying())
            {
                // Cutscene still playing - delay scene load
                // (scene will load on next frame after cutscene ends)
            }
            else
            {
                std::cout << "[CoreEngine] ===== PENDING SCENE LOAD DETECTED =====" << std::endl;
                std::cout << "[CoreEngine] Scene to load: " << m_pendingScene << std::endl;
                std::cout << "[CoreEngine] Auto-play flag: " << (m_autoPlayAfterLoad ? "TRUE" : "FALSE") << std::endl;

                LevelEditor *level = GetLevelEditor();
                level->LoadLevel(m_pendingScene);
                std::cout << "[CoreEngine] Scene loaded successfully" << std::endl;

                // Re-init game-specific systems after scene load
                if (gameCallbacks)
                {
                    gameCallbacks->OnSceneLoadReinit();
                    std::cout << "[CoreEngine] Game systems re-initialised after scene load" << std::endl;
                }

                // ===== AUTO-ENTER PLAY MODE IF REQUESTED =====
                if (m_autoPlayAfterLoad)
                {
                    std::cout << "[CoreEngine] Auto-play flag is TRUE - entering play mode\n";
                    level->EnterPlayMode();
                    std::cout << "[CoreEngine] Play mode entered\n";
                }
                else
                {
                    std::cout << "[CoreEngine] Auto-play flag is FALSE - NOT entering play mode\n";
                }

                // Unpause the game after scene load completes
                if (m_gamePause)
                {
                    std::cout << "[CoreEngine] Unpausing game after scene load" << std::endl;
                    SetGamePaused(false);
                }

                // Clear event system again to ensure no stale events from scene transition
                EventSystem::Clear();

                m_pendingScene = "";
                m_hasPendingLoad = false;
                m_autoPlayAfterLoad = false; // Reset flag

                if (inputManager)
                    inputManager->ResetState();
                CameraManager::Instance().ResetState();
                accumulator = 0.0;
            }
        }

        // Update variable delta_time and FPS
        update_time(fps_interval);

        // Cap delta_time to avoid spiral of death
        double frame_time = (delta_time > max_frame_time) ? max_frame_time : delta_time;
        accumulator += frame_time;

        g_systemTimings.clear();
        g_totalLoopTime = 0.0;

        if (inputManager)
            g_systemTimings[inputManager->GetName()] = 0.0;
        if (gameObjectSystem)
            g_systemTimings[gameObjectSystem->GetName()] = 0.0;
        if (transformSystem)
            g_systemTimings[transformSystem->GetName()] = 0.0;
        if (collisionSystem)
            g_systemTimings[collisionSystem->GetName()] = 0.0;
        if (physicsSystem)
            g_systemTimings[physicsSystem->GetName()] = 0.0;
        if (graphicsSystem)
            g_systemTimings[graphicsSystem->GetName()] = 0.0;
        if (debugSystem)
            g_systemTimings[debugSystem->GetName()] = 0.0;
        if (cutsceneSystem)
            g_systemTimings[cutsceneSystem->GetName()] = 0.0;
        if (levelEditor)
            g_systemTimings[levelEditor->GetName()] = 0.0;
        if (scriptSystem)
            g_systemTimings[scriptSystem->GetName()] = 0.0;
        if (imguiSystem)
            g_systemTimings[imguiSystem->GetName()] = 0.0;
        if (timerSystem)
            g_systemTimings[timerSystem->GetName()] = 0.0;

        // Start ImGui frame
        if (imguiSystem)
        {
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
            glViewport(0, 0, graphicsSystem->GetWindowWidth(), graphicsSystem->GetWindowHeight());
            glClearColor(0.15f, 0.15f, 0.15f, 1.f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            ImGui_ImplOpenGL3_NewFrame();
            ImGui_ImplGlfw_NewFrame();
            ImGui::NewFrame();
            ImGuizmo::BeginFrame();
        }

        // FIXED TIMESTEP LOOP - Run physics multiple times if needed
        while (accumulator >= fixed_dt)
        {
            // InputManager ALWAYS updates (even in step mode) to detect F2/F3 keys
            if (inputManager)
            {
                std::chrono::high_resolution_clock::time_point sysStart = std::chrono::high_resolution_clock::now();
                inputManager->Update(static_cast<float>(fixed_dt));
                std::chrono::high_resolution_clock::time_point sysEnd = std::chrono::high_resolution_clock::now();
                double ms = std::chrono::duration<double, std::milli>(sysEnd - sysStart).count();
                g_systemTimings[inputManager->GetName()] += ms;
                g_totalLoopTime += ms;
            }

            bool isPlaying = (levelEditor && levelEditor->GetEditorMode() == EditorMode::Play);
            bool shouldRunPhysics = ((isPlaying && !stepMode) || stepOnce) && !m_gamePause;
            // Only run physics if NOT in step mode/paused, OR if stepping once
            if (shouldRunPhysics)
            {
                // Update physics-dependent systems with FIXED timestep
                if (gameObjectSystem)
                {
                    std::chrono::high_resolution_clock::time_point sysStart = std::chrono::high_resolution_clock::now();
                    gameObjectSystem->Update(static_cast<float>(fixed_dt));
                    std::chrono::high_resolution_clock::time_point sysEnd = std::chrono::high_resolution_clock::now();
                    double ms = std::chrono::duration<double, std::milli>(sysEnd - sysStart).count();
                    g_systemTimings[gameObjectSystem->GetName()] += ms;
                    g_totalLoopTime += ms;
                }

                if (transformSystem)
                {
                    std::chrono::high_resolution_clock::time_point sysStart = std::chrono::high_resolution_clock::now();
                    transformSystem->Update(static_cast<float>(fixed_dt));
                    std::chrono::high_resolution_clock::time_point sysEnd = std::chrono::high_resolution_clock::now();
                    double ms = std::chrono::duration<double, std::milli>(sysEnd - sysStart).count();
                    g_systemTimings[transformSystem->GetName()] += ms;
                    g_totalLoopTime += ms;
                }

                if (collisionSystem)
                {
                    std::chrono::high_resolution_clock::time_point sysStart = std::chrono::high_resolution_clock::now();
                    collisionSystem->Update(static_cast<float>(fixed_dt));
                    std::chrono::high_resolution_clock::time_point sysEnd = std::chrono::high_resolution_clock::now();
                    double ms = std::chrono::duration<double, std::milli>(sysEnd - sysStart).count();
                    g_systemTimings[collisionSystem->GetName()] += ms;
                    g_totalLoopTime += ms;
                }

                if (physicsSystem)
                {
                    std::chrono::high_resolution_clock::time_point sysStart = std::chrono::high_resolution_clock::now();
                    physicsSystem->Update(static_cast<float>(fixed_dt));
                    std::chrono::high_resolution_clock::time_point sysEnd = std::chrono::high_resolution_clock::now();
                    double ms = std::chrono::duration<double, std::milli>(sysEnd - sysStart).count();
                    g_systemTimings[physicsSystem->GetName()] += ms;
                    g_totalLoopTime += ms;
                }

                SpawnTest_Update(); // Stress Test Update

                if (scriptSystem)
                {
                    std::chrono::high_resolution_clock::time_point sysStart = std::chrono::high_resolution_clock::now();
                    scriptSystem->Update(static_cast<float>(fixed_dt)); // Use fixed timestep for consistency
                    std::chrono::high_resolution_clock::time_point sysEnd = std::chrono::high_resolution_clock::now();
                    double ms = std::chrono::duration<double, std::milli>(sysEnd - sysStart).count();
                    g_systemTimings[scriptSystem->GetName()] = ms;
                    g_totalLoopTime += ms;
                }

                // Flush any GameObjects destroyed from scripts this frame
                GameObjectFactory::Instance().FlushPendingDestroys();

                // Ghost system: record player state + drive ghost object (after scripts)
                if (ghostSystem)
                    ghostSystem->Update(static_cast<float>(fixed_dt));

                // Game-specific fixed update (menu logic, etc.) – injected by the game project
                if (gameCallbacks)
                    gameCallbacks->FixedUpdatePlaying(static_cast<float>(fixed_dt));

                stepOnce = false; // Reset after processing one frame
            }

            // Game-specific always-fixed update (pause menu, win/lose) – runs even when paused
            if (gameCallbacks)
                gameCallbacks->FixedUpdateAlways(static_cast<float>(fixed_dt));
            // In step mode, only process ONE timestep then break
            if (stepMode)
                break;

            accumulator -= fixed_dt;
        }

        EventSystem::ProcessEvents();

        // Update audio fades
        if (audioManager)
        {
            audioManager->UpdateFade(static_cast<float>(delta_time));
        }

        // Update FMOD audio system (required every frame for audio playback)
        if (audioEngine)
        {
            audioEngine->updateAudio();
        }

        // VARIABLE TIMESTEP - Rendering and UI systems
        if (graphicsSystem)
        {
            std::chrono::high_resolution_clock::time_point sysStart = std::chrono::high_resolution_clock::now();
            graphicsSystem->Update(static_cast<float>(delta_time));
            std::chrono::high_resolution_clock::time_point sysEnd = std::chrono::high_resolution_clock::now();
            double ms = std::chrono::duration<double, std::milli>(sysEnd - sysStart).count();
            g_systemTimings[graphicsSystem->GetName()] = ms;
            g_totalLoopTime += ms;
        }

        if (debugSystem)
        {
            std::chrono::high_resolution_clock::time_point sysStart = std::chrono::high_resolution_clock::now();
            debugSystem->Update(static_cast<float>(delta_time));
            std::chrono::high_resolution_clock::time_point sysEnd = std::chrono::high_resolution_clock::now();
            double ms = std::chrono::duration<double, std::milli>(sysEnd - sysStart).count();
            g_systemTimings[debugSystem->GetName()] = ms;
            g_totalLoopTime += ms;
        }

        if (levelEditor)
        {
            std::chrono::high_resolution_clock::time_point sysStart = std::chrono::high_resolution_clock::now();
            levelEditor->Update(static_cast<float>(delta_time));
            std::chrono::high_resolution_clock::time_point sysEnd = std::chrono::high_resolution_clock::now();
            double ms = std::chrono::duration<double, std::milli>(sysEnd - sysStart).count();
            g_systemTimings[levelEditor->GetName()] = ms;
            g_totalLoopTime += ms;
        }

        if (levelEditor && levelEditor->HasPendingSceneReload())
        {
            levelEditor->PerformSceneReload();
        }

        if (timerSystem)
        {
            timerSystem->Update(static_cast<float>(delta_time));
        }

        // TextInputSystem: modal dialog input — runs every frame regardless of pause
        if (textInputSystem)
        {
            textInputSystem->Update(static_cast<float>(delta_time));
        }

        // Game-specific variable-timestep update (game timer, state machine, etc.)
        if (gameCallbacks)
            gameCallbacks->VariableUpdate(static_cast<float>(delta_time));

        if (imguiSystem)
        {
            std::chrono::high_resolution_clock::time_point sysStart = std::chrono::high_resolution_clock::now();
            imguiSystem->Update(static_cast<float>(delta_time));
            std::chrono::high_resolution_clock::time_point sysEnd = std::chrono::high_resolution_clock::now();
            double ms = std::chrono::duration<double, std::milli>(sysEnd - sysStart).count();
            g_systemTimings[imguiSystem->GetName()] = ms;
            g_totalLoopTime += ms;
        }

        // End ImGui frame and render
        if (imguiSystem)
        {
            ImGui::Render();
            ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        }

        // Update cutscene
        if (cutsceneSystem && cutsceneSystem->IsPlaying()) {
            cutsceneSystem->Update(static_cast<float>(delta_time));         
        }

        // Swap buffers and poll events
        glfwSwapBuffers(graphicsSystem->getWindow());
        glfwPollEvents();
    }
}

/* brief
   This function deletes the engines and stops the engine processes and clears all systems

   */
void CoreEngine::DestroySystems()
{
    // Clear factory before destroying systems so GameObjects can still
    // reach the ScriptSystem during their destructors
    GameObjectFactory::Instance().Clear();

    std::cout << "Destroying " << Systems.size() << " systems" << std::endl;
    scriptSystem = nullptr;
    Systems.clear();
    if (audioManager)
    {
        std::cout << "[CoreEngine] Deleting AudioManager..." << std::endl;
        delete audioManager;
        audioManager = nullptr; // Good practice to nullify after delete
    }
}

void CoreEngine::update_time(double fps_calc_interval)
{
    static double prev_time = glfwGetTime();
    double curr_time = glfwGetTime();
    delta_time = curr_time - prev_time; // Now using static delta_time
    prev_time = curr_time;

    // fps calculations
    static double count = 0.0; // number of game loop iterations
    static double start_time = glfwGetTime();
    double elapsed_time = curr_time - start_time;

    ++count;

    if (elapsed_time > fps_calc_interval)
    {
        fps = count / elapsed_time; // Now using static fps
        count = 0.0;
        start_time = curr_time;
    }

    std::stringstream iostream;
    std::string tempfps, tempdt, tempfixeddt;
    std::string title_str;

    iostream << CoreEngine::fps;
    tempfps = iostream.str();

    iostream.str("");
    iostream.clear();

    iostream << CoreEngine::delta_time;
    tempdt = iostream.str();

    iostream.str("");
    iostream.clear();

    iostream << CoreEngine::fixed_dt;
    tempfixeddt = iostream.str();

    title_str = "Game Engine | FPS:" + tempfps + " | DT: " + tempdt + " | Fixed DT: " + tempfixeddt;

    GLFWwindow *Wind = CORE->GetGraphicsSystem()->getWindow();
    if (Wind)
    {
        glfwSetWindowTitle(Wind, title_str.c_str());
    }
}

void CoreEngine::SetGamePaused(bool paused)
{
    SetGamePaused(paused, true);
}

// check if audio needs to be pause
void CoreEngine::SetGamePaused(bool paused, bool pauseAudio)
{
    std::cout << (paused ? "Game is paused\n" : "Game is resuming\n");
    if (m_gamePause == paused)
    {
        return;
    }

    m_gamePause = paused;

    // only pause audio if requested and not in cutscene
    if (pauseAudio && audioManager && !(cutsceneSystem && cutsceneSystem->IsPlaying()))
    {
        audioManager->PauseAllSounds(paused);
    }
}
