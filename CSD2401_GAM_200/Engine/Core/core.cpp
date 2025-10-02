/* Start Header ************************************************************************/
/*!
\file       Core.cpp
\author     Jonathan Fam Qian Wei [71% - 103 lines]
\co-author  Lee Hwee Min [29% - 41 lines]
\par        Fam.q@digipen.edu , l.hweemin@digipen.edu
\date       October, 2, 2025

Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*/
/* End Header **************************************************************************/
#include "core.h"
bool InputManager::KeyStateQuit = false;
CoreEngine *CORE;

double CoreEngine::fps = 0.0;
double CoreEngine::delta_time = 0.0;
double g_totalLoopTime = 0.0;
std::map<std::string, double> g_systemTimings;

CoreEngine::CoreEngine()
{
    LastTime = 0;
    GameActive = true;
    CORE = this;

    std::cout << "CoreEngine constructed" << std::endl;
}

CoreEngine::~CoreEngine()
{
    DestroySystems();
    std::cout << "CoreEngine destroyed" << std::endl;
}

void CoreEngine::ConfigFile(const char *configF)
{
    std::ifstream fp{configF};
    if (!fp)
    {
        std::cout << "Cannot find file";
        glfwTerminate();
    }

    int WindWid = 0, WindHei = 0;
    std::string temp;
    fp >> temp >> WindWid;
    fp >> temp >> WindHei;
    setWindsize(WindWid, WindHei);
}

void CoreEngine::InitSystems()
{
    CoreEngine::ConfigFile("./Logs/config.txt");
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

    // 5. Collision System - detects collisions
    collisionSystem = AddSystem<CollisionSystem>();

    // 6. Physics System - processes collisions and applies physics
    physicsSystem = AddSystem<PhysicsSystem>();

    // Connect CollisionSystem to PhysicsSystem
    collisionSystem->SetPhysicsSystem(physicsSystem);

    std::cout << "Default systems initialized: GameObjectSystem, GraphicsSystem" << std::endl;
}

void CoreEngine::Init()
{
    InitSystems();
    std::cout << "Initializing CoreEngine with " << Systems.size() << " systems" << std::endl;

    for (size_t i = 0; i < Systems.size(); ++i)
    {
        std::cout << "Initializing system: " << Systems[i]->GetName() << std::endl;
        Systems[i]->Initialize();
    }

    std::cout << "CoreEngine initialization complete" << std::endl;
}

void CoreEngine::GameLoop()
{
    double fps_interval = 1.0;
    try
    {
        while (!glfwWindowShouldClose(graphicsSystem->getWindow()))
        {
            update_time(fps_interval);

            // 1. Clear previous frame timings and reset total time
            g_systemTimings.clear();
            g_totalLoopTime = 0.0; // Reset total time for the new frame

            // 2. Update all systems and collect timings
            for (size_t i = 0; i < Systems.size(); ++i)
            {
                if (Systems[i]->GetName() == "ImGui_System")
                    continue;

                std::chrono::high_resolution_clock::time_point sysStart = std::chrono::high_resolution_clock::now();
                Systems[i]->Update(CoreEngine::GetDeltaTime());
                std::chrono::high_resolution_clock::time_point sysEnd = std::chrono::high_resolution_clock::now();

                double ms = std::chrono::duration<double, std::milli>(sysEnd - sysStart).count();
                g_systemTimings[Systems[i]->GetName()] = ms;
                g_totalLoopTime += ms; // Accumulate total time
            }

            // 3. Update ImGui System (Time it like any other system)
            if (imguiSystem)
            {
                std::chrono::high_resolution_clock::time_point sysStart = std::chrono::high_resolution_clock::now();

                imguiSystem->Update(CoreEngine::GetDeltaTime());

                std::chrono::high_resolution_clock::time_point sysEnd = std::chrono::high_resolution_clock::now();

                double ms = std::chrono::duration<double, std::milli>(sysEnd - sysStart).count();
                g_systemTimings[imguiSystem->GetName()] = ms;
                g_totalLoopTime += ms; // Add ImGui time to total loop time
            }
        }
    }
    catch (const std::exception &e)
    {
        // Log standard exceptions to debuglog.txt
        std::ofstream log("./Logs/CrashLog.txt", std::ios::app);
        log << "[EXCEPTION] " << e.what() << std::endl;
        log.close();

        std::cerr << "Exception caught in CoreEngine::GameLoop(): " << e.what() << std::endl;
    }
    catch (...)
    {
        // Log unknown exceptions
        std::ofstream log("./Logs/CrashLog.txt", std::ios::app);
        log << "[UNKNOWN EXCEPTION]" << std::endl;
        log.close();

        std::cerr << "Unknown exception caught in CoreEngine::GameLoop()" << std::endl;
    }
    CoreEngine::DestroySystems();
    glfwTerminate();
}

void CoreEngine::DestroySystems()
{
    std::cout << "Destroying " << Systems.size() << " systems" << std::endl;
    Systems.clear();
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
    std::string tempfps, tempdt;
    std::string title_str;

    iostream << CoreEngine::fps;
    tempfps = iostream.str();

    iostream.str("");
    iostream.clear();

    iostream << CoreEngine::delta_time;
    tempdt = iostream.str();

    title_str = "Game Engine | FPS:" + tempfps + "|DT : " + tempdt;

    GLFWwindow *Wind = CORE->GetGraphicsSystem()->getWindow();
    if (Wind)
    {
        glfwSetWindowTitle(Wind, title_str.c_str());
    }
}
