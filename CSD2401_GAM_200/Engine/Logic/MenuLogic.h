#pragma once
#pragma once

#include "System.h"
#include "factory.h"
#include "Serialization.h"
#include "InputManager.h" //
#include "transform.h"
#include "MathLib.h"
#include "GraphicsSystem.h"
class GraphicsSystem;
class MenuLogicSystem : public ISystem {
public:
    MenuLogicSystem() = default;
    ~MenuLogicSystem() = default;

    // The main update loop for our menu logic
    void Update(float dt) override;


    std::string GetName() override { return "MenuLogicSystem"; }

    void Initialize() override {}
 

private:
    std::string hoveredButtonName = "";
    bool IsMouseOver(GameObject* button, double mouseX, double mouseY);
    void SetButtonTexture(std::string buttonName, bool isHovered);
};