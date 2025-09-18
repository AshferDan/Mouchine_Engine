#pragma once
#include <iostream>
#include <vector>
#include <memory>
#include "System.h"

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

private:
    std::vector<std::unique_ptr<ISystem>> Systems;
    unsigned LastTime;
    bool GameActive;
};

extern CoreEngine* CORE;