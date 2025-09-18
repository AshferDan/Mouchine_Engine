#pragma once
#include "System.h"

class GameObjectFactory;

class GameObjectSystem : public ISystem {
public:
    GameObjectSystem();

    void Update(float timeslice) override;
    std::string GetName() override { return "GameObjectSystem"; }
    void Initialize() override;

private:
    GameObjectFactory& factory;
};