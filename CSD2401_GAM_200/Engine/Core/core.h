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
    CoreEngine();
    ~CoreEngine();

    void GameLoop();
    void DestroySystems();
    void exit();
    void Init();

    template <typename T, typename... Args>
    T *AddSystem(Args &&...args)
    {
        auto system = std::make_unique<T>(std::forward<Args>(args)...);
        T *systemPtr = system.get();
        Systems.push_back(std::move(system));
        return systemPtr;
    }

    // Direct access to commonly used systems
    GameObjectSystem *GetGameObjectSystem() const
    {
        return gameObjectSystem;
    }

    GraphicsSystem *GetGraphicsSystem() const
    {
        return graphicsSystem;
    }

    ImGuiSystem *GetImguiSystem() const
    {
        return imguiSystem;
    }

    InputManager *GetInputManager() const
    {
        return inputManager;
    }

    CollisionSystem *GetCollisionSystem() const
    {
        return collisionSystem;
    }

    PhysicsSystem *GetPhysicsSystem() const
    {
        return physicsSystem;
    }

    DebugSystem *GetDebugSystem() const
    {
        return debugSystem;
    }

    void SetAudioEngine(AudioEngine* audio) { audioEngine = audio; }

    void ConfigFile(const char *configF);

    void setWindsize(int w, int h)
    {
        WindWidth = w;
        WindHeight = h;
    };

    static double GetFPS() { return fps; }
    static double GetDeltaTime() { return delta_time; }

private:
    void InitSystems();
    std::vector<std::unique_ptr<ISystem>> Systems;

    GameObjectSystem *gameObjectSystem = nullptr;
    GraphicsSystem *graphicsSystem = nullptr;
    ImGuiSystem *imguiSystem = nullptr;
    InputManager *inputManager = nullptr;
    CollisionSystem *collisionSystem = nullptr;
    PhysicsSystem *physicsSystem = nullptr;
    DebugSystem *debugSystem = nullptr;
    AudioEngine* audioEngine = nullptr;
    
    unsigned LastTime;
    bool GameActive;
    static double fps;
    static double delta_time;
    int WindWidth;
    int WindHeight;
    void update_time(double fps_calc_interval = 1.0);
};

extern CoreEngine *CORE;