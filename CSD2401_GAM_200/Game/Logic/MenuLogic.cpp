/* Start Header ************************************************************************/
/*!
\file       MenuLogic.cpp
\author     Jonathan Fam Qian Wei [24% - 83 lines],
            Lee Hwee Min [5% - 16 lines],
            Leck Xin Yee [71% - 253 lines]
\par        Fam.q@digipen.edu,
            l.hweemin@digipen.edu,
            xinyee.leck@digipen.edu
\date       October, 2, 2025

Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*/
/* End Header **************************************************************************/

#include "MenuLogic.h"
#include <iostream>

std::string introPath = "../Assets/Cutscene.mpeg";
std::string creditsPath = "../Assets/Credits.mpeg";

// abit overkill but if we want more screens in the menu this is better
enum MenuScreens {
    Home = 0,       // Normal landing page
    Settings,
    LevelSelect
};
MenuScreens currScreen = MenuScreens::Home;

void MenuLogicSystem::Update(float dt)
{
    // Update audio fade
    AudioManager* audioManager = CORE->GetAudioManager();
    if (audioManager)
    {
        audioManager->UpdateFade(dt);
    }

    // if a scene change is pending, count down timer first so the click sound can be played
    if (pendingSceneChange)
    {
        pendingTimer -= dt;
        if (pendingTimer <= 0.0f)                       // timer delay to play the sound effect before loading into scene
        {
            if (currScreen == MenuScreens::Home)
            {
                if (hoveredButtonName == "PlayButton")
                {
                    std::cout << "PlayButton Clicked!" << std::endl;
                    currScreen = MenuScreens::LevelSelect;
                }
                else if (hoveredButtonName == "SettingsButton")
                {
                    std::cout << "SettingsButton Clicked!" << std::endl;
                    currScreen = MenuScreens::Settings;
                }
                else if (hoveredButtonName == "CreditsButton")
                {
                    std::cout << "CreditsButton Clicked!" << std::endl;
                    // to add credits screen here
                    CutsceneSystem* cutscene = CORE->GetCutsceneSystem();
                    if (!cutscene->IsPlaying()) {
                        cutscene->PlayVideo(creditsPath, "CreditsAudio");
                    }
                }
                else if (hoveredButtonName == "QuitButton")
                {
                    std::cout << "QuitButton Clicked!" << std::endl;
                #ifndef _DEBUG
					glfwSetWindowShouldClose(glfwGetCurrentContext(), GLFW_TRUE);
                #else
                    CORE->GetLevelEditor()->StopPlayMode();
                #endif          
                }
            }
            else if (currScreen == MenuScreens::LevelSelect)
            {
                if (hoveredButtonName == "LevelSelectedButton" && !selectedLevelScenePath.empty())
                {
                    std::cout << "Starting level: " << selectedLevelScenePath << std::endl;
                    
                    CutsceneSystem* cutscene = CORE->GetCutsceneSystem();
                    if (!cutscene->HasPlayed(introPath)) {
                        if (!cutscene->IsPlaying()) {
                            cutscene->PlayVideo(introPath, "CutsceneAudio");
                            currScreen = MenuScreens::Home;
                            // Don't fade back in - let the fade out complete
                            std::cout << "[MenuLogic] Playing cutscene, music remains faded out" << std::endl;
                        }
                    }
                    CORE->RequestSceneLoad(selectedLevelScenePath, true);
                }
            }

            // clear
            pendingSceneChange = false;
            hoveredButtonName.clear();
        }

        return;
    }

    if (currScreen == MenuScreens::Home)
    {
        if (GameObjectFactory::Instance().FindGameObject("Screen_Settings"))
        {
            GameObjectFactory::Instance().FindGameObject("Screen_Settings")->SetActive(false);
        }
        if (GameObjectFactory::Instance().FindGameObject("Screen_LevelSelect"))
        {
            GameObjectFactory::Instance().FindGameObject("Screen_LevelSelect")->SetActive(false);
        }
        if (GameObjectFactory::Instance().FindGameObject("Screen_TitleButtons"))
        {
            GameObjectFactory::Instance().FindGameObject("Screen_TitleButtons")->SetActive(true);
        }

        // Hide settings text labels when not on settings screen
        if (auto* masterText = GameObjectFactory::Instance().FindGameObject("MasterText"))
            masterText->SetActive(false);
        if (auto* musicText = GameObjectFactory::Instance().FindGameObject("MusicText"))
            musicText->SetActive(false);
        if (auto* sfxText = GameObjectFactory::Instance().FindGameObject("SFXText"))
            sfxText->SetActive(false);
    }
    else if (currScreen == MenuScreens::Settings)
    {
        //hoveredButtonName.clear();
        if (GameObjectFactory::Instance().FindGameObject("Screen_Settings"))
        {
            GameObjectFactory::Instance().FindGameObject("Screen_Settings")->SetActive(true);
        }
        if (GameObjectFactory::Instance().FindGameObject("Screen_LevelSelect"))
        {
            GameObjectFactory::Instance().FindGameObject("Screen_LevelSelect")->SetActive(false);
        }
        if (GameObjectFactory::Instance().FindGameObject("Screen_TitleButtons"))
        {
            GameObjectFactory::Instance().FindGameObject("Screen_TitleButtons")->SetActive(false);
        }

        // Show settings text labels when on settings screen
        if (auto* masterText = GameObjectFactory::Instance().FindGameObject("MasterText"))
            masterText->SetActive(true);
        if (auto* musicText = GameObjectFactory::Instance().FindGameObject("MusicText"))
            musicText->SetActive(true);
        if (auto* sfxText = GameObjectFactory::Instance().FindGameObject("SFXText"))
            sfxText->SetActive(true);
        //return;
    }
    else if (currScreen == MenuScreens::LevelSelect) 
    {
        if (GameObjectFactory::Instance().FindGameObject("Screen_Settings"))
        {
            GameObjectFactory::Instance().FindGameObject("Screen_Settings")->SetActive(false);
        }
        if (GameObjectFactory::Instance().FindGameObject("Screen_LevelSelect"))
        {
            GameObjectFactory::Instance().FindGameObject("Screen_LevelSelect")->SetActive(true);
        }
        if (GameObjectFactory::Instance().FindGameObject("Screen_TitleButtons"))
        {
            GameObjectFactory::Instance().FindGameObject("Screen_TitleButtons")->SetActive(false);
        }

        // Hide settings text labels when not on settings screen
        if (auto* masterText = GameObjectFactory::Instance().FindGameObject("MasterText"))
            masterText->SetActive(false);
        if (auto* musicText = GameObjectFactory::Instance().FindGameObject("MusicText"))
            musicText->SetActive(false);
        if (auto* sfxText = GameObjectFactory::Instance().FindGameObject("SFXText"))
            sfxText->SetActive(false);
    }

    double mouseX = InputManager::GetMouseWorldX();
    double mouseY = InputManager::GetMouseWorldY();
    bool isClicked = InputManager::IsMouseLeftClicked();
    bool isMouseDown = glfwGetMouseButton(glfwGetCurrentContext(), GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS;

    auto& factory = GameObjectFactory::Instance();
    std::string currentHover = "";
    std::string currentPressed = "";

    // Depending on scene, check for which buttons have been pressed
    switch (currScreen)
    {
    case MenuScreens::Home:
        if (IsMouseOver(factory.FindGameObject("PlayButton"), mouseX, mouseY))
        {
            currentHover = "PlayButton";
        }
        else if (IsMouseOver(factory.FindGameObject("SettingsButton"), mouseX, mouseY))
        {
            currentHover = "SettingsButton";
        }
        else if (IsMouseOver(factory.FindGameObject("CreditsButton"), mouseX, mouseY))
        {
            currentHover = "CreditsButton";
        }
        else if (IsMouseOver(factory.FindGameObject("QuitButton"), mouseX, mouseY))
        {
            currentHover = "QuitButton";
        }
        break;
    case MenuScreens::Settings:
        // do settings stuff here
        if (IsMouseOver(factory.FindGameObject("MasterMuteButton"), mouseX, mouseY))
        {
            currentHover = "MasterMuteButton";
        }
        else if (IsMouseOver(factory.FindGameObject("MusicMuteButton"), mouseX, mouseY))
        {
            currentHover = "MusicMuteButton";
        }
        else if (IsMouseOver(factory.FindGameObject("SFXMuteButton"), mouseX, mouseY))
        {
            currentHover = "SFXMuteButton";
        }
        else if (IsMouseOver(factory.FindGameObject("BackButton"), mouseX, mouseY))
        {
            currentHover = "BackButton";
        }
        break;
    case MenuScreens::LevelSelect:
        if (IsMouseOver(factory.FindGameObject("Level1Button"), mouseX, mouseY))
        {
            currentHover = "Level1Button";
        }
        else if (IsMouseOver(factory.FindGameObject("Level2Button"), mouseX, mouseY))
        {
            currentHover = "Level2Button";
        }
        else if (IsMouseOver(factory.FindGameObject("Level3Button"), mouseX, mouseY))
        {
            currentHover = "Level3Button";
        }
        else if (IsMouseOver(factory.FindGameObject("LevelSelectedButton"), mouseX, mouseY))
        {
            currentHover = "LevelSelectedButton";
        }
        else if (IsMouseOver(factory.FindGameObject("Level_BackButton"), mouseX, mouseY))
        {
            currentHover = "Level_BackButton";
        }
        break;
    default:
        break;
    }

    // Check if mouse is being pressed on the hovered button
    if (isMouseDown && !currentHover.empty())
    {
        currentPressed = currentHover;
    }

    // If the pressed button has changed since last frame
    if (currentPressed != pressedButtonName)
    {
        // Revert the PREVIOUS pressed button texture
        if (!pressedButtonName.empty())
        {
            // Check if still hovering the old pressed button
            bool stillHovering = (pressedButtonName == hoveredButtonName);
            SetButtonTexture(pressedButtonName, stillHovering, false);
        }

        // Update tracked pressed state
        pressedButtonName = currentPressed;
    }

    // If the hovered button has changed since last frame
    if (currentHover != hoveredButtonName)
    {
        // Revert the PREVIOUS button texture to normal
        if (!hoveredButtonName.empty() && hoveredButtonName != pressedButtonName)
        {
            SetButtonTexture(hoveredButtonName, false, false);
        }

        if (!currentHover.empty())
        {
            // If pressed, show pressed texture, otherwise show hover texture
            bool isCurrentPressed = (isMouseDown && currentPressed == currentHover);
            SetButtonTexture(currentHover, true, isCurrentPressed);
            // Play hover sound once when entering the button

            AudioManager* localAudioManager = CORE->GetAudioManager();
            if (localAudioManager)
            {
                localAudioManager->TriggerEvent("UI_Hover");
            }
        }

        // Update tracked state
        hoveredButtonName = currentHover;
    }
    else if (!currentHover.empty())
    {
        // Even if hover didn't change, update texture if pressed state changed
        bool isCurrentPressed = (isMouseDown && currentPressed == currentHover);
        bool wasPressed = (!isCurrentPressed && pressedButtonName == currentHover);
        if (isCurrentPressed != wasPressed)
        {
            SetButtonTexture(currentHover, true, isCurrentPressed);
        }
    }

    // Only process clicks if we are actually hovering over a button
    if (isClicked && !hoveredButtonName.empty())
    {
        AudioManager* localAudioManager = CORE->GetAudioManager();

        if (currScreen == MenuScreens::LevelSelect)
        {
            if (hoveredButtonName == "Level1Button")
            {
                if (localAudioManager) localAudioManager->TriggerEvent("UI_Click");
                SelectLevel("Level1Button", "../Assets/Scenes/M4_LEVEL_FIXED.json"); 
                return;
            }
            else if (hoveredButtonName == "Level2Button")
            {
                if (localAudioManager) localAudioManager->TriggerEvent("UI_Click");
                SelectLevel("Level2Button", "../Assets/Scenes/Level2.json");
                return;
            }
            else if (hoveredButtonName == "Level3Button")
            {
                if (localAudioManager) localAudioManager->TriggerEvent("UI_Click");
                SelectLevel("Level3Button", "../Assets/Scenes/Level3.json");
                return;
            }
            else if (hoveredButtonName == "LevelSelectedButton")
            {
                // Start only works if a level was selected first
                if (selectedLevelButtonName.empty() || selectedLevelScenePath.empty())
                {               
                    std::cout << "No level selected yet.\n";
                    SetButtonTexture("LevelSelectedButton", false, false);
                    return;
                }

                if (localAudioManager) localAudioManager->TriggerEvent("UI_Press");

                pendingSceneChange = true;
                pendingTimer = 0.2f; // just enough for button sound
                return;
            }
            else if (hoveredButtonName == "Level_BackButton")
            {
                if (localAudioManager) localAudioManager->TriggerEvent("UI_Click");

                currScreen = MenuScreens::Home;
                selectedLevelButtonName.clear();
                selectedLevelScenePath.clear();
                hoveredButtonName.clear();
                pressedButtonName.clear();
                RefreshLevelSelectButtons();
                return;
            }
        }

        if (localAudioManager)
        {
            if (hoveredButtonName == "PlayButton")
            {
                localAudioManager->TriggerEvent("UI_Press");
            }
            else if (hoveredButtonName == "QuitButton")
            {
                localAudioManager->TriggerEvent("UI_Quit");
            }
            else
            {
                localAudioManager->TriggerEvent("UI_Click");
            }
            std::cout << "menu button clicking sound is triggered" << std::endl;
        }

        // Start small delay before loading scene
        if (hoveredButtonName != "SettingsButton" && hoveredButtonName != "MasterMuteButton" &&
            hoveredButtonName != "MusicMuteButton" && hoveredButtonName != "SFXMuteButton" &&
            hoveredButtonName != "BackButton" && hoveredButtonName != "CreditsButton" &&
            hoveredButtonName != "Level_BackButton")
        {
            // Start audio fade out for scene transitions
            if (hoveredButtonName == "PlayButton" && audioManager)
            {
                //currScreen = MenuScreens::LevelSelect;
                audioManager->FadeOutCategory("music", 1.0f);  // Fade out over 1.0 sec
                std::cout << "[MenuLogic] Started music fade out" << std::endl;
            }

            pendingSceneChange = true;
            pendingTimer = 2.0f;                       // Wait 3.0s so 2.5s fade completes before scene loads
        }
        else
        {
            if (hoveredButtonName == "MasterMuteButton")
            {
                //AudioManager* audioManager = CORE->GetAudioManager();
                if (audioManager)
                {
                    audioManager->ToggleMasterMute();

                }
                SetButtonTexture("MasterMuteButton", true, false); // Update texture after toggling
            }
            else if (hoveredButtonName == "MusicMuteButton")
            {
                if (audioManager)
                {
                    audioManager->ToggleCategoryMute("music");
                }
                SetButtonTexture("MusicMuteButton", true, false); // Update texture after toggling
            }
            else if (hoveredButtonName == "SFXMuteButton")
            {
                if (audioManager)
                {
                    audioManager->ToggleCategoryMute("sfx");
                }
                SetButtonTexture("SFXMuteButton", true, false); // Update texture after toggling
            }
            else if (hoveredButtonName == "BackButton" || hoveredButtonName == "Level_BackButton")
            {
                currScreen = MenuScreens::Home;
            }
            else if (hoveredButtonName == "SettingsButton")
            {
                currScreen = MenuScreens::Settings;
            }
            else if (hoveredButtonName == "CreditsButton")
            {
                std::cout << "CreditsButton Clicked!" << std::endl;
                pendingSceneChange = true;
            }
        }
    }
}

bool MenuLogicSystem::IsMouseOver(GameObject* button, double mouseX, double mouseY)
{
    if (!button) return false;
    auto* tx = button->GetComponent<TransformComponent>();
    if (!tx) return false;

    return IsAreaClicked(tx->posX, tx->posY, tx->scaleX, tx->scaleY, static_cast<float>(mouseX), static_cast<float>(mouseY));
}

void MenuLogicSystem::RefreshLevelSelectButtons()
{
    SetButtonTexture("Level1Button",
        hoveredButtonName == "Level1Button",
        pressedButtonName == "Level1Button");

    SetButtonTexture("Level2Button",
        hoveredButtonName == "Level2Button",
        pressedButtonName == "Level2Button");

    SetButtonTexture("Level3Button",
        hoveredButtonName == "Level3Button",
        pressedButtonName == "Level3Button");

    SetButtonTexture("LevelSelectedButton",
        hoveredButtonName == "LevelSelectedButton",
        pressedButtonName == "LevelSelectedButton");
}

void MenuLogicSystem::SelectLevel(const std::string& buttonName, const std::string& scenePath)
{
    selectedLevelButtonName = buttonName;
    selectedLevelScenePath = scenePath;
    RefreshLevelSelectButtons();
}

//  swap textures
void MenuLogicSystem::SetButtonTexture(std::string buttonName, bool isHovered, bool isPressed)
{
    auto& factory = GameObjectFactory::Instance();
    GameObject* btn = factory.FindGameObject(buttonName);
    if (!btn) return;

    auto* rc = btn->GetComponent<RenderComponent>();
    if (!rc) return;

    std::string texturePath;

    if (buttonName == "PlayButton") {
        if (isPressed) {
            texturePath = "../Assets/Sprites/Icons/Menu_play_NEW_HOVER.png";
        } else if (isHovered) {
            texturePath = "../Assets/Sprites/Icons/Menu_play_NEW_bright.png";
        } else {
            texturePath = "../Assets/Sprites/Icons/Menu_play.png";
        }
    }
    else if (buttonName == "SettingsButton") {
        if (isPressed) {
            texturePath = "../Assets/Sprites/Icons/Menu_settings_NEW_HOVER.png";
        } else if (isHovered) {
            texturePath = "../Assets/Sprites/Icons/Menu_settings_NEW_bright.png";
        } else {
            texturePath = "../Assets/Sprites/Icons/Menu_settings.png";
        }
    }
    else if (buttonName == "QuitButton") {
        if (isPressed) {
            texturePath = "../Assets/Sprites/Icons/Menu_quit_NEW_HOVER.png";
        } else if (isHovered) {
            texturePath = "../Assets/Sprites/Icons/Menu_quit_NEW_bright.png";
        } else {
            texturePath = "../Assets/Sprites/Icons/Menu_quit.png";
        }
    }
    else if (buttonName == "CreditsButton") {
        if (isPressed) {
            texturePath = "../Assets/Sprites/Icons/Menu_credits_NEW_HOVER.png";
        } else if (isHovered) {
            texturePath = "../Assets/Sprites/Icons/Menu_credits_NEW_bright.png";
        } else {
            texturePath = "../Assets/Sprites/Icons/Menu_credits_NEW.png";
        }
    }
    else if (buttonName == "MasterMuteButton") {
        bool isMuted = false;
        AudioManager* audioManager = CORE->GetAudioManager();
        if (audioManager)
        {
            isMuted = audioManager->IsMuted();
        }
        texturePath = isMuted ? "../Assets/Sprites/Icons/Volume_Mute.png" : "../Assets/Sprites/Icons/Volume_Unmute.png";
    }
    else if (buttonName == "MusicMuteButton") {
        bool isMuted = false;
        AudioManager* audioManager = CORE->GetAudioManager();
        if (audioManager)
        {
            isMuted = audioManager->IsCategoryMuted("music");
        }
        texturePath = isMuted ? "../Assets/Sprites/Icons/Volume_Mute.png" : "../Assets/Sprites/Icons/Volume_Unmute.png";
    }
    else if (buttonName == "SFXMuteButton") {
        bool isMuted = false;
        AudioManager* audioManager = CORE->GetAudioManager();
        if (audioManager)
        {
            isMuted = audioManager->IsCategoryMuted("sfx");
        }
        texturePath = isMuted ? "../Assets/Sprites/Icons/Volume_Mute.png" : "../Assets/Sprites/Icons/Volume_Unmute.png";
    }
    else if (buttonName == "BackButton" || buttonName == "Level_BackButton") {
        if (isPressed) {
            texturePath = "../Assets/Sprites/Icons/Back_button_dark.png";
        } else if (isHovered) {
            texturePath = "../Assets/Sprites/Icons/Back_button_light.png";
        } else {
            texturePath = "../Assets/Sprites/Icons/Back_button.png";
        }
    }
    else if (buttonName == "Level1Button") 
    {
        bool isSelected = (selectedLevelButtonName == "Level1Button");

        if (isSelected || isPressed || isHovered) {
            texturePath = "../Assets/Sprites/Icons/Level_select_01_light.png";
        }
        else {
            texturePath = "../Assets/Sprites/Icons/Level_select_01_dark.png";
        }
    }
    else if (buttonName == "Level2Button")
    {
        bool isSelected = (selectedLevelButtonName == "Level2Button");

        if (isSelected || isPressed || isHovered) {
            texturePath = "../Assets/Sprites/Icons/Level_select_02_light.png";
        }
        else {
            texturePath = "../Assets/Sprites/Icons/Level_select_02_dark.png";
        }
    }
    else if (buttonName == "Level3Button")
    {
        bool isSelected = (selectedLevelButtonName == "Level3Button");

        if (isSelected || isPressed || isHovered) {
            texturePath = "../Assets/Sprites/Icons/Level_select_03_light.png";
        }
        else {
            texturePath = "../Assets/Sprites/Icons/Level_select_03_dark.png";
        }
    }
    else if (buttonName == "LevelSelectedButton")
    {
        bool canStart = !selectedLevelButtonName.empty();

        if (!canStart) {
            texturePath = "../Assets/Sprites/Icons/win_lose_button_dark.png";
        }
        else if (isPressed || isHovered) {
            texturePath = "../Assets/Sprites/Icons/win_lose_button.png";
        }
        else {
            texturePath = "../Assets/Sprites/Icons/win_lose_button.png";
        }
    }


    unsigned int newtex = GraphicsSystem::LoadTexture(texturePath.c_str());
    rc->SetTextureID(newtex);
}

void MenuLogicSystem::Reset()
{
    currScreen = MenuScreens::Home;

    hoveredButtonName.clear();
    pressedButtonName.clear();
    selectedLevelButtonName.clear();
    selectedLevelScenePath.clear();
    pendingSceneChange = false;
    pendingTimer = 0.0f;
}
