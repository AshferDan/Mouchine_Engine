/* Start Header ************************************************************************/
/*!
\file       Core.h
\author     Jonathan Fam Qian Wei [100% - 102 lines]
\par        Fam.q@digipen.edu
\date       October, 2, 2025

Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*/
/* End Header **************************************************************************/

#pragma once
#include <iostream>
#include <vector>
#include <memory>
#include "System.h"
#include "audio.h"
#include <fstream>
#include <iomanip>
#include "GameObjectSystem.h"
#include "GraphicsSystem.h"
#include <GLFW/glfw3.h>
#include <thread>
#include <chrono>
#include <Physics.h>
#include "ImGuiSystem.h"
#include "InputManager.h"
#include "CollisionSystem.h"
#include "PhysicsSystem.h"
#include "DebugSystem.h"
#include <map>
#include "MathLib.h"

// Forward declarations
class GameObjectSystem;
class GraphicsSystem;
class ImGuiSystem;
class InputManager;
class CollisionSystem;
class PhysicsSystem;
class DebugSystem;
class CoreEngine
{
public:
    /**
     * @brief Constructs the CoreEngine and sets up initial variables.
     */
    CoreEngine();

    /**
     * @brief Destructs the CoreEngine and calls DestroySystems.
     */
    ~CoreEngine();

    /**
     * @brief The main loop that drives the game, including update and render cycles.
     */
    void GameLoop();

    /**
     * @brief Cleans up and destroys all active systems.
     */
    void DestroySystems();

    /**
     * @brief Signals the game to exit the GameLoop.
     */
    void exit();

    /**
     * @brief Initializes the CoreEngine and its systems.
     */
    void Init();

    /**
     * @brief Creates, initializes, and adds a new system to the engine.
     * @tparam T The type of the system to add (must derive from ISystem).
     * @tparam Args The types of the constructor arguments.
     * @param args The constructor arguments forwarded to the system's constructor.
     * @return A raw pointer to the newly created system.
     */
    template <typename T, typename... Args>
    T* AddSystem(Args &&...args)
    {
        auto system = std::make_unique<T>(std::forward<Args>(args)...);
        T* systemPtr = system.get();
        Systems.push_back(std::move(system));
        return systemPtr;
    }

    /**
     * @brief Gets a pointer to the GameObjectSystem.
     * @return A pointer to the GameObjectSystem.
     */
    GameObjectSystem* GetGameObjectSystem() const
    {
        return gameObjectSystem;
    }

    /**
     * @brief Gets a pointer to the GraphicsSystem.
     * @return A pointer to the GraphicsSystem.
     */
    GraphicsSystem* GetGraphicsSystem() const
    {
        return graphicsSystem;
    }

    /**
     * @brief Gets a pointer to the ImGuiSystem (for debugging/tools).
     * @return A pointer to the ImGuiSystem.
     */
    ImGuiSystem* GetImguiSystem() const
    {
        return imguiSystem;
    }

    /**
     * @brief Gets a pointer to the InputManager.
     * @return A pointer to the InputManager.
     */
    InputManager* GetInputManager() const
    {
        return inputManager;
    }

    /**
     * @brief Gets a pointer to the CollisionSystem.
     * @return A pointer to the CollisionSystem.
     */
    CollisionSystem* GetCollisionSystem() const
    {
        return collisionSystem;
    }

    /**
     * @brief Gets a pointer to the PhysicsSystem.
     * @return A pointer to the PhysicsSystem.
     */
    PhysicsSystem* GetPhysicsSystem() const
    {
        return physicsSystem;
    }

    /**
     * @brief Gets a pointer to the DebugSystem.
     * @return A pointer to the DebugSystem.
     */
    DebugSystem* GetDebugSystem() const
    {
        return debugSystem;
    }

    /**
     * @brief Sets the pointer to the AudioEngine.
     * @param audio A pointer to the external AudioEngine instance.
     */
    void SetAudioEngine(AudioEngine* audio) { audioEngine = audio; }

    /**
     * @brief Reads and applies configuration settings from a file.
     * @param configF The file path to the configuration file.
     */
    void ConfigFile(const char* configF);

    /**
     * @brief Sets the dimensions of the game window.
     * @param w The desired width of the window.
     * @param h The desired height of the window.
     */
    void setWindsize(int w, int h)
    {
        WindWidth = w;
        WindHeight = h;
    };

    /**
     * @brief Gets the current Frames Per Second (FPS).
     * @return The calculated FPS as a double.
     */
    static double GetFPS() { return fps; }

    /**
     * @brief Gets the time elapsed since the last frame (delta time).
     * @return The delta time as a double.
     */
    static double GetDeltaTime() { return delta_time; }

private:
    void InitSystems();
    std::vector<std::unique_ptr<ISystem>> Systems;

    GameObjectSystem* gameObjectSystem = nullptr;
    GraphicsSystem* graphicsSystem = nullptr;
    ImGuiSystem* imguiSystem = nullptr;
    InputManager* inputManager = nullptr;
    CollisionSystem* collisionSystem = nullptr;
    PhysicsSystem* physicsSystem = nullptr;
    DebugSystem* debugSystem = nullptr;
    AudioEngine* audioEngine = nullptr;

    unsigned LastTime;
    bool GameActive;
    static double fps;
    static double delta_time;
    int WindWidth;
    int WindHeight;

    /**
     * @brief Updates the FPS and Delta Time counters.
     * @param fps_calc_interval The interval in seconds to recalculate FPS (defaults to 1.0).
     */
    void update_time(double fps_calc_interval = 1.0);
};

extern CoreEngine* CORE;