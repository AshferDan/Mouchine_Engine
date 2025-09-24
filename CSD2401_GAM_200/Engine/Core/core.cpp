#include "core.h"
#include "System.h"
#include "GameObjectSystem.h"
#include "GraphicsSystem.h"
#include "Graphics.h"
#include <GLFW/glfw3.h>
#include <iostream>
#include <thread>
#include <chrono>

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

void CoreEngine::InitSystems() {
    std::cout << "Initializing default systems..." << std::endl;
    gameObjectSystem = AddSystem<GameObjectSystem>();
    graphicsSystem = AddSystem<GraphicsSystem>();

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

    while (!glfwWindowShouldClose(glfwGetCurrentContext())) {
       
        update_time(1.0);
        unsigned currenttime = static_cast<unsigned>(glfwGetTime());
        float dt = (currenttime - LastTime);
        LastTime = currenttime;

        // Update every system
        for (size_t i = 0; i < Systems.size(); ++i) {
            Systems[i]->Update(dt);
        }

        // Simple exit condition for testing
        static int frameCount = 0;
        frameCount++;
        if (frameCount > 300) {
            GameActive = false;
            std::cout << "Stopping game loop after " << frameCount << " frames" << std::endl;
            std::cout << "Final FPS: " << fps << ", Final Delta Time: " << delta_time << "s" << std::endl;
        }
    }
}


void CoreEngine::DestroySystems() {
    std::cout << "Destroying " << Systems.size() << " systems" << std::endl;
    Systems.clear();
}

void CoreEngine::update_time(double fps_calc_interval) {
    // EXACT COPY FROM YOUR IMAGE with static variables:
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