#include "core.h"
#include "System.h"
#include "GameObjectSystem.h"
#include "GraphicsSystem.h"
#include <GLFW/glfw3.h>
#include <iostream>
#include <thread>
#include <chrono>
#include <Physics.h>
#include "ImGuiSystem.h"
#include "InputManager.h"
#include "CollisionSystem.h"
#include "PhysicsSystem.h"
#include "DebugSystem.h"

CoreEngine *CORE;
double CoreEngine::fps = 0.0;
double CoreEngine::delta_time = 0.0;

CoreEngine::CoreEngine()
{
    LastTime = 0;
    GameActive = true;
    CORE = this;

    std::cout << "CoreEngine constructed" << std::endl;

    // Initialize default systems automatically
    InitSystems();
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
    CoreEngine::ConfigFile("../Logs/config.txt");
    std::cout << "Initializing default systems..." << std::endl;
    // 1. Graphics first (needed for window)
    graphicsSystem = AddSystem<GraphicsSystem>(WindWidth, WindHeight);

    // 2. ImGui (needs graphics window)
    imguiSystem = AddSystem<ImGuiSystem>(graphicsSystem->getWindow());

    // 3. Input Manager - MUST BE EARLY to capture input before physics updates
    inputManager = AddSystem<InputManager>(graphicsSystem->getWindow());
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

            for (size_t i = 0; i < Systems.size(); ++i)
            {

                Systems[i]->Update(CoreEngine::GetDeltaTime());
            }
        }
    }
    catch (const std::exception &e)
    {
        // Log standard exceptions to debuglog.txt
        std::ofstream log("../Logs/CrashLog.txt", std::ios::app);
        log << "[EXCEPTION] " << e.what() << std::endl;
        log.close();

        std::cerr << "Exception caught in CoreEngine::GameLoop(): " << e.what() << std::endl;
    }
    catch (...)
    {
        // Log unknown exceptions
        std::ofstream log("../Logs/CrashLog.txt", std::ios::app);
        log << "[UNKNOWN EXCEPTION]" << std::endl;
        log.close();

        std::cerr << "Unknown exception caught in CoreEngine::GameLoop()" << std::endl;
    }
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

    iostream << CoreEngine::delta_time;
    tempdt = iostream.str();

    title_str = "Game Engine | FPS:" + tempfps + "|DT : " + tempdt;

    GLFWwindow *Wind = CORE->GetGraphicsSystem()->getWindow();
    if (Wind)
    {
        glfwSetWindowTitle(Wind, title_str.c_str());
    }
}