#pragma once
#include <iostream>
#include <vector>
#include <memory>
#include "System.h"

// Forward declarations
class GameObjectSystem;
class GraphicsSystem;

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

private:
    void InitSystems();
    std::vector<std::unique_ptr<ISystem>> Systems;
    GameObjectSystem* gameObjectSystem = nullptr;
    GraphicsSystem* graphicsSystem = nullptr;
    unsigned LastTime;
    bool GameActive;
};

extern CoreEngine* CORE;