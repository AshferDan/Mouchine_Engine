#include "MenuLogic.h"
#include <iostream>

// Optional: Define your generic generic texture paths if they follow a pattern, 
// or just hardcode them in the helper function below.

void MenuLogicSystem::Update(float dt)
{
    double mouseX = InputManager::GetMouseWorldX();
    double mouseY = InputManager::GetMouseWorldY();
    bool isClicked = InputManager::IsMouseLeftClicked();

    auto& factory = GameObjectFactory::Instance();

    // --- 1. HOVER DETECTION ---
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
        // Revert the PREVIOUS button texture to normal (if there was one)
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

// Renamed from IsButtonClicked to IsMouseOver for clarity, 
// since it only checks intersection, not the actual click action.
bool MenuLogicSystem::IsMouseOver(GameObject* button, double mouseX, double mouseY)
{
    if (!button) return false;
    auto* tx = button->GetComponent<TransformComponent>();
    if (!tx) return false;

    return IsAreaClicked(tx->posX, tx->posY, tx->scaleX, tx->scaleY, mouseX, mouseY);
}

// Helper function to swap textures
void MenuLogicSystem::SetButtonTexture(std::string buttonName, bool isHovered)
{
    auto& factory = GameObjectFactory::Instance();
    GameObject* btn = factory.FindGameObject(buttonName);
    if (!btn) return;

    // Assuming you have a RenderComponent that holds the texture path or ID.
    // Adjust 'RenderComponent' and 'setTexture' to match your actual engine's names.
    auto* rc = btn->GetComponent<RenderComponent>();
    if (!rc) return;

    std::string texturePath;

    // Define your texture swaps here based on the button name
    if (buttonName == "PlayButton") {
        texturePath = isHovered ? "../Assets/Menu_Play_Hover.png" : "../Assets/Menu_Play.png";
    }
    else if (buttonName == "SettingsButton") {
        texturePath = isHovered ? "../Assets/Menu_Settings_Hover.png" : "../Assets/Menu_Settings.png";
    }
    else if (buttonName == "QuitButton") {
        texturePath = isHovered ? "../Assets/Menu_Quit_Hover.png" : "../Assets/Menu_Quit.png";
    }

    // Hypothetical function to update the texture on your render component
    unsigned int newtex = GraphicsSystem::LoadTexture(texturePath.c_str());
    rc->SetTextureID(newtex);
}