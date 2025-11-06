#pragma once
#pragma once

#include "System.h"
#include "factory.h"
#include "Serialization.h"
#include "InputManager.h" //
#include "transform.h"
#include "MathLib.h"

class MenuLogicSystem : public ISystem {
public:
    MenuLogicSystem() = default;
    ~MenuLogicSystem() = default;

    // The main update loop for our menu logic
    void Update(float dt) override;

    std::string GetName() override { return "MenuLogicSystem"; }
    void Initialize() override {}

private:
};