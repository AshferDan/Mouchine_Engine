//#include "Core.h"
//#include <GLFW/glfw3.h>
//
//CoreEngine* CORE;
//
//CoreEngine::CoreEngine() {
//
//	LastTime = 0;
//	GameActive = true;
//	CORE = this;
//}
//
//void CoreEngine::Init()
//{
//	for (unsigned i = 0; i < Systems.size(); ++i)
//		Systems[i]->Init();
//}
//
//void CoreEngine::GameLoop()
//{
//	//Initialize the last time variable so our first frame
//	//is "zero" seconds (and not some huge unknown number)
//	LastTime = glfwGetTime();
//
//	while (GameActive)
//	{
//		//Get the current time in milliseconds
//		unsigned currenttime = glfwGetTime();
//		//Convert it to the time passed since the last frame (in seconds)
//		float dt = (currenttime - LastTime) / 1000.0f;
//		//Update the when the last update started
//		LastTime = currenttime;
//
//		//Update every system and tell each one how much
//		//time has passed since the last update
//		for (unsigned i = 0; i < Systems.size(); ++i)
//			Systems[i]->Update(dt);
//	}
//
//}


#include "core.h"
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
}

CoreEngine::~CoreEngine() {
    DestroySystems();
    std::cout << "CoreEngine destroyed" << std::endl;
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
    // Initialize the last time variable so our first frame
    // is "zero" seconds (and not some huge unknown number)
    LastTime = static_cast<unsigned>(glfwGetTime() * 1000); // Convert to milliseconds

    std::cout << "Starting game loop..." << std::endl;

    while (GameActive) {
        // Get the current time in milliseconds
        unsigned currenttime = static_cast<unsigned>(glfwGetTime() * 1000);

        // Convert it to the time passed since the last frame (in seconds)
        float dt = (currenttime - LastTime) / 1000.0f;

        // Update the when the last update started
        LastTime = currenttime;

        // Update every system and tell each one how much
        // time has passed since the last update
        for (size_t i = 0; i < Systems.size(); ++i) {
            Systems[i]->Update(dt);
        }

        // Simple exit condition for testing (remove this for real game loop)
        static int frameCount = 0;
        frameCount++;
        if (frameCount > 300) { // Run for ~5 seconds at 60fps
            GameActive = false;
            std::cout << "Stopping game loop after " << frameCount << " frames" << std::endl;
        }

        // Add a small delay to simulate frame rate
        // In a real game, this would be handled by your graphics API
        std::this_thread::sleep_for(std::chrono::milliseconds(16)); // ~60 FPS
    }
}

void CoreEngine::DestroySystems() {
    std::cout << "Destroying " << Systems.size() << " systems" << std::endl;
    Systems.clear();
}