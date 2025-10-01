#pragma once
#include <iostream>
#include <vector>
#include <memory>
#include "System.h"
#include <fstream>
#include <iomanip>

// Forward declarations
class GameObjectSystem;
class GraphicsSystem;
class ImGuiSystem;
class InputManager;
class CollisionSystem;
class PhysicsSystem;

class CoreEngine {
public:
    CoreEngine();
    ~CoreEngine();

    void GameLoop();
    void DestroySystems();
    void Init();

    template<typename T, typename... Args>
    T* AddSystem(Args&&... args) {
        auto system = std::make_unique<T>(std::forward<Args>(args)...);
        T* systemPtr = system.get();
        Systems.push_back(std::move(system));
        return systemPtr;
    }

    // Direct access to commonly used systems
    GameObjectSystem* GetGameObjectSystem() const {
        return gameObjectSystem;
    }

    GraphicsSystem* GetGraphicsSystem() const {
        return graphicsSystem;
    }

    ImGuiSystem* GetImguiSystem() const {
        return imguiSystem;
    }

    InputManager* GetInputManager() const {
        return inputManager;
    }

    CollisionSystem* GetCollisionSystem() const {
        return collisionSystem;
    }

    PhysicsSystem* GetPhysicsSystem() const {
        return physicsSystem;
    }

    void ConfigFile(const char* configF);

    void setWindsize(int w, int h) {
        WindWidth = w;
        WindHeight = h;
    };
    

    static double GetFPS() { return fps; }
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

    unsigned LastTime;
    bool GameActive;
    static double fps;
    static double delta_time;
    int WindWidth;
    int WindHeight;
    void update_time(double fps_calc_interval = 1.0);
};

extern CoreEngine* CORE;