/* Start Header ************************************************************************/
/*!
\file       MenuLogic.cpp
\author     Jonathan Fam Qian Wei [100% - 102 lines]
\par        Fam.q@digipen.edu
\date       October, 2, 2025

Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*/
/* End Header **************************************************************************/

#include "MenuLogic.h"
#include <iostream>


void MenuLogicSystem::Update(float dt)
{
    double mouseX = InputManager::GetMouseWorldX();
    double mouseY = InputManager::GetMouseWorldY();
    bool isClicked = InputManager::IsMouseLeftClicked();

    auto& factory = GameObjectFactory::Instance();
    std::string currentHover = "";

    // Check all buttons for intersection
    if (IsMouseOver(factory.FindGameObject("PlayButton"), mouseX, mouseY))
    {
        currentHover = "PlayButton";
    }
    else if (IsMouseOver(factory.FindGameObject("SettingsButton"), mouseX, mouseY))
    {
        currentHover = "SettingsButton";
    }
    else if (IsMouseOver(factory.FindGameObject("QuitButton"), mouseX, mouseY))
    {
        currentHover = "QuitButton";
    }

   
    // If the hovered button has changed since last frame
    if (currentHover != hoveredButtonName)
    {
        // Revert the PREVIOUS button texture to normal 
        if (!hoveredButtonName.empty())
        {
            SetButtonTexture(hoveredButtonName, false);
        }

        if (!currentHover.empty())
        {
            SetButtonTexture(currentHover, true);
            // Play hover sound once when entering the button
  
            CORE->GetAudioEngine()->loadSound("menu_hover", "../Assets/Audio/hovering_Button.wav", false);
            CORE->GetAudioEngine()->playSound("menu_hover");
        }

        // Update tracked state
        hoveredButtonName = currentHover;
    }

    // Only process clicks if we are actually hovering over a button
    if (isClicked && !hoveredButtonName.empty())
    {
        // Play Click Sound
        CORE->GetAudioEngine()->loadSound("menu_click", "../Assets/Audio/Menu_Button_Press.wav",false);
        CORE->GetAudioEngine()->playSound("menu_click");

        if (hoveredButtonName == "PlayButton")
        {
            std::cout << "PlayButton Clicked!" << std::endl;
            CORE->RequestSceneLoad("../Assets/Scenes/Prefab_TestScene.json");
        }
        else if (hoveredButtonName == "SettingsButton")
        {
            std::cout << "SettingsButton Clicked!" << std::endl;
            CORE->RequestSceneLoad("../Assets/Scenes/Prefab_TestScene2.json");
        }
        else if (hoveredButtonName == "QuitButton")
        {
            std::cout << "QuitButton Clicked!" << std::endl;
            CORE->RequestSceneLoad("../Assets/Scenes/Prefab_TestScene3.json");
        }
    }
}

bool MenuLogicSystem::IsMouseOver(GameObject* button, double mouseX, double mouseY)
{
    if (!button) return false;
    auto* tx = button->GetComponent<TransformComponent>();
    if (!tx) return false;

    return IsAreaClicked(tx->posX, tx->posY, tx->scaleX, tx->scaleY, mouseX, mouseY);
}

//  swap textures
void MenuLogicSystem::SetButtonTexture(std::string buttonName, bool isHovered)
{
    auto& factory = GameObjectFactory::Instance();
    GameObject* btn = factory.FindGameObject(buttonName);
    if (!btn) return;

    auto* rc = btn->GetComponent<RenderComponent>();
    if (!rc) return;

    std::string texturePath;

    if (buttonName == "PlayButton") {
        texturePath = isHovered ? "../Assets/Menu_Play_Hover.png" : "../Assets/Menu_Play.png";
    }
    else if (buttonName == "SettingsButton") {
        texturePath = isHovered ? "../Assets/Menu_Settings_Hover.png" : "../Assets/Menu_Settings.png";
    }
    else if (buttonName == "QuitButton") {
        texturePath = isHovered ? "../Assets/Menu_Quit_Hover.png" : "../Assets/Menu_Quit.png";
    }

  
    unsigned int newtex = GraphicsSystem::LoadTexture(texturePath.c_str());
    rc->SetTextureID(newtex);
}