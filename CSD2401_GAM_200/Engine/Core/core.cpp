#include "core.h"
#include "System.h"
#include "GameObjectSystem.h"
#include "GraphicsSystem.h"
#include <GLFW/glfw3.h>
#include <iostream>
#include <thread>
#include <chrono>

CoreEngine* CORE;

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
    LastTime = static_cast<unsigned>(glfwGetTime() * 1000);

    std::cout << "Starting game loop..." << std::endl;

    while (GameActive) {
        unsigned currenttime = static_cast<unsigned>(glfwGetTime() * 1000);
        float dt = (currenttime - LastTime) / 1000.0f;
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
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(16));
    }
}

void CoreEngine::DestroySystems() {
    std::cout << "Destroying " << Systems.size() << " systems" << std::endl;
    Systems.clear();
}