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
#include "GhostSystem.h"
#include "CameraManager.h"
#include "TextComponent.h"
#include <iostream>
#include <filesystem>
#include <algorithm>
#include <fstream>
#include <nlohmann/json.hpp>

std::string introPath = "../Assets/Cutscene.mpeg";
std::string creditsPath = "../Assets/Credits.mpeg";

// abit overkill but if we want more screens in the menu this is better
enum MenuScreens
{
    Home = 0, // Normal landing page
    Settings,
    QUIT,
    ReplayPicker
};
MenuScreens currScreen = MenuScreens::Home;

void MenuLogicSystem::Update(float dt)
{
    // Update audio fade
    AudioManager *audioManager = CORE->GetAudioManager();
    if (audioManager)
    {
        audioManager->UpdateFade(dt);
    }

    // Check if credits were played and have finished, then restart BGM
    if (creditsWerePlayed)
    {
        CutsceneSystem *cutscene = CORE->GetCutsceneSystem();
        if (cutscene && !cutscene->IsPlaying())
        {
            // Credits finished, restart BGM
            std::cout << "[MenuLogic] Credits finished - Restarting main menu BGM" << std::endl;
            if (audioManager)
            {
                const auto &sounds = audioManager->GetSoundDatabase();
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

            creditsWerePlayed = false; // Reset flag
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

                    CutsceneSystem *cutscene = CORE->GetCutsceneSystem();

                    if (!cutscene->IsPlaying())
                    {
                        cutscene->PlayVideo(introPath, "CutsceneAudio");
                    }

                    // std::cout << "[MenuLogic] Requesting scene load with AUTO-PLAY = TRUE" << std::endl;
                    // std::cout << "[MenuLogic] Music should already be faded out" << std::endl;
                    CORE->RequestSceneLoad("../Assets/Scenes/M4_LEVEL_FIXED.json", true);
                    std::cout << "[MenuLogic] RequestSceneLoad called" << std::endl;
                }
                else if (hoveredButtonName == "SettingsButton")
                {
                    std::cout << "SettingsButton Clicked!" << std::endl;

                    currScreen = MenuScreens::Settings;
                    // CORE->RequestSceneLoad("../Assets/Scenes/Scene2.json", true);
                }
                else if (hoveredButtonName == "CreditsButton")
                {
                    std::cout << "CreditsButton Clicked!" << std::endl;
                    // to add credits screen here
                    // CORE->RequestSceneLoad("../Assets/Scenes/Credits.json", true);
                }
                else if (hoveredButtonName == "ReplayButton")
                {
                    // ReplayButton now opens the picker immediately, not via pendingSceneChange.
                    // This branch is kept for safety but should not be reached.
                }
                else if (hoveredButtonName == "YesButton")
                {
                    std::cout << "YesButton Clicked! Quitting..." << std::endl;
#ifndef _DEBUG
                    glfwSetWindowShouldClose(glfwGetCurrentContext(), GLFW_TRUE);
#else
                    CORE->GetLevelEditor()->StopPlayMode();
                    // CORE->RequestSceneLoad("../Assets/Scenes/Scene3.json", true);
#endif
                }
            }

            else if (currScreen == MenuScreens::ReplayPicker)
            {
                if (!m_selectedReplay.empty())
                {
                    HideReplayPicker();
                    if (GhostSystem *ghost = CORE->GetGhostSystem())
                    {
                        ghost->Configure("Hero", "GhostHero", "Hero_vfx", "GhostHero_vfx");
                        ghost->RequestReplayOnLoad(m_selectedReplay);
                    }
                    CORE->RequestSceneLoad("../Assets/Scenes/M4_LEVEL_FIXED.json", true);
                    m_selectedReplay.clear();
                    currScreen = MenuScreens::Home;
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

        if (GameObjectFactory::Instance().FindGameObject("Screen_QuitButton"))
        {

            GameObjectFactory::Instance().FindGameObject("Screen_QuitButton")->SetActive(false);
        }

        // Hide settings text labels when not on settings screen
        if (auto *masterText = GameObjectFactory::Instance().FindGameObject("MasterText"))
            masterText->SetActive(false);
        if (auto *musicText = GameObjectFactory::Instance().FindGameObject("MusicText"))
            musicText->SetActive(false);
        if (auto *sfxText = GameObjectFactory::Instance().FindGameObject("SFXText"))
            sfxText->SetActive(false);
    }
    else if (currScreen == MenuScreens::Settings)
    {
        // hoveredButtonName.clear();
        if (GameObjectFactory::Instance().FindGameObject("Screen_Settings"))
        {
            GameObjectFactory::Instance().FindGameObject("Screen_Settings")->SetActive(true);
        }
        if (GameObjectFactory::Instance().FindGameObject("Screen_TitleButtons"))
        {
            GameObjectFactory::Instance().FindGameObject("Screen_TitleButtons")->SetActive(false);
        }
        if (GameObjectFactory::Instance().FindGameObject("Screen_QuitButton"))
        {

            GameObjectFactory::Instance().FindGameObject("Screen_QuitButton")->SetActive(false);
        }
        // Show settings text labels when on settings screen
        if (auto *masterText = GameObjectFactory::Instance().FindGameObject("MasterText"))
            masterText->SetActive(true);
        if (auto *musicText = GameObjectFactory::Instance().FindGameObject("MusicText"))
            musicText->SetActive(true);
        if (auto *sfxText = GameObjectFactory::Instance().FindGameObject("SFXText"))
            sfxText->SetActive(true);
        // return;
    }
    else if (currScreen == MenuScreens::QUIT)
    {
        if (GameObjectFactory::Instance().FindGameObject("Screen_Settings"))
        {
            GameObjectFactory::Instance().FindGameObject("Screen_Settings")->SetActive(false);
        }
        if (GameObjectFactory::Instance().FindGameObject("Screen_TitleButtons"))
        {
            GameObjectFactory::Instance().FindGameObject("Screen_TitleButtons")->SetActive(false);
        }
        if (GameObjectFactory::Instance().FindGameObject("Screen_QuitButton"))
        {

            GameObjectFactory::Instance().FindGameObject("Screen_QuitButton")->SetActive(true);
        }
    }
    else if (currScreen == MenuScreens::ReplayPicker)
    {
        if (GameObjectFactory::Instance().FindGameObject("Screen_Settings"))
            GameObjectFactory::Instance().FindGameObject("Screen_Settings")->SetActive(false);
        if (GameObjectFactory::Instance().FindGameObject("Screen_TitleButtons"))
            GameObjectFactory::Instance().FindGameObject("Screen_TitleButtons")->SetActive(false);
        if (GameObjectFactory::Instance().FindGameObject("Screen_QuitButton"))
            GameObjectFactory::Instance().FindGameObject("Screen_QuitButton")->SetActive(false);
        if (auto *masterText = GameObjectFactory::Instance().FindGameObject("MasterText"))
            masterText->SetActive(false);
        if (auto *musicText = GameObjectFactory::Instance().FindGameObject("MusicText"))
            musicText->SetActive(false);
        if (auto *sfxText = GameObjectFactory::Instance().FindGameObject("SFXText"))
            sfxText->SetActive(false);

        UpdateReplayPickerPositions();
    }

    double mouseX = InputManager::GetMouseWorldX();
    double mouseY = InputManager::GetMouseWorldY();
    bool isClicked = InputManager::IsMouseLeftClicked();
    bool isMouseDown = glfwGetMouseButton(glfwGetCurrentContext(), GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS;

    auto &factory = GameObjectFactory::Instance();
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

        else if (IsMouseOver(factory.FindGameObject("ReplayButton"), mouseX, mouseY))
        {
            currentHover = "ReplayButton";
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
    case MenuScreens::QUIT:
        if (IsMouseOver(factory.FindGameObject("YesButton"), mouseX, mouseY))
        {
            currentHover = "YesButton";
        }
        else if (IsMouseOver(factory.FindGameObject("NoButton"), mouseX, mouseY))
        {
            currentHover = "NoButton";
        }
        break;
    case MenuScreens::ReplayPicker:
        if (IsMouseOver(factory.FindGameObject("ReplayPicker_Slot0"), mouseX, mouseY))
            currentHover = "ReplayPicker_Slot0";
        else if (IsMouseOver(factory.FindGameObject("ReplayPicker_Slot1"), mouseX, mouseY))
            currentHover = "ReplayPicker_Slot1";
        else if (IsMouseOver(factory.FindGameObject("ReplayPicker_Slot2"), mouseX, mouseY))
            currentHover = "ReplayPicker_Slot2";
        else if (IsMouseOver(factory.FindGameObject("ReplayPicker_Back"), mouseX, mouseY))
            currentHover = "ReplayPicker_Back";
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

            AudioManager *localAudioManager = CORE->GetAudioManager();
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
        AudioManager *localAudioManager = CORE->GetAudioManager();
        if (localAudioManager)
        {
            if (hoveredButtonName == "PlayButton")
            {
                localAudioManager->TriggerEvent("UI_Press");
            }
            else if (hoveredButtonName == "QuitButton" || hoveredButtonName == "YesButton")
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
            hoveredButtonName != "QuitButton" && hoveredButtonName != "YesButton" && hoveredButtonName != "NoButton" &&
            hoveredButtonName != "ReplayButton" && hoveredButtonName != "ReplayPicker_Back" &&
            hoveredButtonName != "ReplayPicker_Slot0" && hoveredButtonName != "ReplayPicker_Slot1" &&
            hoveredButtonName != "ReplayPicker_Slot2")
        {
            // Start audio fade out for scene transitions
            if (hoveredButtonName == "PlayButton" && audioManager)
            {
                audioManager->FadeOutCategory("music", 2.5f);
                std::cout << "[MenuLogic] Started music fade out (2.5 seconds)" << std::endl;
            }

            pendingSceneChange = true;
            pendingTimer = 2.6f;
        }
        else
        {
            if (hoveredButtonName == "MasterMuteButton")
            {
                // AudioManager* audioManager = CORE->GetAudioManager();
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
            else if (hoveredButtonName == "ReplayButton")
            {
                std::cout << "ReplayButton Clicked! Opening replay picker..." << std::endl;
                ShowReplayPicker();
            }
            else if (hoveredButtonName == "ReplayPicker_Back")
            {
                HideReplayPicker();
                currScreen = MenuScreens::Home;
            }
            else if (hoveredButtonName == "ReplayPicker_Slot0" ||
                     hoveredButtonName == "ReplayPicker_Slot1" ||
                     hoveredButtonName == "ReplayPicker_Slot2")
            {
                int idx = hoveredButtonName.back() - '0';
                if (idx >= 0 && idx < static_cast<int>(m_replayNames.size()))
                {
                    m_selectedReplay = m_replayNames[idx];
                    std::cout << "[MenuLogic] Replay selected: " << m_selectedReplay << std::endl;
                    if (audioManager)
                        audioManager->FadeOutCategory("music", 2.5f);
                    pendingSceneChange = true;
                    pendingTimer = 2.6f;
                }
                else
                {
                    std::cout << "[MenuLogic] Clicked empty replay slot." << std::endl;
                }
            }
            // else if (hoveredButtonName == "TestFadeButton")
            //{
            //     std::cout << "[TEST] TestFadeButton Clicked - Starting fade out!" << std::endl;
            //     if (audioManager)
            //     {
            //         audioManager->FadeOutCategory("music", 2.0f);  // Fade out over 2 seconds
            //         std::cout << "[TEST] Music fade out started (2 seconds duration)" << std::endl;
            //         testFadeTriggered = true;
            //     }
            //     else
            //     {
            //         std::cout << "[TEST] ERROR: AudioManager is null!" << std::endl;
            //     }
            // }
            else if (hoveredButtonName == "CreditsButton")
            {
                std::cout << "CreditsButton Clicked!" << std::endl;

                CutsceneSystem *cutscene = CORE->GetCutsceneSystem();
                if (!cutscene->IsPlaying())
                {
                    cutscene->PlayVideo(creditsPath, "CreditsAudio");
                    creditsWerePlayed = true; // Set flag to restart BGM after credits
                    std::cout << "[MenuLogic] Credits flag set - will restart BGM when finished" << std::endl;
                }
            }
            else if (hoveredButtonName == "QuitButton")
            {
                std::cout << "QuitButton Clicked! Showing confirmation..." << std::endl;
                currScreen = MenuScreens::QUIT;
            }
            else if (hoveredButtonName == "YesButton")
            {
                std::cout << "YesButton Clicked! Quitting..." << std::endl;
#ifndef _DEBUG
                glfwSetWindowShouldClose(glfwGetCurrentContext(), GLFW_TRUE);
#else
                CORE->GetLevelEditor()->StopPlayMode();
#endif
            }
            else if (hoveredButtonName == "NoButton")
            {
                std::cout << "NoButton Clicked! Returning to menu..." << std::endl;
                currScreen = MenuScreens::Home;
            }
        }
    }
}

bool MenuLogicSystem::IsMouseOver(GameObject *button, double mouseX, double mouseY)
{
    if (!button)
        return false;
    auto *tx = button->GetComponent<TransformComponent>();
    if (!tx)
        return false;

    return IsAreaClicked(tx->posX, tx->posY, tx->scaleX, tx->scaleY, static_cast<float>(mouseX), static_cast<float>(mouseY));
}

//  swap textures
void MenuLogicSystem::SetButtonTexture(std::string buttonName, bool isHovered, bool isPressed)
{
    auto &factory = GameObjectFactory::Instance();
    GameObject *btn = factory.FindGameObject(buttonName);
    if (!btn)
        return;

    auto *rc = btn->GetComponent<RenderComponent>();
    if (!rc)
        return;

    std::string texturePath;

    if (buttonName == "PlayButton")
    {
        if (isPressed)
        {
            texturePath = "../Assets/Sprites/Icons/Menu_play_NEW_HOVER.png";
        }
        else if (isHovered)
        {
            texturePath = "../Assets/Sprites/Icons/Menu_play_NEW_bright.png";
        }
        else
        {
            texturePath = "../Assets/Sprites/Icons/Menu_play.png";
        }
    }
    else if (buttonName == "SettingsButton")
    {
        if (isPressed)
        {
            texturePath = "../Assets/Sprites/Icons/Menu_settings_NEW_HOVER.png";
        }
        else if (isHovered)
        {
            texturePath = "../Assets/Sprites/Icons/Menu_settings_NEW_bright.png";
        }
        else
        {
            texturePath = "../Assets/Sprites/Icons/Menu_settings.png";
        }
    }
    else if (buttonName == "QuitButton")
    {
        if (isPressed)
        {
            texturePath = "../Assets/Sprites/Icons/Menu_quit_NEW_HOVER.png";
        }
        else if (isHovered)
        {
            texturePath = "../Assets/Sprites/Icons/Menu_quit_NEW_bright.png";
        }
        else
        {
            texturePath = "../Assets/Sprites/Icons/Menu_quit.png";
        }
    }

    else if (buttonName == "ReplayButton")
    {
        if (isPressed)
        {
            texturePath = "../Assets/Sprites/Icons/Replay_button_dark.png";
        }
        else if (isHovered)
        {
            texturePath = "../Assets/Sprites/Icons/Replay_button_light.png";
        }
        else
        {
            texturePath = "../Assets/Sprites/Icons/Replay_button.png";
        }
    }

    else if (buttonName == "CreditsButton")
    {
        if (isPressed)
        {
            texturePath = "../Assets/Sprites/Icons/Menu_credits_NEW_HOVER.png";
        }
        else if (isHovered)
        {
            texturePath = "../Assets/Sprites/Icons/Menu_credits_NEW_bright.png";
        }
        else
        {
            texturePath = "../Assets/Sprites/Icons/Menu_credits_NEW.png";
        }
    }
    else if (buttonName == "MasterMuteButton")
    {
        bool isMuted = false;
        AudioManager *audioManager = CORE->GetAudioManager();
        if (audioManager)
        {
            isMuted = audioManager->IsMuted();
        }
        texturePath = isMuted ? "../Assets/Sprites/Icons/Volume_Mute.png" : "../Assets/Sprites/Icons/Volume_Unmute.png";
    }
    else if (buttonName == "MusicMuteButton")
    {
        bool isMuted = false;
        AudioManager *audioManager = CORE->GetAudioManager();
        if (audioManager)
        {
            isMuted = audioManager->IsCategoryMuted("music");
        }
        texturePath = isMuted ? "../Assets/Sprites/Icons/Volume_Mute.png" : "../Assets/Sprites/Icons/Volume_Unmute.png";
    }
    else if (buttonName == "SFXMuteButton")
    {
        bool isMuted = false;
        AudioManager *audioManager = CORE->GetAudioManager();
        if (audioManager)
        {
            isMuted = audioManager->IsCategoryMuted("sfx");
        }
        texturePath = isMuted ? "../Assets/Sprites/Icons/Volume_Mute.png" : "../Assets/Sprites/Icons/Volume_Unmute.png";
    }
    else if (buttonName == "YesButton")
    {
        if (isPressed)
        {
            texturePath = "../Assets/Sprites/Icons/Hovering_button_dark.png";
        }
        else if (isHovered)
        {
            texturePath = "../Assets/Sprites/Icons/Hovering_button_light.png";
        }
        else
        {
            texturePath = "../Assets/Sprites/Icons/blankbtn.png";
        }
    }
    else if (buttonName == "NoButton")
    {
        if (isPressed)
        {
            texturePath = "../Assets/Sprites/Icons/Hovering_button_dark.png";
        }
        else if (isHovered)
        {
            texturePath = "../Assets/Sprites/Icons/Hovering_button_light.png";
        }
        else
        {
            texturePath = "../Assets/Sprites/Icons/blankbtn.png";
        }
    }
    else if (buttonName == "BackButton")
    {
        if (isPressed)
        {
            texturePath = "../Assets/Sprites/Icons/Back_button_dark.png";
        }
        else if (isHovered)
        {
            texturePath = "../Assets/Sprites/Icons/Back_button_light.png";
        }
        else
        {
            texturePath = "../Assets/Sprites/Icons/Back_button.png";
        }
    }
    else if (buttonName == "ReplayPicker_Slot0" || buttonName == "ReplayPicker_Slot1" ||
             buttonName == "ReplayPicker_Slot2" || buttonName == "ReplayPicker_Back")
    {
        // Colored quads — no texture swap needed.
        return;
    }

    unsigned int newtex = GraphicsSystem::LoadTexture(texturePath.c_str());
    rc->SetTextureID(newtex);
}

void MenuLogicSystem::Reset()
{
    currScreen = MenuScreens::Home;
}

void MenuLogicSystem::ScanReplays()
{
    m_replayNames.clear();
    m_replayLevels.clear();
    namespace fs = std::filesystem;

    const std::string replayDir = "../Assets/Replay/";
    if (!fs::exists(replayDir))
    {
        std::cout << "[MenuLogic] Replay directory not found: " << replayDir << std::endl;
        return;
    }

    std::vector<std::pair<fs::file_time_type, std::string>> files;
    for (const auto &entry : fs::directory_iterator(replayDir))
    {
        if (entry.is_regular_file() && entry.path().extension() == ".json")
            files.push_back({entry.last_write_time(), entry.path().stem().string()});
    }

    // Sort newest-first
    std::sort(files.begin(), files.end(),
              [](const auto &a, const auto &b) { return a.first > b.first; });

    for (size_t i = 0; i < std::min(files.size(), size_t(3)); ++i)
    {
        const std::string &stem = files[i].second;
        m_replayNames.push_back(stem);

        // Read the "level" field from the JSON header
        std::string levelName;
        std::ifstream f(replayDir + stem + ".json");
        if (f)
        {
            try
            {
                nlohmann::json root;
                f >> root;
                levelName = root.value("level", std::string{});
            }
            catch (...) {}
        }
        m_replayLevels.push_back(levelName);
    }

    std::cout << "[MenuLogic] Found " << m_replayNames.size() << " replay(s)." << std::endl;
}

void MenuLogicSystem::ShowReplayPicker()
{
    if (m_hasCreatedReplayPicker) return;

    ScanReplays();

    auto &factory           = GameObjectFactory::Instance();
    const vec2 camPos       = CameraManager::Instance().GetCameraPosition();
    const std::string font  = "../Assets/FrederickatheGreat-Regular.ttf";
    const int titleSize     = 40;
    const int slotSize      = 32;
    const int baseLayer     = 150;

    // Dark overlay
    GameObject *overlay = factory.CreateGameObject("ReplayPicker_Overlay");
    auto *ovT           = overlay->AddComponent<TransformComponent>();
    ovT->posX = camPos.x; ovT->posY = camPos.y;
    ovT->scaleX = 1920.0f; ovT->scaleY = 1080.0f;
    auto *ovR = overlay->AddComponent<RenderComponent>(
        RenderComponent::ShapeType::Square, vec4(0.0f, 0.0f, 0.0f, 0.78f));
    ovR->SetLayer(baseLayer);

    // Title panel
    GameObject *titleObj = factory.CreateGameObject("ReplayPicker_Title");
    auto *titleT         = titleObj->AddComponent<TransformComponent>();
    titleT->posX = camPos.x; titleT->posY = camPos.y + 300.0f;
    titleT->scaleX = 700.0f; titleT->scaleY = 100.0f;
    auto *titleR = titleObj->AddComponent<RenderComponent>(
        RenderComponent::ShapeType::Square, vec4(0.1f, 0.1f, 0.28f, 0.95f));
    titleR->SetLayer(baseLayer + 1);
    titleObj->AddComponent<TextComponent>(font.c_str(), titleSize, "CHOOSE A REPLAY",
                                          vec4(1.0f, 1.0f, 1.0f, 1.0f), false);

    // Slot buttons (3 slots, stacked vertically)
    const float slotStartY  = camPos.y + 130.0f;
    const float slotSpacing = 150.0f;
    for (int i = 0; i < 3; ++i)
    {
        std::string slotName    = "ReplayPicker_Slot" + std::to_string(i);
        bool        hasReplay   = (i < static_cast<int>(m_replayNames.size()));
        std::string displayName;
        if (hasReplay)
        {
            displayName = m_replayNames[i];
            if (i < static_cast<int>(m_replayLevels.size()) && !m_replayLevels[i].empty())
                displayName += "  [" + m_replayLevels[i] + "]";
        }
        else
        {
            displayName = "(empty)";
        }

        GameObject *slot = factory.CreateGameObject(slotName);
        auto *slotT      = slot->AddComponent<TransformComponent>();
        slotT->posX   = camPos.x;
        slotT->posY   = slotStartY - i * slotSpacing;
        slotT->scaleX = 620.0f;
        slotT->scaleY = 110.0f;

        vec4 color = hasReplay
            ? vec4(0.15f, 0.15f, 0.38f, 0.92f)
            : vec4(0.10f, 0.10f, 0.15f, 0.55f);
        auto *slotR = slot->AddComponent<RenderComponent>(
            RenderComponent::ShapeType::Square, color);
        slotR->SetLayer(baseLayer + 1);

        vec4 textColor = hasReplay
            ? vec4(1.0f, 1.0f, 1.0f, 1.0f)
            : vec4(0.5f, 0.5f, 0.5f, 1.0f);
        slot->AddComponent<TextComponent>(font.c_str(), slotSize, displayName, textColor, false);
    }

    // Back button
    GameObject *backBtn = factory.CreateGameObject("ReplayPicker_Back");
    auto *backT         = backBtn->AddComponent<TransformComponent>();
    backT->posX = camPos.x; backT->posY = camPos.y - 300.0f;
    backT->scaleX = 280.0f; backT->scaleY = 90.0f;
    auto *backR = backBtn->AddComponent<RenderComponent>(
        RenderComponent::ShapeType::Square, vec4(0.28f, 0.10f, 0.10f, 0.92f));
    backR->SetLayer(baseLayer + 1);
    backBtn->AddComponent<TextComponent>(font.c_str(), slotSize, "BACK",
                                         vec4(1.0f, 1.0f, 1.0f, 1.0f), false);

    m_hasCreatedReplayPicker = true;
    currScreen               = MenuScreens::ReplayPicker;
    std::cout << "[MenuLogic] Replay picker shown." << std::endl;
}

void MenuLogicSystem::HideReplayPicker()
{
    if (!m_hasCreatedReplayPicker) return;

    auto &factory = GameObjectFactory::Instance();
    for (const std::string &name : {"ReplayPicker_Overlay", "ReplayPicker_Title",
                                    "ReplayPicker_Slot0",   "ReplayPicker_Slot1",
                                    "ReplayPicker_Slot2",   "ReplayPicker_Back"})
    {
        GameObject *obj = factory.FindGameObject(name);
        if (obj) factory.DestroyGameObject(obj->GetID());
    }

    m_hasCreatedReplayPicker = false;
    m_replayNames.clear();
    m_replayLevels.clear();
    std::cout << "[MenuLogic] Replay picker hidden." << std::endl;
}

void MenuLogicSystem::UpdateReplayPickerPositions()
{
    if (!m_hasCreatedReplayPicker) return;

    const vec2 camPos = CameraManager::Instance().GetCameraPosition();
    auto &factory     = GameObjectFactory::Instance();

    struct Elem { const char *name; float dy; };
    const Elem elems[] = {
        {"ReplayPicker_Overlay",  0.0f   },
        {"ReplayPicker_Title",    300.0f },
        {"ReplayPicker_Slot0",    130.0f },
        {"ReplayPicker_Slot1",    -20.0f },
        {"ReplayPicker_Slot2",   -170.0f },
        {"ReplayPicker_Back",    -300.0f },
    };

    for (const auto &e : elems)
    {
        GameObject *obj = factory.FindGameObject(e.name);
        if (!obj) continue;
        auto *t = obj->GetComponent<TransformComponent>();
        if (t) { t->posX = camPos.x; t->posY = camPos.y + e.dy; }
    }
}
