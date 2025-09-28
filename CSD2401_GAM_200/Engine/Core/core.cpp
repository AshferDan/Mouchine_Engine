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
#include <csignal>


CoreEngine* CORE;
double CoreEngine::fps = 0.0;
double CoreEngine::delta_time = 0.0;

// Signal handler for real crashes
void CrashHandler(int signal) {
    const char* signalName = "Unknown";

    if (signal == SIGSEGV) signalName = "Segmentation Fault (SIGSEGV)";
    else if (signal == SIGFPE) signalName = "Floating Point Exception (SIGFPE)";
    else if (signal == SIGABRT) signalName = "Abort (SIGABRT)";
    else if (signal == SIGILL) signalName = "Illegal Instruction (SIGILL)";
    else if (signal == SIGTERM) signalName = "Termination (SIGTERM)";

    std::cerr << "=== CRASH DETECTED ===" << std::endl;
    std::cerr << "Signal: " << signalName << " Signal Code: (" << signal << ")" << std::endl;

    // Log to file
    std::ofstream log("debuglog.txt", std::ios::app);
    if (log.is_open()) {
        log << "[CRASH] Signal: " << signalName << " Signal Code: (" << signal << ")" << std::endl;
        log.close();
    }

    std::_Exit(signal); // exit immediately
}

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
    std::cout << "Initializing default systems..." << std::endl;
    gameObjectSystem = AddSystem<GameObjectSystem>();
    graphicsSystem = AddSystem<GraphicsSystem>(WindWidth,WindHeight);
    imguiSystem = AddSystem<ImGuiSystem>(graphicsSystem->getWindow());
    inputManager = AddSystem<InputManager>(graphicsSystem->getWindow());

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
    // Register OS-level crash signals
    std::signal(SIGSEGV, CrashHandler);
    std::signal(SIGFPE, CrashHandler);
    std::signal(SIGABRT, CrashHandler);
    std::signal(SIGILL, CrashHandler);
    std::signal(SIGTERM, CrashHandler);

    double fps_interval = 1.0;

    try {
        while (!glfwWindowShouldClose(graphicsSystem->getWindow())) {
            update_time(fps_interval);

            for (size_t i = 0; i < Systems.size(); ++i) {
                Systems[i]->Update(CoreEngine::GetDeltaTime());
            }
        }
    }
    catch (const std::exception& e) {
        // Log standard exceptions to debuglog.txt
        std::ofstream log("debuglog.txt", std::ios::app);
        log << "[EXCEPTION] " << e.what() << std::endl;
        log.close();

        std::cerr << "Exception caught in CoreEngine::GameLoop(): " << e.what() << std::endl;
    }
    catch (...) {
        // Log unknown exceptions
        std::ofstream log("debuglog.txt", std::ios::app);
        log << "[UNKNOWN EXCEPTION]" << std::endl;
        log.close();

        std::cerr << "Unknown exception caught in CoreEngine::GameLoop()" << std::endl;
    }
}

void CoreEngine::DestroySystems() {
    std::cout << "Destroying " << Systems.size() << " systems" << std::endl;
    Systems.clear();
}

void CoreEngine::update_time(double fps_calc_interval) {
    static double prev_time = glfwGetTime();
    double curr_time = glfwGetTime();
    delta_time = curr_time - prev_time;  // Now using static delta_time
    prev_time = curr_time;

    // fps calculations
    static double count = 0.0; // number of game loop iterations
    static double start_time = glfwGetTime();
    double elapsed_time = curr_time - start_time;

    ++count;

    if (elapsed_time > fps_calc_interval) {
        fps = count / elapsed_time;  // Now using static fps
        count = 0.0;
        start_time = curr_time;
    }

    std::stringstream iostream;
    std::string tempfps , tempdt;
    std::string title_str;
    tempfps = iostream.str();
    
    iostream << CoreEngine::fps;
    tempfps = iostream.str();

    iostream << CoreEngine::delta_time;
    tempdt = iostream.str();


    title_str = "Game Engine | FPS:" + tempfps + "|DT : " + tempdt;
    
    GLFWwindow* Wind = CORE->GetGraphicsSystem()->getWindow();
    if (Wind) {
        glfwSetWindowTitle(Wind,title_str.c_str());
    }
}