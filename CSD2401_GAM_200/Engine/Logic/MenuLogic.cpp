#include "MenuLogic.h"
#include <iostream> // For testing

void MenuLogicSystem::Update(float dt)
{
    // Only do work if the mouse is actually clicked
    if (InputManager::IsMouseLeftClicked())
    {
        double mouseX = InputManager::GetMouseWorldX();
        double mouseY = InputManager::GetMouseWorldY();

        // std::cout << "Mouse World: (" << mouseX << ", " << mouseY << ")" << std::endl;

        auto& factory = GameObjectFactory::Instance();

        // 1. Check Play Button
        if (IsButtonClicked(factory.FindGameObject("PlayButton"), mouseX, mouseY))
        {
            std::cout << "PlayButton Clicked!" << std::endl;
            CORE->RequestSceneLoad("Prefab_TestScene");
            return;
        }

        // 2. Check Settings Button
        if (IsButtonClicked(factory.FindGameObject("SettingsButton"), mouseX, mouseY))
        {
            std::cout << "SettingsButton Clicked!" << std::endl;
            CORE->RequestSceneLoad("Prefab_TestScene2");
            return;
        }

        // 3. Check Quit Button
        if (IsButtonClicked(factory.FindGameObject("QuitButton"), mouseX, mouseY))
        {
            std::cout << "QuitButton Clicked!" << std::endl;
            CORE->RequestSceneLoad("Prefab_TestScene3");
            return;
        }
    }
}

bool MenuLogicSystem::IsButtonClicked(GameObject* button, double mouseX, double mouseY)
{
    // Step 1: Automate null checks
    if (!button) return false;
    auto* tx = button->GetComponent<TransformComponent>();
    if (!tx) return false;

    // Step 2: Automate calling your math library function
    // It STILL uses your math function here!
    return IsAreaClicked(tx->posX, tx->posY, tx->scaleX, tx->scaleY, mouseX, mouseY);
}