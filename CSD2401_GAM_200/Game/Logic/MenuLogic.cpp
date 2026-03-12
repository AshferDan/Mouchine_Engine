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
    Settings
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

    // Check if credits were played and have finished, then restart BGM
    if (creditsWerePlayed)
    {
        CutsceneSystem* cutscene = CORE->GetCutsceneSystem();
        if (cutscene && !cutscene->IsPlaying())
        {
            // Credits finished, restart BGM
            std::cout << "[MenuLogic] Credits finished - Restarting main menu BGM" << std::endl;
            if (audioManager)
            {
                const auto& sounds = audioManager->GetSoundDatabase();
                if (sounds.find("MainMenuBGM") != sounds.end())
                {
                    std::cout << "[MenuLogic] MainMenuBGM found, starting playback" << std::endl;
                    audioManager->StopSound("bgm_channel");
                    audioManager->PlaySound("MainMenuBGM", "bgm_channel");
                    audioManager->FadeInCategory("music", 3.0f);
                    std::cout << "[MenuLogic] Main menu BGM restarted with 3 second fade in" << std::endl;
                }
                else
                {
                    std::cout << "[MenuLogic] ERROR: MainMenuBGM not found in sound database!" << std::endl;
                }
            }
            else
            {
                std::cout << "[MenuLogic] ERROR: AudioManager is null!" << std::endl;
            }

            creditsWerePlayed = false;  // Reset flag
        }
    }

    // if a scene change is pending, count down timer first so the click sound can be played
    if (pendingSceneChange)
    {
        pendingTimer -= dt;
        if (pendingTimer <= 0.0f) // timer delay to play the sound effect before loading into scene
        {
            if (audioManager && audioManager->IsAnyFadeActive())
            {
                pendingTimer = 0.1f; // wait another 100ms and check again
                return;
            }

            if (currScreen == MenuScreens::Home)
            {
                if (hoveredButtonName == "PlayButton")
                {
                    std::cout << "PlayButton Clicked!" << std::endl;
                    std::cout << "[MenuLogic] Fade complete! Loading scene..." << std::endl;

                    CutsceneSystem* cutscene = CORE->GetCutsceneSystem();

                    if (!cutscene->IsPlaying()) {
                        cutscene->PlayVideo(introPath, "CutsceneAudio");
                    }

                    //std::cout << "[MenuLogic] Requesting scene load with AUTO-PLAY = TRUE" << std::endl;
                    //std::cout << "[MenuLogic] Music should already be faded out" << std::endl;
                    CORE->RequestSceneLoad("../Assets/Scenes/M4_LEVEL_FIXED.json", true);
                    std::cout << "[MenuLogic] RequestSceneLoad called" << std::endl;
                }
                else if (hoveredButtonName == "SettingsButton")
                {
                    std::cout << "SettingsButton Clicked!" << std::endl;

                    currScreen = MenuScreens::Settings;
                    //CORE->RequestSceneLoad("../Assets/Scenes/Scene2.json", true);
                }
                else if (hoveredButtonName == "CreditsButton")
                {
                    std::cout << "CreditsButton Clicked!" << std::endl;
                    // to add credits screen here
                    //CORE->RequestSceneLoad("../Assets/Scenes/Credits.json", true);
                }
                else if (hoveredButtonName == "QuitButton")
                {
                    std::cout << "QuitButton Clicked!" << std::endl;
                #ifndef _DEBUG
					glfwSetWindowShouldClose(glfwGetCurrentContext(), GLFW_TRUE);
                #else
                    CORE->GetLevelEditor()->StopPlayMode();
                    //CORE->RequestSceneLoad("../Assets/Scenes/Scene3.json", true);
                #endif
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
        //else if (IsMouseOver(factory.FindGameObject("TestFadeButton"), mouseX, mouseY))
        //{
        //    currentHover = "TestFadeButton";
        //}
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
            hoveredButtonName != "BackButton" && hoveredButtonName != "CreditsButton"
            /*&& hoveredButtonName != "TestFadeButton"*/)
        {
            // Start audio fade out for scene transitions
            if (hoveredButtonName == "PlayButton" && audioManager)
            {
                audioManager->FadeOutCategory("music", 2.5f);  // Fade out over 3.0 sec
                std::cout << "[MenuLogic] Started music fade out (3.0 seconds)" << std::endl;
            }

            pendingSceneChange = true;
            pendingTimer = 2.6f;
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
            else if (hoveredButtonName == "BackButton")
            {
                currScreen = MenuScreens::Home;
            }
            else if (hoveredButtonName == "SettingsButton")
            {
                currScreen = MenuScreens::Settings;
            }
            //else if (hoveredButtonName == "TestFadeButton")
            //{
            //    std::cout << "[TEST] TestFadeButton Clicked - Starting fade out!" << std::endl;
            //    if (audioManager)
            //    {
            //        audioManager->FadeOutCategory("music", 2.0f);  // Fade out over 2 seconds
            //        std::cout << "[TEST] Music fade out started (2 seconds duration)" << std::endl;
            //        testFadeTriggered = true;
            //    }
            //    else
            //    {
            //        std::cout << "[TEST] ERROR: AudioManager is null!" << std::endl;
            //    }
            //}
            else if (hoveredButtonName == "CreditsButton")
            {
                std::cout << "CreditsButton Clicked!" << std::endl;

                CutsceneSystem* cutscene = CORE->GetCutsceneSystem();
                if (!cutscene->IsPlaying()) {
                    cutscene->PlayVideo(creditsPath, "CreditsAudio");
                    creditsWerePlayed = true;  // Set flag to restart BGM after credits
                    std::cout << "[MenuLogic] Credits flag set - will restart BGM when finished" << std::endl;
                }
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
    else if (buttonName == "BackButton") {
        if (isPressed) {
            texturePath = "../Assets/Sprites/Icons/Back_button_dark.png";
        } else if (isHovered) {
            texturePath = "../Assets/Sprites/Icons/Back_button_light.png";
        } else {
            texturePath = "../Assets/Sprites/Icons/Back_button.png";
        }
    }

    unsigned int newtex = GraphicsSystem::LoadTexture(texturePath.c_str());
    rc->SetTextureID(newtex);
}

void MenuLogicSystem::Reset()
{
    currScreen = MenuScreens::Home;
}
