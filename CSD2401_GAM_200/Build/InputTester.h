#pragma once
#include "System.h"

class GameObjectFactory;

class InputSystem : public ISystem {
public:
    InputSystem();

    void Update(float timeslice) override;
    std::string GetName() override { return "InputSystem"; }
    void Initialize() override;

private:
    GameObjectFactory& factory;
};