#include "core.h"
#include "System.h"
#include "GameObjectSystem.h"
#include "GraphicsSystem.h"
#include "Graphics.h"
#include <GLFW/glfw3.h>
#include <iostream>
#include <thread>
#include <chrono>
#include <Physics.h>
#include "ImGuiSystem.h"


CoreEngine* CORE;
double CoreEngine::fps = 0.0;
double CoreEngine::delta_time = 0.0;

CoreEngine::CoreEngine() {
    LastTime = 0;
    GameActive = true;
    CORE = this;
    
    std::cout << "CoreEngine constructed" << std::endl;

    // Initialize default systems automatically
    InitSystems();
}

CoreEngine::~CoreEngine() {
    DestroySystems();
    std::cout << "CoreEngine destroyed" << std::endl;
}

void CoreEngine::ConfigFile(const char* configF)
{
    std::ifstream fp{ configF };
    if (!fp) {
        std::cout << "Cannot find file";
        glfwTerminate();
    }

    int WindWid = 0, WindHei = 0;
    std::string temp;
    fp >> temp >> WindWid;
    fp >> temp >> WindHei;
    setWindsize(WindWid, WindHei);
}

void CoreEngine::InitSystems() {
    CoreEngine::ConfigFile("../Logs/config.txt");
    std::cout << WindWidth << WindHeight << "HERE IS THE WINDOW SIZESSSSS\n";
    std::cout << "Initializing default systems..." << std::endl;
    gameObjectSystem = AddSystem<GameObjectSystem>();
    graphicsSystem = AddSystem<GraphicsSystem>(WindWidth,WindHeight);
    imguiSystem = AddSystem<ImGuiSystem>(graphicsSystem->getWindow());

    std::cout << "Default systems initialized: GameObjectSystem, GraphicsSystem" << std::endl;
}

void CoreEngine::Init() {
    std::cout << "Initializing CoreEngine with " << Systems.size() << " systems" << std::endl;

    for (size_t i = 0; i < Systems.size(); ++i) {
        std::cout << "Initializing system: " << Systems[i]->GetName() << std::endl;
        Systems[i]->Initialize();
    }


    std::cout << "CoreEngine initialization complete" << std::endl;
}

void CoreEngine::GameLoop() {
    // Initialize the last time variable
    LastTime = static_cast<unsigned>(glfwGetTime());

    std::cout << "Starting game loop..." << std::endl;

    while (!glfwWindowShouldClose(graphicsSystem->getWindow())) {
       
        
        //update_time(1.0);

       
        for (size_t i = 0; i < Systems.size(); ++i) {
            Systems[i]->Update(0.1);  // Use the actual delta_time
        }
        //std::cout << "Final FPS: " << fps << ", Final Delta Time: " << delta_time << "s" << std::endl;
    }
}


void CoreEngine::DestroySystems() {
    std::cout << "Destroying " << Systems.size() << " systems" << std::endl;
    Systems.clear();
}

void CoreEngine::update_time(double fps_calc_interval) {
    // get elapsed time (in seconds) between previous and current frames
    static double prev_time = glfwGetTime();
    double curr_time = glfwGetTime();
    delta_time = curr_time - prev_time;  // Now using static delta_time
    prev_time = curr_time;

    // fps calculations
    static double count = 0.0; // number of game loop iterations
    static double start_time = glfwGetTime();
    // get elapsed time since very beginning (in seconds) ...
    double elapsed_time = curr_time - start_time;

    ++count;

    // update fps at least every 10 seconds ...
    fps_calc_interval = (fps_calc_interval < 0.0) ? 0.0 : fps_calc_interval;
    fps_calc_interval = (fps_calc_interval > 10.0) ? 10.0 : fps_calc_interval;
    if (elapsed_time > fps_calc_interval) {
        fps = count / elapsed_time;  // Now using static fps
        start_time = curr_time;
        count = 0.0;
    }
}