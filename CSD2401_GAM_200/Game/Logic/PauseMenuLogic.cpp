/* Start Header ************************************************************************/
/*!
\file       PauseMenuLogic.cpp
\author     Leck Xin Yee [100% - 1240 lines]
\par        xinyee.leck@digipen.edu
\date       November, 28, 2025

Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*/
/* End Header **************************************************************************/

#include "PauseMenuLogic.h"
#include "core.h"
#include "GhostSystem.h"
#include "LevelEditor.h"
#include "CameraManager.h"
#include "GameCallbacks.h"
#include "TextComponent.h"
#include <iostream>
#include <filesystem>
#include <algorithm>
#include <fstream>
#include <cmath>
#include <nlohmann/json.hpp>
#include "ColliderComponent.h"
#include "Collision.h"

/**
 * @brief Initializes all pause menu state variables.
 */
void PauseMenuLogicSystem::Initialize()
{
    // Clean up any existing UI before resetting
    DestroyHowToPlayUI();
    DestroyGhostMenuUI();
    DestroyPauseSettingsUI();
    DestroyPauseMenuUI();

    // Remove any lingering low pass filter from previous scene/restart
    AudioManager* audioManager = CORE->GetAudioManager();
    if (audioManager)
    {
        audioManager->SetMasterLowPassFilter(false);
        std::cout << "[PauseMenu] Initialize - low pass filter cleaned up" << std::endl;
    }

    isPauseMenuActive = false;
    hasCreatedUI = false;
    hoveredButtonName = "";
    pendingSceneChange = false;
    isPauseSettingsOpen = false;
    hasCreatedSettingsUI = false;
    wasSettingsOpenLastFrame = false;
    pendingSettingsClose = false;
    settingsCloseTimer = 0.0f;
    isHowToPlayOpen = false;
    hasCreatedHowToPlayUI = false;
    wasHowToPlayOpenLastFrame = false;
    pendingHowToPlayClose = false;
    howToPlayCloseTimer = 0.0f;
    howToPlayOpenedViaTab = false;

    isGhostMenuOpen = false;
    wasGhostMenuOpenLastFrame = false;
    hasCreatedGhostMenuUI = false;
    pendingGhostMenuClose = false;
    ghostMenuCloseTimer = 0.0f;
    m_ghostReplayNames.clear();
    m_ghostReplayLevels.clear();
}

/**
 * @brief Main update loop handling pause menu activation, UI movement, hover/click logic, and delayed scene actions.
 */
void PauseMenuLogicSystem::Update(float dt)
{
    // Update audio fade
    AudioManager* audioManager = CORE->GetAudioManager();
    if (audioManager)
    {
        audioManager->UpdateFade(dt);
    }

    // check if we're in play mode
    LevelEditor* editor = CORE ? CORE->GetLevelEditor() : nullptr;
    if (!editor || editor->GetEditorMode() != EditorMode::Play)
    {
        // not in play mode, hide pause menu if active
        if (isPauseMenuActive)
        {
            TogglePauseMenu();
        }
        return;
    }

    // Declare factory once at the top
    auto& factory = GameObjectFactory::Instance();

    // Handle InGamePauseButton hover/click functionality (uses screen-space coordinates)
    GameObject* pauseBtn = factory.FindGameObject("InGamePauseButton");

    // Check if win/lose screens are active
    bool isWinLoseScreenActive = false;
    if (CORE && CORE->GetGameCallbacks())
    {
        IGameCallbacksSystem* cb = CORE->GetGameCallbacks();
        isWinLoseScreenActive = cb->IsWinScreenActive() || cb->IsLoseScreenActive();
    }

    if (pauseBtn && !isPauseMenuActive && !isWinLoseScreenActive)
    {
        // Use screen-space mouse coordinates since button has screenSpace = true
        InputManager* inputManager = CORE->GetInputManager();
        if (!inputManager)
        {
            return;
        }

        double mouseX = inputManager->GetMouseX();
        double mouseY = inputManager->GetMouseY();
        bool isClicked = InputManager::IsMouseLeftClicked();

        auto* transform = pauseBtn->GetComponent<TransformComponent>();
        auto* rc = pauseBtn->GetComponent<RenderComponent>();

        if (transform && rc)
        {
            // get actual window dimensions
            GraphicsSystem* graphics = CORE->GetGraphicsSystem();
            if (!graphics)
            {
                return;
            }

            float screenHeight = static_cast<float>(graphics->GetWindowHeight());

            float btnX = transform->posX -10.0f;
            float btnY = screenHeight - transform->posY;
            float halfWidth = transform->scaleX / 2.0f;
            float halfHeight = transform->scaleY / 2.0f;

            bool isHoveringPauseBtn = (mouseX >= btnX - halfWidth && mouseX <= btnX + halfWidth &&
                                       mouseY >= btnY - halfHeight && mouseY <= btnY + halfHeight);

            static bool wasHoveringLastFrame = false;

            if (isClicked && isHoveringPauseBtn)
            {
                // Pressed state (dark)
                rc->SetTextureID(GraphicsSystem::LoadTexture("../Assets/Sprites/Icons/Paused_Button_dark.png"));

                // Play click sound
                //AudioManager* audioManager = CORE->GetAudioManager();
                if (audioManager)
                {
                    audioManager->TriggerEvent("UI_Click");
                }

                // Toggle pause menu
                TogglePauseMenu();
                //std::cout << "[InGamePauseButton] CLICKED! Opening pause menu" << std::endl;
                }
                else if (isHoveringPauseBtn)
                {
                // Hover state (light)
                rc->SetTextureID(GraphicsSystem::LoadTexture("../Assets/Sprites/Icons/Paused_Button_light.png"));

                // Play hover sound only on hover state change
                if (!wasHoveringLastFrame)
                {
                    //AudioManager* audioManager = CORE->GetAudioManager();
                    if (audioManager)
                    {
                        audioManager->TriggerEvent("UI_Hover");
                    }
                    //std::cout << "[InGamePauseButton] HOVER START" << std::endl;
                }
                wasHoveringLastFrame = true;
                }
            else
            {
                // Normal state
                rc->SetTextureID(GraphicsSystem::LoadTexture("../Assets/Sprites/Icons/paused_button.png"));
                wasHoveringLastFrame = false;
            }
        }
    }

    // Hide InGamePauseButton when pause menu is active OR when win/lose screens are showing
    bool shouldHidePauseButton = isPauseMenuActive;

    // Check if win/lose screens are active
    if (CORE && CORE->GetGameCallbacks())
    {
        IGameCallbacksSystem* cb = CORE->GetGameCallbacks();
        if (cb->IsWinScreenActive() || cb->IsLoseScreenActive())
        {
            shouldHidePauseButton = true;
        }
    }

    if (pauseBtn)
    {
        pauseBtn->SetActive(!shouldHidePauseButton);
    }

    // Hide shell/coin UI when pause menu is active or win/lose screens are showing
    GameObject* shellIcon1 = factory.FindGameObject("ShellIcon1");
    GameObject* shellIcon2 = factory.FindGameObject("ShellIcon2");
    GameObject* shellIcon3 = factory.FindGameObject("ShellIcon3");
    GameObject* coinIcon = factory.FindGameObject("CoinIcon");
    GameObject* coinText = factory.FindGameObject("CoinCountText");

    if (shellIcon1) shellIcon1->SetActive(!shouldHidePauseButton);
    if (shellIcon2) shellIcon2->SetActive(!shouldHidePauseButton);
    if (shellIcon3) shellIcon3->SetActive(!shouldHidePauseButton);
    if (coinIcon) coinIcon->SetActive(!shouldHidePauseButton);
    if (coinText) coinText->SetActive(!shouldHidePauseButton);

    // update pause menu position to follow camera (only when menu is active)
    if (isPauseMenuActive)
    {
        vec2 camPos = CameraManager::Instance().GetCameraPosition();

        std::vector<std::string> uiElements = {
            "PauseOverlay",
            "PauseBoard",
            "PauseResumeButton",
            "PauseRestartButton",
            "PauseSettingsButton",
            "PauseMainMenuButton",
            "PauseHowToPlayButton",
            "PauseGhostButton",
            "PauseScreen_Settings",
            "PauseMasterText",
            "PauseMusicText",
            "PauseSFXText",
            "PauseSlider_Bar_Master",
            "PauseSlider_Bar_Back_Master",
            "PauseSlider_Knob_Master",
            "PauseSlider_Bar_Music",
            "PauseSlider_Bar_Back_Music",
            "PauseSlider_Knob_Music",
            "PauseSlider_Bar_SFX",
            "PauseSlider_Bar_Back_SFX",
            "PauseSlider_Knob_SFX",
            "PauseMasterMuteButton",
            "PauseMusicMuteButton",
            "PauseSFXMuteButton",
            "PauseBackButton",
            "PauseScreen_HowToPlay",
            "HowToPlayBackButton",
            "PauseGhostBoard",
            "PauseGhostTitle",
            "PauseGhost_Toggle",
            "PauseGhost_Slot0",
            "PauseGhost_Slot1",
            "PauseGhost_Slot2",
            "PauseGhost_Back"
        };

        for (const auto& name : uiElements)
        {
            GameObject* obj = factory.FindGameObject(name);
            if (obj)
            {
                auto* transform = obj->GetComponent<TransformComponent>();
                if (transform)
                {
                    static std::map<std::string, vec2> originalPositions;
                    if (originalPositions.find(name) == originalPositions.end())
                    {
                        originalPositions[name] = vec2(transform->posX, transform->posY);
                    }

                    transform->posX = camPos.x + originalPositions[name].x;
                    transform->posY = camPos.y + originalPositions[name].y;
                }
            }
        }
    }

    // handle pending scene change
    if (pendingSceneChange)
    {
        pendingTimer -= dt;
        if (pendingTimer <= 0.0f)
        {
            // Check if fade is still active for buttons that fade - if so, wait a bit longer
            if ((hoveredButtonName == "PauseMainMenuButton" || hoveredButtonName == "PauseRestartButton")
                && audioManager && audioManager->IsAnyFadeActive())
            {
                //std::cout << "[PauseMenu] Fade still active, waiting..." << std::endl;
                pendingTimer = 0.1f; // Wait another 100ms and check again
                return;
            }

            if (hoveredButtonName == "PauseResumeButton")
            {
                std::cout << "[PauseMenu] Resume clicked" << std::endl;
                // Reset button texture before closing
                SetButtonTexture("PauseResumeButton", ButtonState::Normal);
                TogglePauseMenu(); // close menu and unpause
            }
            else if (hoveredButtonName == "PauseRestartButton")
            {
                std::cout << "[PauseMenu] Restart - fade complete, reloading level" << std::endl;

                // DON'T remove low pass filter here - keep audio muffled during fade
                // It will be cleaned up after scene loads

                // Discard any in-progress recording before restarting
                if (GhostSystem* ghost = CORE ? CORE->GetGhostSystem() : nullptr)
                {
                    ghost->StopRecording();
                    ghost->ResetRecording();
                    ghost->StopPlayback(); // stop current playback; WinLoseLogic::Initialize will re-park if toggle is ON
                }

                // Destroy UI before scene change
                DestroyHowToPlayUI();
                DestroyGhostMenuUI();
                DestroyPauseSettingsUI();
                DestroyPauseMenuUI();

                // close menu first
                if (isPauseMenuActive)
                {
                    isPauseMenuActive = false;
                    CORE->SetGamePaused(false, false); // don't unpause audio when pause, use low pass
                }

                // restart the current level
                if (!currentScenePath.empty())
                {
                    CORE->RequestSceneLoad(currentScenePath, true);
                }
            }
            else if (hoveredButtonName == "PauseSettingsButton")
            {
                //std::cout << "[PauseMenu] Settings clicked - opening settings" << std::endl;

                // Create settings UI if not created yet
                if (!hasCreatedSettingsUI)
                {
                    CreatePauseSettingsUI();
                }

                isPauseSettingsOpen = true;

                // Clear the pending state immediately for settings
                pendingSceneChange = false;
                hoveredButtonName.clear();
            }
            else if (hoveredButtonName == "PauseMainMenuButton")
            {
                //std::cout << "[PauseMenu] Main Menu - fade complete, loading scene" << std::endl;

                // Destroy UI before scene change
                DestroyHowToPlayUI();
                DestroyGhostMenuUI();
                DestroyPauseSettingsUI();
                DestroyPauseMenuUI();

                // close menu first
                if (isPauseMenuActive)
                {
                    isPauseMenuActive = false;
                    CORE->SetGamePaused(false, false);
                }
                // back to main menu
                CORE->RequestSceneLoad("../Assets/Scenes/MainMenu.json", true);
            }
            else if (hoveredButtonName == "PauseHowToPlayButton")
            {
                std::cout << "[PauseMenu] How To Play clicked - opening how to play" << std::endl;

                // Create how to play UI if not created yet
                if (!hasCreatedHowToPlayUI)
                {
                    CreateHowToPlayUI();
                }

                isHowToPlayOpen = true;
                howToPlayOpenedViaTab = false; // Opened via button, not TAB

                // Clear the pending state immediately
                pendingSceneChange = false;
                hoveredButtonName.clear();
            }
            else if (hoveredButtonName == "PauseGhostButton")
            {
                std::cout << "[PauseMenu] Ghost button clicked - opening ghost menu" << std::endl;

                // Create ghost menu UI if not created yet
                if (!hasCreatedGhostMenuUI)
                {
                    CreateGhostMenuUI();
                }

                isGhostMenuOpen = true;

                // Clear the pending state immediately
                pendingSceneChange = false;
                hoveredButtonName.clear();
            }

            pendingSceneChange = false;
            hoveredButtonName.clear();
        }
        return;
    }

    // only process menu interactions if pause menu is active
    if (!isPauseMenuActive)
    {
        return;
    }

    // Handle how to play screen visibility
    if (isHowToPlayOpen != wasHowToPlayOpenLastFrame)
    {
        if (isHowToPlayOpen)
        {
            // Clear any hover state
            hoveredButtonName.clear();

            // how to play opens has low pass
            if (audioManager)
            {
                audioManager->SetMasterLowPassFilter(true, 800.0f, 1.0f);
                //std::cout << "[PauseMenu] Low pass filter enabled for How To Play" << std::endl;
            }

            // Show how to play UI
            SetHowToPlayVisible(true);

            // Reset button textures to normal before hiding them
            SetButtonTexture("PauseResumeButton", ButtonState::Normal);
            SetButtonTexture("PauseRestartButton", ButtonState::Normal);
            SetButtonTexture("PauseSettingsButton", ButtonState::Normal);
            SetButtonTexture("PauseMainMenuButton", ButtonState::Normal);
            SetButtonTexture("PauseHowToPlayButton", ButtonState::Normal);

            // Hide pause menu buttons and board
            if (auto* resumeBtn = factory.FindGameObject("PauseResumeButton"))
                resumeBtn->SetActive(false);
            if (auto* restartBtn = factory.FindGameObject("PauseRestartButton"))
                restartBtn->SetActive(false);
            if (auto* settingsBtn = factory.FindGameObject("PauseSettingsButton"))
                settingsBtn->SetActive(false);
            if (auto* mainMenuBtn = factory.FindGameObject("PauseMainMenuButton"))
                mainMenuBtn->SetActive(false);
            if (auto* howToPlayBtn = factory.FindGameObject("PauseHowToPlayButton"))
                howToPlayBtn->SetActive(false);
            if (auto* board = factory.FindGameObject("PauseBoard"))
                board->SetActive(false);
        }
        else
        {
            // no low pass filter when how to play closes
            if (audioManager)
            {
                audioManager->SetMasterLowPassFilter(false);
                //std::cout << "[PauseMenu] Low pass filter disabled for How To Play" << std::endl;
            }

            // Hide how to play UI
            SetHowToPlayVisible(false);

            // Reset how to play button texture to normal
            SetButtonTexture("PauseHowToPlayButton", ButtonState::Normal);

            // Show pause menu buttons and board
            if (auto* resumeBtn = factory.FindGameObject("PauseResumeButton"))
                resumeBtn->SetActive(true);
            if (auto* restartBtn = factory.FindGameObject("PauseRestartButton"))
                restartBtn->SetActive(true);
            if (auto* settingsBtn = factory.FindGameObject("PauseSettingsButton"))
                settingsBtn->SetActive(true);
            if (auto* mainMenuBtn = factory.FindGameObject("PauseMainMenuButton"))
                mainMenuBtn->SetActive(true);
            if (auto* howToPlayBtn = factory.FindGameObject("PauseHowToPlayButton"))
                howToPlayBtn->SetActive(true);
            if (auto* board = factory.FindGameObject("PauseBoard"))
                board->SetActive(true);
        }
        wasHowToPlayOpenLastFrame = isHowToPlayOpen;
    }

    // Handle Ghost menu screen visibility - ONLY update when state changes
    if (isGhostMenuOpen != wasGhostMenuOpenLastFrame)
    {
        if (isGhostMenuOpen)
        {
            hoveredButtonName.clear();

            // Show ghost menu UI
            SetGhostMenuVisible(true);

            // Reset and hide pause menu buttons and board
            SetButtonTexture("PauseResumeButton", ButtonState::Normal);
            SetButtonTexture("PauseRestartButton", ButtonState::Normal);
            SetButtonTexture("PauseSettingsButton", ButtonState::Normal);
            SetButtonTexture("PauseMainMenuButton", ButtonState::Normal);
            SetButtonTexture("PauseHowToPlayButton", ButtonState::Normal);
            SetButtonTexture("PauseGhostButton", ButtonState::Normal);

            if (auto* resumeBtn = factory.FindGameObject("PauseResumeButton"))   resumeBtn->SetActive(false);
            if (auto* restartBtn = factory.FindGameObject("PauseRestartButton")) restartBtn->SetActive(false);
            if (auto* settingsBtn = factory.FindGameObject("PauseSettingsButton")) settingsBtn->SetActive(false);
            if (auto* mainMenuBtn = factory.FindGameObject("PauseMainMenuButton")) mainMenuBtn->SetActive(false);
            if (auto* howToPlayBtn = factory.FindGameObject("PauseHowToPlayButton")) howToPlayBtn->SetActive(false);
            if (auto* ghostBtn = factory.FindGameObject("PauseGhostButton")) ghostBtn->SetActive(false);
            if (auto* board = factory.FindGameObject("PauseBoard")) board->SetActive(false);
        }
        else
        {
            // Hide ghost menu UI
            SetGhostMenuVisible(false);

            // Show pause menu buttons and board
            if (auto* resumeBtn = factory.FindGameObject("PauseResumeButton"))   resumeBtn->SetActive(true);
            if (auto* restartBtn = factory.FindGameObject("PauseRestartButton")) restartBtn->SetActive(true);
            if (auto* settingsBtn = factory.FindGameObject("PauseSettingsButton")) settingsBtn->SetActive(true);
            if (auto* mainMenuBtn = factory.FindGameObject("PauseMainMenuButton")) mainMenuBtn->SetActive(true);
            if (auto* howToPlayBtn = factory.FindGameObject("PauseHowToPlayButton")) howToPlayBtn->SetActive(true);
            if (auto* ghostBtn = factory.FindGameObject("PauseGhostButton")) ghostBtn->SetActive(true);
            if (auto* board = factory.FindGameObject("PauseBoard")) board->SetActive(true);
        }
        wasGhostMenuOpenLastFrame = isGhostMenuOpen;
    }

    // Handle pause settings screen visibility - ONLY update when state changes
    if (isPauseSettingsOpen != wasSettingsOpenLastFrame)
    {
        if (isPauseSettingsOpen)
        {
            // Clear any hover state from previous menu
            hoveredButtonName.clear();

            // Show settings UI
            SetPauseSettingsVisible(true);

            // Reset button textures to normal before hiding them
            SetButtonTexture("PauseResumeButton", ButtonState::Normal);
            SetButtonTexture("PauseRestartButton", ButtonState::Normal);
            SetButtonTexture("PauseSettingsButton", ButtonState::Normal);
            SetButtonTexture("PauseMainMenuButton", ButtonState::Normal);
            SetButtonTexture("PauseHowToPlayButton", ButtonState::Normal);

            // Hide pause menu buttons and board
            if (auto* resumeBtn = factory.FindGameObject("PauseResumeButton"))
                resumeBtn->SetActive(false);
            if (auto* restartBtn = factory.FindGameObject("PauseRestartButton"))
                restartBtn->SetActive(false);
            if (auto* settingsBtn = factory.FindGameObject("PauseSettingsButton"))
                settingsBtn->SetActive(false);
            if (auto* mainMenuBtn = factory.FindGameObject("PauseMainMenuButton"))
                mainMenuBtn->SetActive(false);
            if (auto* howToPlayBtn = factory.FindGameObject("PauseHowToPlayButton"))
                howToPlayBtn->SetActive(false);
            if (auto* board = factory.FindGameObject("PauseBoard"))
                board->SetActive(false);
        }
        else
        {
            // Hide settings UI
            SetPauseSettingsVisible(false);

            // Show pause menu buttons and board
            if (auto* resumeBtn = factory.FindGameObject("PauseResumeButton"))
                resumeBtn->SetActive(true);
            if (auto* restartBtn = factory.FindGameObject("PauseRestartButton"))
                restartBtn->SetActive(true);
            if (auto* settingsBtn = factory.FindGameObject("PauseSettingsButton"))
                settingsBtn->SetActive(true);
            if (auto* mainMenuBtn = factory.FindGameObject("PauseMainMenuButton"))
                mainMenuBtn->SetActive(true);
            if (auto* howToPlayBtn = factory.FindGameObject("PauseHowToPlayButton"))
                howToPlayBtn->SetActive(true);
            if (auto* board = factory.FindGameObject("PauseBoard"))
                board->SetActive(true);
        }
        wasSettingsOpenLastFrame = isPauseSettingsOpen;
    }

    // Handle pause settings screen interactions
    if (isPauseSettingsOpen)
    {
        // Handle pending settings close (back button was clicked or ESC pressed)
        if (pendingSettingsClose)
        {
            settingsCloseTimer -= dt;
            if (settingsCloseTimer <= 0.0f)
            {
                std::cout << "[PauseMenu] Closing settings after delay" << std::endl;
                isPauseSettingsOpen = false;
                pendingSettingsClose = false;
                hoveredButtonName.clear();
            }
            return; // Don't process interactions during pending close
        }

        // Check if back button is clicked to close settings
        double mouseX = InputManager::GetMouseWorldX();
        double mouseY = InputManager::GetMouseWorldY();
        bool isClicked = InputManager::IsMouseLeftClicked();

        std::string currentHover = "";

        // Check for mute button hovers
        if (IsMouseOver(factory.FindGameObject("PauseMasterMuteButton"), mouseX, mouseY))
        {
            currentHover = "PauseMasterMuteButton";
        }
        else if (IsMouseOver(factory.FindGameObject("PauseMusicMuteButton"), mouseX, mouseY))
        {
            currentHover = "PauseMusicMuteButton";
        }
        else if (IsMouseOver(factory.FindGameObject("PauseSFXMuteButton"), mouseX, mouseY))
        {
            currentHover = "PauseSFXMuteButton";
        }
        else if (IsMouseOver(factory.FindGameObject("PauseBackButton"), mouseX, mouseY))
        {
            currentHover = "PauseBackButton";
        }

        // Handle hover state changes
        if (currentHover != hoveredButtonName)
        {
            if (!hoveredButtonName.empty())
            {
                SetButtonTexture(hoveredButtonName, ButtonState::Normal);
            }

            if (!currentHover.empty())
            {
                SetButtonTexture(currentHover, ButtonState::Hover);
                AudioManager* localAudioManager = CORE->GetAudioManager();
                if (localAudioManager)
                {
                    localAudioManager->TriggerEvent("UI_Hover");
                }
            }

            hoveredButtonName = currentHover;
        }

        // Handle clicks
        if (isClicked && !hoveredButtonName.empty())
        {
            // Set button to pressed state
            SetButtonTexture(hoveredButtonName, ButtonState::Pressed);

            audioManager = CORE->GetAudioManager();
            if (audioManager)
            {
                audioManager->TriggerEvent("UI_Click");
            }

            if (hoveredButtonName == "PauseBackButton")
            {
                std::cout << "[PauseMenu] Back button clicked - starting close delay" << std::endl;
                pendingSettingsClose = true;
                settingsCloseTimer = 0.15f; // 150ms delay to show pressed state
            }
            else if (hoveredButtonName == "PauseMasterMuteButton")
            {
                if (audioManager)
                {
                    audioManager->ToggleMasterMute();
                }
                SetButtonTexture("PauseMasterMuteButton", ButtonState::Hover);
            }
            else if (hoveredButtonName == "PauseMusicMuteButton")
            {
                if (audioManager)
                {
                    audioManager->ToggleCategoryMute("music");
                }
                SetButtonTexture("PauseMusicMuteButton", ButtonState::Hover);
            }
            else if (hoveredButtonName == "PauseSFXMuteButton")
            {
                if (audioManager)
                {
                    audioManager->ToggleCategoryMute("sfx");
                }
                SetButtonTexture("PauseSFXMuteButton", ButtonState::Hover);
            }
        }

        return; // Don't process normal pause menu interactions
    }

    // Handle how to play screen interactions
    if (isHowToPlayOpen)
    {
        // Handle pending how to play close
        if (pendingHowToPlayClose)
        {
            howToPlayCloseTimer -= dt;
            if (howToPlayCloseTimer <= 0.0f)
            {
                std::cout << "[PauseMenu] Closing how to play after delay" << std::endl;

                // Reset back button texture to normal before closing
                SetButtonTexture("HowToPlayBackButton", ButtonState::Normal);

                isHowToPlayOpen = false;
                pendingHowToPlayClose = false;
                hoveredButtonName.clear();
            }
            return;
        }

        // Check for ESC key or back button click to close
        double mouseX = InputManager::GetMouseWorldX();
        double mouseY = InputManager::GetMouseWorldY();
        bool isClicked = InputManager::IsMouseLeftClicked();

        // Check if user pressed ESC to close
        static bool escWasPressedHowToPlay = false;
        if (glfwGetKey(glfwGetCurrentContext(), GLFW_KEY_ESCAPE) == GLFW_PRESS)
        {
            if (!escWasPressedHowToPlay)
            {
                //std::cout << "[PauseMenu] ESC pressed - closing how to play" << std::endl;

                // if opened via TAB, close immediately without opening pause menu
                if (howToPlayOpenedViaTab)
                {
                    //std::cout << "[PauseMenu] How To Play opened via TAB - closing completely" << std::endl;
                    CloseHowToPlay();
                }
                else
                {
                    // otherwise just close how to play and return to pause menu
                    pendingHowToPlayClose = true;
                    howToPlayCloseTimer = 0.15f;
                }
                escWasPressedHowToPlay = true;
            }
        }
        else
        {
            escWasPressedHowToPlay = false;
        }

        // Check for back button hover and click
        std::string currentHover = "";
        GameObject* backBtn = factory.FindGameObject("HowToPlayBackButton");
        if (backBtn && IsMouseOver(backBtn, mouseX, mouseY))
        {
            currentHover = "HowToPlayBackButton";
        }

        // Handle hover state changes
        if (currentHover != hoveredButtonName)
        {
            if (!hoveredButtonName.empty())
            {
                SetButtonTexture(hoveredButtonName, ButtonState::Normal);
            }

            if (!currentHover.empty())
            {
                SetButtonTexture(currentHover, ButtonState::Hover);
                AudioManager* localAudioManager = CORE->GetAudioManager();
                if (localAudioManager)
                {
                    localAudioManager->TriggerEvent("UI_Hover");
                }
            }

            hoveredButtonName = currentHover;
        }

        // Handle click
        if (isClicked && hoveredButtonName == "HowToPlayBackButton")
        {
            SetButtonTexture("HowToPlayBackButton", ButtonState::Pressed);
            audioManager = CORE->GetAudioManager();
            if (audioManager)
            {
                audioManager->TriggerEvent("UI_Click");
            }
            std::cout << "[PauseMenu] How To Play back button clicked" << std::endl;

            if (howToPlayOpenedViaTab)
            {
                CloseHowToPlay();
            }
            else
            {
                pendingHowToPlayClose = true;
                howToPlayCloseTimer = 0.15f;
            }
        }

        return; // Don't process normal pause menu interactions
    }

    // Handle Ghost menu screen interactions
    if (isGhostMenuOpen)
    {
        // Handle pending ghost menu close
        if (pendingGhostMenuClose)
        {
            ghostMenuCloseTimer -= dt;
            if (ghostMenuCloseTimer <= 0.0f)
            {
                isGhostMenuOpen = false;
                pendingGhostMenuClose = false;
                hoveredButtonName.clear();
            }
            return;
        }

        double mouseX = InputManager::GetMouseWorldX();
        double mouseY = InputManager::GetMouseWorldY();
        bool isClicked = InputManager::IsMouseLeftClicked();

        std::string currentHover = "";

        if (IsMouseOver(factory.FindGameObject("PauseGhost_Toggle"), mouseX, mouseY))
            currentHover = "PauseGhost_Toggle";
        else if (IsMouseOver(factory.FindGameObject("PauseGhost_Slot0"), mouseX, mouseY))
            currentHover = "PauseGhost_Slot0";
        else if (IsMouseOver(factory.FindGameObject("PauseGhost_Slot1"), mouseX, mouseY))
            currentHover = "PauseGhost_Slot1";
        else if (IsMouseOver(factory.FindGameObject("PauseGhost_Slot2"), mouseX, mouseY))
            currentHover = "PauseGhost_Slot2";
        else if (IsMouseOver(factory.FindGameObject("PauseGhost_Back"), mouseX, mouseY))
            currentHover = "PauseGhost_Back";

        // Hover state changes
        if (currentHover != hoveredButtonName)
        {
            if (!hoveredButtonName.empty())
                SetButtonTexture(hoveredButtonName, ButtonState::Normal);
            if (!currentHover.empty())
            {
                SetButtonTexture(currentHover, ButtonState::Hover);
                AudioManager* localAudio = CORE->GetAudioManager();
                if (localAudio) localAudio->TriggerEvent("UI_Hover");
            }
            hoveredButtonName = currentHover;
        }

        // Clicks
        if (isClicked && !hoveredButtonName.empty())
        {
            SetButtonTexture(hoveredButtonName, ButtonState::Pressed);
            audioManager = CORE->GetAudioManager();
            if (audioManager) audioManager->TriggerEvent("UI_Click");

            if (hoveredButtonName == "PauseGhost_Back")
            {
                pendingGhostMenuClose = true;
                ghostMenuCloseTimer = 0.15f;
            }
            else if (hoveredButtonName == "PauseGhost_Toggle")
            {
                GhostSystem* ghost = CORE ? CORE->GetGhostSystem() : nullptr;
                if (ghost)
                {
                    // ToggleGhost() parks the ghost at frame 0 (ON) or destroys it (OFF).
                    // It deliberately does NOT start playback — that is triggered by
                    // WinLoseLogic::CheckSafeZoneExit() when the player leaves SafeZone.
                    ghost->ToggleGhostVisible();
                    bool nowVisible = ghost->IsGhostVisible();

                    if (nowVisible)
                    {
                        // Ghost is parked at frame 0.
                        // Only start playback immediately if the player is already past the SafeZone.
                        // If still inside SafeZone, WinLoseLogic will call StartPlayback() on exit.
                        bool playerInSafeZone = true;

                        GameObject* safeZone = factory.FindGameObject("SafeZone");
                        GameObject* player   = factory.FindGameObject("Hero");
                        if (safeZone && player && safeZone->IsActive() && player->IsActive())
                        {
                            auto* szCol = safeZone->GetComponent<ColliderComponent>();
                            auto* plCol = player->GetComponent<ColliderComponent>();
                            if (szCol && plCol)
                            {
                                AABB szAABB = szCol->GetAABB();
                                AABB plAABB = plCol->GetAABB();
                                float szCX = (szAABB.min.x + szAABB.max.x) / 2.0f;
                                float szCY = (szAABB.min.y + szAABB.max.y) / 2.0f;
                                float plCX = (plAABB.min.x + plAABB.max.x) / 2.0f;
                                float plCY = (plAABB.min.y + plAABB.max.y) / 2.0f;
                                float halfW = (szAABB.max.x - szAABB.min.x) / 2.0f + (plAABB.max.x - plAABB.min.x) / 2.0f;
                                float halfH = (szAABB.max.y - szAABB.min.y) / 2.0f + (plAABB.max.y - plAABB.min.y) / 2.0f;
                                playerInSafeZone = (std::abs(plCX - szCX) < halfW) && (std::abs(plCY - szCY) < halfH);
                            }
                        }

                        if (!playerInSafeZone)
                        {
                            // Player is mid-run past SafeZone — start ghost from the beginning now
                            ghost->StopPlayback();
                            ghost->StartPlayback();
                        }
                        // else: player is in SafeZone — ghost waits at frame 0,
                        //       WinLoseLogic::CheckSafeZoneExit() will fire StartPlayback() on exit
                    }
                    // OFF path: ToggleGhost() already called StopPlayback() and destroyed the ghost

                    // Refresh toggle button label
                    GameObject* toggleBtn = factory.FindGameObject("PauseGhost_Toggle");
                    if (toggleBtn)
                    {
                        auto* tc = toggleBtn->GetComponent<TextComponent>();
                        if (tc) tc->SetText(std::string("Ghost: ") + (nowVisible ? "ON" : "OFF"));
                    }
                }
                // Stay in hover state after toggle
                SetButtonTexture("PauseGhost_Toggle", ButtonState::Hover);
            }
            else if (hoveredButtonName == "PauseGhost_Slot0" || hoveredButtonName == "PauseGhost_Slot1" || hoveredButtonName == "PauseGhost_Slot2")
            {
                int idx = hoveredButtonName.back() - '0';
                if (idx >= 0 && idx < static_cast<int>(m_ghostReplayNames.size()))
                {
                    GhostSystem* ghost = CORE ? CORE->GetGhostSystem() : nullptr;
                    if (ghost)
                    {
                        ghost->LoadRecording(m_ghostReplayNames[idx]);

                        // Make sure ghost is toggled visible (ToggleGhost parks at frame 0)
                        if (!ghost->IsGhostVisible())
                            ghost->ToggleGhostVisible();

                        // Same SafeZone logic: only start playback if player is already past SafeZone
                        bool playerInSafeZone = true;
                        GameObject* safeZone = factory.FindGameObject("SafeZone");
                        GameObject* player   = factory.FindGameObject("Hero");
                        if (safeZone && player && safeZone->IsActive() && player->IsActive())
                        {
                            auto* szCol = safeZone->GetComponent<ColliderComponent>();
                            auto* plCol = player->GetComponent<ColliderComponent>();
                            if (szCol && plCol)
                            {
                                AABB szAABB = szCol->GetAABB();
                                AABB plAABB = plCol->GetAABB();
                                float szCX = (szAABB.min.x + szAABB.max.x) / 2.0f;
                                float szCY = (szAABB.min.y + szAABB.max.y) / 2.0f;
                                float plCX = (plAABB.min.x + plAABB.max.x) / 2.0f;
                                float plCY = (plAABB.min.y + plAABB.max.y) / 2.0f;
                                float halfW = (szAABB.max.x - szAABB.min.x) / 2.0f + (plAABB.max.x - plAABB.min.x) / 2.0f;
                                float halfH = (szAABB.max.y - szAABB.min.y) / 2.0f + (plAABB.max.y - plAABB.min.y) / 2.0f;
                                playerInSafeZone = (std::abs(plCX - szCX) < halfW) && (std::abs(plCY - szCY) < halfH);
                            }
                        }
                        if (!playerInSafeZone)
                        {
                            ghost->StopPlayback();
                            ghost->StartPlayback();
                        }

                        // Refresh toggle label
                        GameObject* toggleBtn = factory.FindGameObject("PauseGhost_Toggle");
                        if (toggleBtn)
                        {
                            auto* tc = toggleBtn->GetComponent<TextComponent>();
                            if (tc) tc->SetText("Ghost: ON");
                        }
                        std::cout << "[PauseMenu] Ghost replay loaded: " << m_ghostReplayNames[idx] << std::endl;
                    }
                    // Close ghost menu after selecting
                    pendingGhostMenuClose = true;
                    ghostMenuCloseTimer = 0.15f;
                }
            }
        }

        return; // Don't process normal pause menu interactions
    }

    // Normal pause menu button handling
    double mouseX = InputManager::GetMouseWorldX();
    double mouseY = InputManager::GetMouseWorldY();
    bool isClicked = InputManager::IsMouseLeftClicked();

    std::string currentHover = "";

    // Don't process hover if we're in pending state (button is pressed)
    if (!pendingSceneChange)
    {
        if (IsMouseOver(factory.FindGameObject("PauseResumeButton"), mouseX, mouseY))
        {
            currentHover = "PauseResumeButton";
        }
        else if (IsMouseOver(factory.FindGameObject("PauseRestartButton"), mouseX, mouseY))
        {
            currentHover = "PauseRestartButton";
        }
        else if (IsMouseOver(factory.FindGameObject("PauseSettingsButton"), mouseX, mouseY))
        {
            currentHover = "PauseSettingsButton";
        }
        else if (IsMouseOver(factory.FindGameObject("PauseMainMenuButton"), mouseX, mouseY))
        {
            currentHover = "PauseMainMenuButton";
        }
        else if (IsMouseOver(factory.FindGameObject("PauseHowToPlayButton"), mouseX, mouseY))
        {
            currentHover = "PauseHowToPlayButton";
        }
        else if (IsMouseOver(factory.FindGameObject("PauseGhostButton"), mouseX, mouseY))
        {
            currentHover = "PauseGhostButton";
        }
    }

    if (currentHover != hoveredButtonName && !pendingSceneChange)
    {
        if (!hoveredButtonName.empty())
        {
            SetButtonTexture(hoveredButtonName, ButtonState::Normal);
        }

        if (!currentHover.empty())
        {
            SetButtonTexture(currentHover, ButtonState::Hover);
            audioManager = CORE->GetAudioManager();
            if (audioManager)
            {
                audioManager->TriggerEvent("UI_Hover");
            }
        }

        hoveredButtonName = currentHover;
    }

    // only process clicks if we are actually hovering over a button
    if (isClicked && !hoveredButtonName.empty())
    {
        // Set button to pressed state
        SetButtonTexture(hoveredButtonName, ButtonState::Pressed);
        // play click sound based on button type
        audioManager = CORE->GetAudioManager();
        if (audioManager)
        {
            if (hoveredButtonName == "PauseRestartButton")
            {
                audioManager->TriggerEvent("UI_Restart");
                // Start audio fade when Restart button is clicked
                audioManager->FadeOutAll(1.5f);
                std::cout << "[PauseMenu] Started fade out for restart (1.5 seconds)" << std::endl;
            }
            else
            {
                audioManager->TriggerEvent("UI_Click");
            }

            // Start audio fade when Main Menu button is clicked
            if (hoveredButtonName == "PauseMainMenuButton")
            {
                audioManager->FadeOutCategory("music", 2.0f);
                std::cout << "[PauseMenu] Started music fade out (2.0 seconds)" << std::endl;
            }
        }

        // start delay before executing action
        pendingSceneChange = true;
        // Set timer based on button type to allow fades to complete
        if (hoveredButtonName == "PauseMainMenuButton")
        {
            pendingTimer = 2.2f; // Main Menu: 2.0s fade + 0.2s buffer
        }
        else if (hoveredButtonName == "PauseRestartButton")
        {
            pendingTimer = 1.7f; // Restart: 1.5s fade + 0.2s buffer
        }
        else
        {
            pendingTimer = 0.20f; // Other buttons: immediate
        }
    }
}

/**
 * @brief Creates all UI elements for the pause menu.
 */
void PauseMenuLogicSystem::CreatePauseMenuUI()
{
    if (hasCreatedUI)
        return;

    auto& factory = GameObjectFactory::Instance();

    // Use a scale multiplier to make the UI larger
    float uiScale = 2.0f;

    // create semi-transparent overlay background
    GameObject* overlay = factory.CreateGameObject("PauseOverlay");
    auto* overlayTransform = overlay->AddComponent<TransformComponent>();
    overlayTransform->posX = 0.0f;
    overlayTransform->posY = 0.0f;
    overlayTransform->scaleX = 5000.0f * uiScale;
    overlayTransform->scaleY = 5000.0f * uiScale;
    auto* overlayRender = overlay->AddComponent<RenderComponent>();

    // check if we're in main menu - use medium grey, otherwise use dark overlay
    bool isMainMenu = (currentScenePath.find("MainMenu") != std::string::npos);
    if (isMainMenu)
    {
        overlayRender->SetColor(vec4(0.5f, 0.5f, 0.5f, 1.0f)); // medium grey solid background
        overlayRender->SetLayer(600); // very high layer to cover main menu elements including settings text
    }
    else
    {
        overlayRender->SetColor(vec4(0.0f, 0.0f, 0.0f, 0.7f)); // dark semi-transparent for in-game
        overlayRender->SetLayer(100); // normal layer for in-game
    }

    // pause board background
    GameObject* board = factory.CreateGameObject("PauseBoard");
    auto* boardTransform = board->AddComponent<TransformComponent>();
    boardTransform->posX = 0.0f;
    boardTransform->posY = 0.0f;
    boardTransform->scaleX = 1920.0f * uiScale;
    boardTransform->scaleY = 1080.0f * uiScale;
    auto* boardRender = board->AddComponent<RenderComponent>();
    boardRender->SetTextureID(GraphicsSystem::LoadTexture("../Assets/Sprites/Pause_Menu_New.png"));
    boardRender->SetColor(vec4(1.0f, 1.0f, 1.0f, 1.0f));
    boardRender->SetLayer(601); // above overlay

    // back button (resume)
    GameObject* pausedBtn = factory.CreateGameObject("PauseResumeButton");
    auto* pausedTransform = pausedBtn->AddComponent<TransformComponent>();
    pausedTransform->posX = 650.0f * uiScale;
    pausedTransform->posY = -275.0f * uiScale;
    pausedTransform->scaleX = 200.0f * uiScale;
    pausedTransform->scaleY = 175.0f * uiScale;
    auto* pausedRender = pausedBtn->AddComponent<RenderComponent>();
    pausedRender->SetTextureID(GraphicsSystem::LoadTexture("../Assets/Sprites/Icons/back_button.png"));
    pausedRender->SetColor(vec4(1.0f, 1.0f, 1.0f, 1.0f));
    pausedRender->SetLayer(602);

    // restart button
    GameObject* restartBtn = factory.CreateGameObject("PauseRestartButton");
    auto* restartTransform = restartBtn->AddComponent<TransformComponent>();
    restartTransform->posX = 0.0f;
    restartTransform->posY = -80.0f * uiScale;
    restartTransform->scaleX = 420.0f * uiScale;
    restartTransform->scaleY = 100.0f * uiScale;
    auto* restartRender = restartBtn->AddComponent<RenderComponent>();
    restartRender->SetTextureID(GraphicsSystem::LoadTexture("../Assets//Sprites/Icons/blankbtnL.png"));
    restartRender->SetColor(vec4(1.0f, 1.0f, 1.0f, 1.0f));
    restartRender->SetLayer(602);
    restartBtn->AddComponent<TextComponent>(
        "../Assets/FrederickatheGreat-Regular.ttf", static_cast<int>(38 * uiScale), "Restart", vec4(0.0f, 0.0f, 0.0f, 1.0f), false);

    // settings button
    GameObject* settingsBtn = factory.CreateGameObject("PauseSettingsButton");
    auto* settingsTransform = settingsBtn->AddComponent<TransformComponent>();
    settingsTransform->posX = 0.0f;
    settingsTransform->posY = 40.0f * uiScale;
    settingsTransform->scaleX = 420.0f * uiScale;
    settingsTransform->scaleY = 100.0f * uiScale;
    auto* settingsRender = settingsBtn->AddComponent<RenderComponent>();
    settingsRender->SetTextureID(GraphicsSystem::LoadTexture("../Assets/Sprites/Icons/blankbtnL.png"));
    settingsRender->SetColor(vec4(1.0f, 1.0f, 1.0f, 1.0f));
    settingsRender->SetLayer(602);
    settingsBtn->AddComponent<TextComponent>(
        "../Assets/FrederickatheGreat-Regular.ttf", static_cast<int>(38 * uiScale), "Settings", vec4(0.0f, 0.0f, 0.0f, 1.0f), false);

    // quit button
    GameObject* mainMenuBtn = factory.CreateGameObject("PauseMainMenuButton");
    auto* mainMenuTransform = mainMenuBtn->AddComponent<TransformComponent>();
    mainMenuTransform->posX = 0.0f;
    mainMenuTransform->posY = -200.0f * uiScale;
    mainMenuTransform->scaleX = 420.0f * uiScale;
    mainMenuTransform->scaleY = 100.0f * uiScale;
    auto* mainMenuRender = mainMenuBtn->AddComponent<RenderComponent>();
    mainMenuRender->SetTextureID(GraphicsSystem::LoadTexture("../Assets/Sprites/Icons/blankbtnL.png"));
    mainMenuRender->SetColor(vec4(1.0f, 1.0f, 1.0f, 1.0f));
    mainMenuRender->SetLayer(602);
    mainMenuBtn->AddComponent<TextComponent>(
        "../Assets/FrederickatheGreat-Regular.ttf", static_cast<int>(38 * uiScale), "Main Menu", vec4(0.0f, 0.0f, 0.0f, 1.0f), false);

    // how to play button
    GameObject* howToPlayBtn = factory.CreateGameObject("PauseHowToPlayButton");
    auto* howToPlayTransform = howToPlayBtn->AddComponent<TransformComponent>();
    howToPlayTransform->posX = 0.0f;
    howToPlayTransform->posY = -320.0f * uiScale;
    howToPlayTransform->scaleX = 420.0f * uiScale;
    howToPlayTransform->scaleY = 100.0f * uiScale;
    auto* howToPlayRender = howToPlayBtn->AddComponent<RenderComponent>();
    howToPlayRender->SetTextureID(GraphicsSystem::LoadTexture("../Assets/Sprites/Icons/blankbtnL.png"));
    howToPlayRender->SetColor(vec4(1.0f, 1.0f, 1.0f, 1.0f));
    howToPlayRender->SetLayer(602);
    howToPlayBtn->AddComponent<TextComponent>(
        "../Assets/FrederickatheGreat-Regular.ttf", static_cast<int>(38 * uiScale), "How To Play", vec4(0.0f, 0.0f, 0.0f, 1.0f), false);

    // ghost button — right side, above the resume/back button
    GameObject* ghostBtn = factory.CreateGameObject("PauseGhostButton");
    auto* ghostTransform = ghostBtn->AddComponent<TransformComponent>();
    ghostTransform->posX = 650.0f * uiScale;
    ghostTransform->posY = -100.0f * uiScale;
    ghostTransform->scaleX = 260.0f * uiScale;
    ghostTransform->scaleY = 100.0f * uiScale;
    auto* ghostRender = ghostBtn->AddComponent<RenderComponent>();
    ghostRender->SetTextureID(GraphicsSystem::LoadTexture("../Assets/Sprites/Icons/blankbtnL.png"));
    ghostRender->SetColor(vec4(1.0f, 1.0f, 1.0f, 1.0f));
    ghostRender->SetLayer(602);
    ghostBtn->AddComponent<TextComponent>(
        "../Assets/FrederickatheGreat-Regular.ttf", static_cast<int>(38 * uiScale), "Ghost", vec4(0.0f, 0.0f, 0.0f, 1.0f), false);

    hasCreatedUI = true;

    // initially hide the UI
    SetPauseMenuVisible(false);
}

/**
 * @brief Creates settings screen UI for the pause menu.
 */
void PauseMenuLogicSystem::CreatePauseSettingsUI()
{
    if (hasCreatedSettingsUI)
        return;

    auto& factory = GameObjectFactory::Instance();

    // Use a scale multiplier to make the UI larger
    float uiScale = 2.0f;

    // Settings screen background
    GameObject* settingsScreen = factory.CreateGameObject("PauseScreen_Settings");
    auto* settingsTransform = settingsScreen->AddComponent<TransformComponent>();
    settingsTransform->posX = 0.0f;
    settingsTransform->posY = 0.0f;
    settingsTransform->scaleX = 1920.0f * uiScale;
    settingsTransform->scaleY = 1080.0f * uiScale;
    auto* settingsRender = settingsScreen->AddComponent<RenderComponent>();
    settingsRender->SetTextureID(GraphicsSystem::LoadTexture("../Assets/Sprites/Settings_New.png"));
    settingsRender->SetColor(vec4(1.0f, 1.0f, 1.0f, 1.0f));
    settingsRender->SetLayer(601);

    // Master Text
    GameObject* masterText = factory.CreateGameObject("PauseMasterText");
    auto* masterTextTransform = masterText->AddComponent<TransformComponent>();
    masterTextTransform->posX = -350.0f * uiScale;
    masterTextTransform->posY = 50.0f * uiScale;
    masterTextTransform->scaleX = 1.0f;
    masterTextTransform->scaleY = 1.0f;
    auto* masterTextRender = masterText->AddComponent<RenderComponent>();
    masterTextRender->SetColor(vec4(1.0f, 0.5f, 0.5f, 1.0f));
    masterTextRender->SetLayer(603);
    masterText->AddComponent<TextComponent>(
        "../Assets/FrederickatheGreat-Regular.ttf", static_cast<int>(48 * uiScale), "Master", vec4(0.0f, 0.0f, 0.0f, 1.0f), false);

    // Music Text
    GameObject* musicText = factory.CreateGameObject("PauseMusicText");
    auto* musicTextTransform = musicText->AddComponent<TransformComponent>();
    musicTextTransform->posX = -340.0f * uiScale;
    musicTextTransform->posY = -75.0f * uiScale;
    musicTextTransform->scaleX = 1.0f;
    musicTextTransform->scaleY = 1.0f;
    auto* musicTextRender = musicText->AddComponent<RenderComponent>();
    musicTextRender->SetColor(vec4(1.0f, 0.5f, 0.5f, 1.0f));
    musicTextRender->SetLayer(603);
    musicText->AddComponent<TextComponent>(
        "../Assets/FrederickatheGreat-Regular.ttf", static_cast<int>(48 * uiScale), "Music", vec4(0.0f, 0.0f, 0.0f, 1.0f), false);

    // SFX Text
    GameObject* sfxText = factory.CreateGameObject("PauseSFXText");
    auto* sfxTextTransform = sfxText->AddComponent<TransformComponent>();
    sfxTextTransform->posX = -320.0f * uiScale;
    sfxTextTransform->posY = -200.0f * uiScale;
    sfxTextTransform->scaleX = 1.0f;
    sfxTextTransform->scaleY = 1.0f;
    auto* sfxTextRender = sfxText->AddComponent<RenderComponent>();
    sfxTextRender->SetColor(vec4(1.0f, 0.5f, 0.5f, 1.0f));
    sfxTextRender->SetLayer(603);
    sfxText->AddComponent<TextComponent>(
        "../Assets/FrederickatheGreat-Regular.ttf", static_cast<int>(48 * uiScale), "SFX", vec4(0.0f, 0.0f, 0.0f, 1.0f), false);

    // Master Slider - Bar
    GameObject* masterBar = factory.CreateGameObject("PauseSlider_Bar_Master");
    auto* masterBarTransform = masterBar->AddComponent<TransformComponent>();
    masterBarTransform->posX = 200.0f * uiScale;
    masterBarTransform->posY = 50.0f * uiScale;
    masterBarTransform->scaleX = 670.0f * uiScale;
    masterBarTransform->scaleY = 110.0f * uiScale;
    auto* masterBarRender = masterBar->AddComponent<RenderComponent>();
    masterBarRender->SetTextureID(GraphicsSystem::LoadTexture("../Assets/Sprites/Icons/Slider_Bar.png"));
    masterBarRender->SetColor(vec4(1.0f, 1.0f, 1.0f, 1.0f));
    masterBarRender->SetLayer(602);

    // Master Slider - Back
    GameObject* masterBarBack = factory.CreateGameObject("PauseSlider_Bar_Back_Master");
    auto* masterBarBackTransform = masterBarBack->AddComponent<TransformComponent>();
    masterBarBackTransform->posX = 200.0f * uiScale;
    masterBarBackTransform->posY = 50.0f * uiScale;
    masterBarBackTransform->scaleX = 670.0f * uiScale;
    masterBarBackTransform->scaleY = 110.0f * uiScale;
    auto* masterBarBackRender = masterBarBack->AddComponent<RenderComponent>();
    masterBarBackRender->SetTextureID(GraphicsSystem::LoadTexture("../Assets/Sprites/Icons/Slider_Bar_Back.png"));
    masterBarBackRender->SetColor(vec4(1.0f, 1.0f, 1.0f, 1.0f));
    masterBarBackRender->SetLayer(601);

    // Master Slider - Knob
    GameObject* masterKnob = factory.CreateGameObject("PauseSlider_Knob_Master");
    auto* masterKnobTransform = masterKnob->AddComponent<TransformComponent>();
    masterKnobTransform->posX = 200.0f * uiScale;
    masterKnobTransform->posY = 50.0f * uiScale;
    masterKnobTransform->scaleX = 670.0f * uiScale;
    masterKnobTransform->scaleY = 110.0f * uiScale;
    auto* masterKnobRender = masterKnob->AddComponent<RenderComponent>();
    masterKnobRender->SetTextureID(GraphicsSystem::LoadTexture("../Assets/Sprites/Icons/Slider_Icon.png"));
    masterKnobRender->SetColor(vec4(1.0f, 1.0f, 1.0f, 1.0f));
    masterKnobRender->SetLayer(603);

    // Music Slider - Bar
    GameObject* musicBar = factory.CreateGameObject("PauseSlider_Bar_Music");
    auto* musicBarTransform = musicBar->AddComponent<TransformComponent>();
    musicBarTransform->posX = 200.0f * uiScale;
    musicBarTransform->posY = -75.0f * uiScale;
    musicBarTransform->scaleX = 670.0f * uiScale;
    musicBarTransform->scaleY = 110.0f * uiScale;
    auto* musicBarRender = musicBar->AddComponent<RenderComponent>();
    musicBarRender->SetTextureID(GraphicsSystem::LoadTexture("../Assets/Sprites/Icons/Slider_Bar.png"));
    musicBarRender->SetColor(vec4(1.0f, 1.0f, 1.0f, 1.0f));
    musicBarRender->SetLayer(602);

    // Music Slider - Back
    GameObject* musicBarBack = factory.CreateGameObject("PauseSlider_Bar_Back_Music");
    auto* musicBarBackTransform = musicBarBack->AddComponent<TransformComponent>();
    musicBarBackTransform->posX = 200.0f * uiScale;
    musicBarBackTransform->posY = -75.0f * uiScale;
    musicBarBackTransform->scaleX = 670.0f * uiScale;
    musicBarBackTransform->scaleY = 110.0f * uiScale;
    auto* musicBarBackRender = musicBarBack->AddComponent<RenderComponent>();
    musicBarBackRender->SetTextureID(GraphicsSystem::LoadTexture("../Assets/Sprites/Icons/Slider_Bar_Back.png"));
    musicBarBackRender->SetColor(vec4(1.0f, 1.0f, 1.0f, 1.0f));
    musicBarBackRender->SetLayer(601);

    // Music Slider - Knob
    GameObject* musicKnob = factory.CreateGameObject("PauseSlider_Knob_Music");
    auto* musicKnobTransform = musicKnob->AddComponent<TransformComponent>();
    musicKnobTransform->posX = 200.0f * uiScale;
    musicKnobTransform->posY = -75.0f * uiScale;
    musicKnobTransform->scaleX = 670.0f * uiScale;
    musicKnobTransform->scaleY = 110.0f * uiScale;
    auto* musicKnobRender = musicKnob->AddComponent<RenderComponent>();
    musicKnobRender->SetTextureID(GraphicsSystem::LoadTexture("../Assets/Sprites/Icons/Slider_Icon.png"));
    musicKnobRender->SetColor(vec4(1.0f, 1.0f, 1.0f, 1.0f));
    musicKnobRender->SetLayer(603);

    // SFX Slider - Bar
    GameObject* sfxBar = factory.CreateGameObject("PauseSlider_Bar_SFX");
    auto* sfxBarTransform = sfxBar->AddComponent<TransformComponent>();
    sfxBarTransform->posX = 200.0f * uiScale;
    sfxBarTransform->posY = -200.0f * uiScale;
    sfxBarTransform->scaleX = 670.0f * uiScale;
    sfxBarTransform->scaleY = 110.0f * uiScale;
    auto* sfxBarRender = sfxBar->AddComponent<RenderComponent>();
    sfxBarRender->SetTextureID(GraphicsSystem::LoadTexture("../Assets/Sprites/Icons/Slider_Bar.png"));
    sfxBarRender->SetColor(vec4(1.0f, 1.0f, 1.0f, 1.0f));
    sfxBarRender->SetLayer(602);

    // SFX Slider - Back
    GameObject* sfxBarBack = factory.CreateGameObject("PauseSlider_Bar_Back_SFX");
    auto* sfxBarBackTransform = sfxBarBack->AddComponent<TransformComponent>();
    sfxBarBackTransform->posX = 200.0f * uiScale;
    sfxBarBackTransform->posY = -200.0f * uiScale;
    sfxBarBackTransform->scaleX = 670.0f * uiScale;
    sfxBarBackTransform->scaleY = 110.0f * uiScale;
    auto* sfxBarBackRender = sfxBarBack->AddComponent<RenderComponent>();
    sfxBarBackRender->SetTextureID(GraphicsSystem::LoadTexture("../Assets/Sprites/Icons/Slider_Bar_Back.png"));
    sfxBarBackRender->SetColor(vec4(1.0f, 1.0f, 1.0f, 1.0f));
    sfxBarBackRender->SetLayer(601);

    // SFX Slider - Knob
    GameObject* sfxKnob = factory.CreateGameObject("PauseSlider_Knob_SFX");
    auto* sfxKnobTransform = sfxKnob->AddComponent<TransformComponent>();
    sfxKnobTransform->posX = 200.0f * uiScale;
    sfxKnobTransform->posY = -200.0f * uiScale;
    sfxKnobTransform->scaleX = 670.0f * uiScale;
    sfxKnobTransform->scaleY = 110.0f * uiScale;
    auto* sfxKnobRender = sfxKnob->AddComponent<RenderComponent>();
    sfxKnobRender->SetTextureID(GraphicsSystem::LoadTexture("../Assets/Sprites/Icons/Slider_Icon.png"));
    sfxKnobRender->SetColor(vec4(1.0f, 1.0f, 1.0f, 1.0f));
    sfxKnobRender->SetLayer(603);

    // Master Mute Button
    GameObject* masterMute = factory.CreateGameObject("PauseMasterMuteButton");
    auto* masterMuteTransform = masterMute->AddComponent<TransformComponent>();
    masterMuteTransform->posX = -200.0f * uiScale;
    masterMuteTransform->posY = 50.5f * uiScale;
    masterMuteTransform->scaleX = 125.0f * uiScale;
    masterMuteTransform->scaleY = 100.0f * uiScale;
    auto* masterMuteRender = masterMute->AddComponent<RenderComponent>();
    masterMuteRender->SetTextureID(GraphicsSystem::LoadTexture("../Assets/Sprites/Icons/Volume_Unmute.png"));
    masterMuteRender->SetColor(vec4(1.0f, 1.0f, 1.0f, 1.0f));
    masterMuteRender->SetLayer(601);

    // Music Mute Button
    GameObject* musicMute = factory.CreateGameObject("PauseMusicMuteButton");
    auto* musicMuteTransform = musicMute->AddComponent<TransformComponent>();
    musicMuteTransform->posX = -200.0f * uiScale;
    musicMuteTransform->posY = -74.5f * uiScale;
    musicMuteTransform->scaleX = 125.0f * uiScale;
    musicMuteTransform->scaleY = 100.0f * uiScale;
    auto* musicMuteRender = musicMute->AddComponent<RenderComponent>();
    musicMuteRender->SetTextureID(GraphicsSystem::LoadTexture("../Assets/Sprites/Icons/Volume_Unmute.png"));
    musicMuteRender->SetColor(vec4(1.0f, 1.0f, 1.0f, 1.0f));
    musicMuteRender->SetLayer(601);

    // SFX Mute Button
    GameObject* sfxMute = factory.CreateGameObject("PauseSFXMuteButton");
    auto* sfxMuteTransform = sfxMute->AddComponent<TransformComponent>();
    sfxMuteTransform->posX = -200.0f * uiScale;
    sfxMuteTransform->posY = -199.5f * uiScale;
    sfxMuteTransform->scaleX = 125.0f * uiScale;
    sfxMuteTransform->scaleY = 100.0f * uiScale;
    auto* sfxMuteRender = sfxMute->AddComponent<RenderComponent>();
    sfxMuteRender->SetTextureID(GraphicsSystem::LoadTexture("../Assets/Sprites/Icons/Volume_Unmute.png"));
    sfxMuteRender->SetColor(vec4(1.0f, 1.0f, 1.0f, 1.0f));
    sfxMuteRender->SetLayer(601);

    // Back Button
    GameObject* backBtn = factory.CreateGameObject("PauseBackButton");
    auto* backBtnTransform = backBtn->AddComponent<TransformComponent>();
    backBtnTransform->posX = 650.0f * uiScale;
    backBtnTransform->posY = -250.0f * uiScale;
    backBtnTransform->scaleX = 200.0f * uiScale;
    backBtnTransform->scaleY = 180.0f * uiScale;
    auto* backBtnRender = backBtn->AddComponent<RenderComponent>();
    backBtnRender->SetTextureID(GraphicsSystem::LoadTexture("../Assets/Sprites/Icons/Back_button.png"));
    backBtnRender->SetColor(vec4(1.0f, 1.0f, 1.0f, 1.0f));
    backBtnRender->SetLayer(601);

    hasCreatedSettingsUI = true;

    // Initially hide settings UI
    SetPauseSettingsVisible(false);
}

/**
 * @brief Sets visibility of pause settings UI elements.
 */
void PauseMenuLogicSystem::SetPauseSettingsVisible(bool show)
{
    auto& factory = GameObjectFactory::Instance();

    std::vector<std::string> settingsElements = {
        "PauseScreen_Settings",
        "PauseMasterText",
        "PauseMusicText",
        "PauseSFXText",
        "PauseSlider_Bar_Master",
        "PauseSlider_Bar_Back_Master",
        "PauseSlider_Knob_Master",
        "PauseSlider_Bar_Music",
        "PauseSlider_Bar_Back_Music",
        "PauseSlider_Knob_Music",
        "PauseSlider_Bar_SFX",
        "PauseSlider_Bar_Back_SFX",
        "PauseSlider_Knob_SFX",
        "PauseMasterMuteButton",
        "PauseMusicMuteButton",
        "PauseSFXMuteButton",
        "PauseBackButton"
    };

    for (const auto& name : settingsElements)
    {
        GameObject* obj = factory.FindGameObject(name);
        if (obj)
        {
            obj->SetActive(show);
        }
    }
}

/**
 * @brief Destroys all pause menu UI elements.
 */
void PauseMenuLogicSystem::DestroyPauseMenuUI()
{
    if (!hasCreatedUI)
        return;

    auto& factory = GameObjectFactory::Instance();

    // xestroy all ui elements on pause menu
    std::vector<std::string> uiElements = {
        "PauseOverlay",
        "PauseBoard",
        "PauseResumeButton",
        "PauseRestartButton",
        "PauseSettingsButton",
        "PauseMainMenuButton",
        "PauseHowToPlayButton",
        "PauseGhostButton"
    };

    for (const auto& name : uiElements)
    {
        GameObject* obj = factory.FindGameObject(name);
        if (obj)
        {
            factory.DestroyGameObject(obj->GetID());
        }
    }

    hasCreatedUI = false;
    std::cout << "[PauseMenu] UI destroyed" << std::endl;
}

/**
 * @brief Destroys pause settings UI elements.
 */
void PauseMenuLogicSystem::DestroyPauseSettingsUI()
{
    if (!hasCreatedSettingsUI)
        return;

    auto& factory = GameObjectFactory::Instance();

    std::vector<std::string> settingsElements = {
        "PauseScreen_Settings",
        "PauseMasterText",
        "PauseMusicText",
        "PauseSFXText",
        "PauseSlider_Bar_Master",
        "PauseSlider_Bar_Back_Master",
        "PauseSlider_Knob_Master",
        "PauseSlider_Bar_Music",
        "PauseSlider_Bar_Back_Music",
        "PauseSlider_Knob_Music",
        "PauseSlider_Bar_SFX",
        "PauseSlider_Bar_Back_SFX",
        "PauseSlider_Knob_SFX",
        "PauseMasterMuteButton",
        "PauseMusicMuteButton",
        "PauseSFXMuteButton",
        "PauseBackButton"
    };

    for (const auto& name : settingsElements)
    {
        GameObject* obj = factory.FindGameObject(name);
        if (obj)
        {
            factory.DestroyGameObject(obj->GetID());
        }
    }

    hasCreatedSettingsUI = false;
    std::cout << "[PauseMenu] Settings UI destroyed" << std::endl;
}

/**
 * @brief Shows or hides pause menu UI elements.
 */
void PauseMenuLogicSystem::SetPauseMenuVisible(bool show)
{
    auto& factory = GameObjectFactory::Instance();

    std::vector<std::string> uiElements = {
        "PauseOverlay",
        "PauseBoard",
        "PauseResumeButton",
        "PauseRestartButton",
        "PauseSettingsButton",
        "PauseMainMenuButton",
        "PauseHowToPlayButton",
        "PauseGhostButton"
    };

    for (const auto& name : uiElements)
    {
        GameObject* obj = factory.FindGameObject(name);
        if (obj)
        {
            obj->SetActive(show);
        }
    }

    // update overlay color and layer based on current scene when showing
    if (show)
    {
        GameObject* overlay = factory.FindGameObject("PauseOverlay");
        if (overlay)
        {
            auto* overlayRender = overlay->GetComponent<RenderComponent>();
            if (overlayRender)
            {
                // check if when in main menu
                bool isMainMenu = (currentScenePath.find("MainMenu") != std::string::npos);
                std::cout << "[PauseMenu] Current scene: " << currentScenePath << ", isMainMenu: " << isMainMenu << std::endl;
                if (isMainMenu)
                {
                    overlayRender->SetColor(vec4(0.5f, 0.5f, 0.5f, 1.0f)); // medium grey solid background
                    overlayRender->SetLayer(600); // very high layer to cover main menu elements including settings text
                }
                else
                {
                    overlayRender->SetColor(vec4(0.0f, 0.0f, 0.0f, 0.3f)); // semi-transparent for in-game
                    overlayRender->SetLayer(100); // normal layer
                }
            }
        }

        // Hide settings text labels when pause menu is showing
        if (auto* masterText = factory.FindGameObject("MasterText"))
            masterText->SetActive(false);
        if (auto* musicText = factory.FindGameObject("MusicText"))
            musicText->SetActive(false);
        if (auto* sfxText = factory.FindGameObject("SFXText"))
            sfxText->SetActive(false);
    }
    else
    {
        // Restore settings text labels visibility when pause menu is hidden
        // Only show them if we're actually in the settings screen
        bool isMainMenu = (currentScenePath.find("MainMenu") != std::string::npos);
        if (isMainMenu)
        {
            // Settings text will be managed by MenuLogic based on current screen
            if (auto* settingsScreen = factory.FindGameObject("Screen_Settings"))
            {
                if (settingsScreen->IsActive())
                {
                    if (auto* masterText = factory.FindGameObject("MasterText"))
                        masterText->SetActive(true);
                    if (auto* musicText = factory.FindGameObject("MusicText"))
                        musicText->SetActive(true);
                    if (auto* sfxText = factory.FindGameObject("SFXText"))
                        sfxText->SetActive(true);
                }
            }
        }
    }
}

/**
 * @brief Toggles the pause menu open or closed and pauses/unpauses the game.
 */
void PauseMenuLogicSystem::TogglePauseMenu()
{
    // create ui on first toggle if not created
    if (!hasCreatedUI)
    {
        CreatePauseMenuUI();
    }

    // create settings UI on first toggle to avoid delays later
    if (!hasCreatedSettingsUI)
    {
        CreatePauseSettingsUI();
    }

    // create how to play UI on first toggle to avoid delays later
    if (!hasCreatedHowToPlayUI)
    {
        CreateHowToPlayUI();
    }

    // create ghost menu UI on first toggle to avoid delays later
    if (!hasCreatedGhostMenuUI)
    {
        CreateGhostMenuUI();
    }

    isPauseMenuActive = !isPauseMenuActive;
    SetPauseMenuVisible(isPauseMenuActive);

    // Get audio manager for low pass filter
    AudioManager* audioManager = CORE->GetAudioManager();

    // Reset pause settings and how to play state when closing pause menu
    if (!isPauseMenuActive)
    {
        isPauseSettingsOpen = false;
        wasSettingsOpenLastFrame = false;
        isHowToPlayOpen = false;
        wasHowToPlayOpenLastFrame = false;
        howToPlayOpenedViaTab = false;
        isGhostMenuOpen = false;
        wasGhostMenuOpenLastFrame = false;

        // Reset all button textures when closing pause menu
        SetButtonTexture("PauseResumeButton", ButtonState::Normal);
        SetButtonTexture("PauseRestartButton", ButtonState::Normal);
        SetButtonTexture("PauseSettingsButton", ButtonState::Normal);
        SetButtonTexture("PauseMainMenuButton", ButtonState::Normal);
        SetButtonTexture("PauseHowToPlayButton", ButtonState::Normal);
        SetButtonTexture("PauseGhostButton", ButtonState::Normal);

        // remove low pass filter when pause menu closes
        if (audioManager)
        {
            audioManager->SetMasterLowPassFilter(false);
            //std::cout << "[PauseMenu] Low pass filter disabled" << std::endl;
        }
    }
    else
    {
        // apply low pass filter when pause menu opens
        if (audioManager)
        {
            audioManager->SetMasterLowPassFilter(true, 800.0f, 1.0f);
            //std::cout << "[PauseMenu] Low pass filter enabled" << std::endl;
        }
    }

    // pause/unpause the game but don't pause audio, using low pass filter instead
    CORE->SetGamePaused(isPauseMenuActive, false);
}

/**
 * @brief Checks whether the mouse overlaps a UI button area.
 */
bool PauseMenuLogicSystem::IsMouseOver(GameObject* button, double mouseX, double mouseY)
{
    if (!button) return false;
    if (!button->IsActive()) return false;

    auto* tx = button->GetComponent<TransformComponent>();
    if (!tx) return false;

    return IsAreaClicked(tx->posX, tx->posY, tx->scaleX, tx->scaleY, static_cast<float>(mouseX), static_cast<float>(mouseY));
}

/**
 * @brief Updates button texture based on button state (Normal, Hover, Pressed).
 */
void PauseMenuLogicSystem::SetButtonTexture(std::string buttonName, ButtonState state)
{
    auto& factory = GameObjectFactory::Instance();
    GameObject* btn = factory.FindGameObject(buttonName);
    if (!btn) return;

    auto* rc = btn->GetComponent<RenderComponent>();
    if (!rc) return;

    std::string texturePath;

    if (buttonName == "PauseResumeButton")
    {
        // Resume button: back button (default), back button light (hover), back button dark (pressed)
        switch (state)
        {
            case ButtonState::Normal:
                texturePath = "../Assets/Sprites/Icons/back_button.png";
                break;
            case ButtonState::Hover:
                texturePath = "../Assets/Sprites/Icons/Back_button_light.png";
                break;
            case ButtonState::Pressed:
                texturePath = "../Assets/Sprites/Icons/Back_button_dark.png";
                break;
        }
    }
    else if (buttonName == "PauseMasterMuteButton" || buttonName == "PauseMusicMuteButton" || buttonName == "PauseSFXMuteButton")
    {
        // Mute buttons show mute/unmute state, not hover state
        bool isMuted = false;
        AudioManager* audioManager = CORE->GetAudioManager();
        if (audioManager)
        {
            if (buttonName == "PauseMasterMuteButton")
                isMuted = audioManager->IsMuted();
            else if (buttonName == "PauseMusicMuteButton")
                isMuted = audioManager->IsCategoryMuted("music");
            else if (buttonName == "PauseSFXMuteButton")
                isMuted = audioManager->IsCategoryMuted("sfx");
        }
        texturePath = isMuted ? "../Assets/Sprites/Icons/Volume_Mute.png" : "../Assets/Sprites/Icons/Volume_Unmute.png";
    }
    else if (buttonName == "PauseBackButton" || buttonName == "HowToPlayBackButton")
    {
        // Back button: normal, light (hover), dark (pressed)
        switch (state)
        {
            case ButtonState::Normal:
                texturePath = "../Assets/Sprites/Icons/Back_button.png";
                break;
            case ButtonState::Hover:
                texturePath = "../Assets/Sprites/Icons/Back_button_light.png";
                break;
            case ButtonState::Pressed:
                texturePath = "../Assets/Sprites/Icons/Back_button_dark.png";
                break;
        }
    }
    else if (buttonName == "PauseHowToPlayButton")
    {
        // How to play button: default (blank), hover (light), pressed (dark)
        switch (state)
        {
            case ButtonState::Normal:
                texturePath = "../Assets/Sprites/Icons/blankbtnL.png";
                break;
            case ButtonState::Hover:
                texturePath = "../Assets/Sprites/Icons/Button_Large_light.png";
                break;
            case ButtonState::Pressed:
                texturePath = "../Assets/Sprites/Icons/Button Large_dark.png";
                break;
        }
    }
    else if (buttonName == "PauseGhostButton" || buttonName == "PauseGhost_Toggle" ||
             buttonName == "PauseGhost_Slot0" || buttonName == "PauseGhost_Slot1" ||
             buttonName == "PauseGhost_Slot2")
    {
        // Ghost buttons: blank/light/dark
        switch (state)
        {
            case ButtonState::Normal:
                texturePath = "../Assets/Sprites/Icons/blankbtnL.png";
                break;
            case ButtonState::Hover:
                texturePath = "../Assets/Sprites/Icons/Button_Large_light.png";
                break;
            case ButtonState::Pressed:
                texturePath = "../Assets/Sprites/Icons/Button Large_dark.png";
                break;
        }
    }
    else if (buttonName == "PauseGhost_Back")
    {
        switch (state)
        {
            case ButtonState::Normal:
                texturePath = "../Assets/Sprites/Icons/Back_button.png";
                break;
            case ButtonState::Hover:
                texturePath = "../Assets/Sprites/Icons/Back_button_light.png";
                break;
            case ButtonState::Pressed:
                texturePath = "../Assets/Sprites/Icons/Back_button_dark.png";
                break;
        }
    }
    else
    {
        // Standard pause menu buttons: default (blank), hover (light), pressed (dark)
        switch (state)
        {
            case ButtonState::Normal:
                texturePath = "../Assets/Sprites/Icons/blankbtnL.png";
                break;
            case ButtonState::Hover:
                texturePath = "../Assets/Sprites/Icons/Button_Large_light.png";
                break;
            case ButtonState::Pressed:
                texturePath = "../Assets/Sprites/Icons/Button Large_dark.png";
                break;
        }
    }

    std::cout << "[SetButtonTexture] Button: " << buttonName << " | State: "
              << (state == ButtonState::Normal ? "Normal" : state == ButtonState::Hover ? "Hover" : "Pressed")
              << " | Texture: " << texturePath << std::endl;

    unsigned int newtex = GraphicsSystem::LoadTexture(texturePath.c_str());
    rc->SetTextureID(newtex);
}

/**
 * @brief Opens the How To Play screen (creates pause menu if needed).
 */
void PauseMenuLogicSystem::OpenHowToPlay()
{
    auto& factory = GameObjectFactory::Instance();

    // Create pause menu UI if not created
    if (!hasCreatedUI)
    {
        CreatePauseMenuUI();
    }

    // Create how to play UI if not created
    if (!hasCreatedHowToPlayUI)
    {
        CreateHowToPlayUI();
    }

    // Open pause menu if not active
    if (!isPauseMenuActive)
    {
        isPauseMenuActive = true;
        // DON'T pause the game when opening How To Play - let audio/game continue
        // Only the timer will stop (handled in GameTimerSystem)
        // CORE->SetGamePaused(true); // Commented out - don't pause

        // Show overlay only (not board, since we're showing How To Play instead)
        if (auto* overlay = factory.FindGameObject("PauseOverlay"))
            overlay->SetActive(true);

        std::cout << "[PauseMenu] Pause menu auto-opened for How To Play (game NOT paused)" << std::endl;
    }

    // Hide pause menu buttons and board
    if (auto* resumeBtn = factory.FindGameObject("PauseResumeButton"))
        resumeBtn->SetActive(false);
    if (auto* restartBtn = factory.FindGameObject("PauseRestartButton"))
        restartBtn->SetActive(false);
    if (auto* settingsBtn = factory.FindGameObject("PauseSettingsButton"))
        settingsBtn->SetActive(false);
    if (auto* mainMenuBtn = factory.FindGameObject("PauseMainMenuButton"))
        mainMenuBtn->SetActive(false);
    if (auto* howToPlayBtn = factory.FindGameObject("PauseHowToPlayButton"))
        howToPlayBtn->SetActive(false);
    if (auto* board = factory.FindGameObject("PauseBoard"))
        board->SetActive(false);

    // Show how to play screen
    SetHowToPlayVisible(true);

    // apply low pass filter when how to play opens
    AudioManager* audioManager = CORE->GetAudioManager();
    if (audioManager)
    {
        audioManager->SetMasterLowPassFilter(true, 800.0f, 1.0f);
        // std::cout << "[PauseMenu] Low pass filter enabled for How To Play (TAB)" << std::endl;
    }

    // Open how to play screen
    isHowToPlayOpen = true;
    howToPlayOpenedViaTab = true;
    //std::cout << "[PauseMenu] How To Play opened via TAB" << std::endl;
}

/**
 * @brief Closes the How To Play screen.
 */
void PauseMenuLogicSystem::CloseHowToPlay()
{
    // remove low pass filter when how to play closes
    AudioManager* audioManager = CORE->GetAudioManager();
    if (audioManager)
    {
        audioManager->SetMasterLowPassFilter(false);
        //std::cout << "[PauseMenu] Low pass filter disabled for How To Play" << std::endl;
    }

    isHowToPlayOpen = false;
    wasHowToPlayOpenLastFrame = false; // Reset state tracking

    // If How To Play was opened via TAB, close the entire pause menu
    if (howToPlayOpenedViaTab)
    {
        std::cout << "[PauseMenu] How To Play closed via TAB - closing pause menu" << std::endl;
        howToPlayOpenedViaTab = false;

        if (isPauseMenuActive)
        {
            isPauseMenuActive = false;
            SetPauseMenuVisible(false);
            SetHowToPlayVisible(false);
            CORE->SetGamePaused(false, false); // Don't unpause audio (using low pass filter)
        }
    }
    else
    {
        std::cout << "[PauseMenu] How To Play closed" << std::endl;
    }
}

/**
 * @brief Closes the pause settings screen.
 */
void PauseMenuLogicSystem::ClosePauseSettings()
{
    std::cout << "[PauseMenu] Closing settings screen (ESC pressed)" << std::endl;
    pendingSettingsClose = true;
    settingsCloseTimer = 0.15f; // 150ms delay to match back button behavior
}

void PauseMenuLogicSystem::CloseGhostMenu()
{
    std::cout << "[PauseMenu] Closing ghost menu (ESC pressed)" << std::endl;
    pendingGhostMenuClose = true;
    ghostMenuCloseTimer = 0.15f;
}

/**
 * @brief Creates how to play screen UI.
 */
void PauseMenuLogicSystem::CreateHowToPlayUI()
{
    if (hasCreatedHowToPlayUI)
        return;

    auto& factory = GameObjectFactory::Instance();

    float uiScale = 2.0f;

    // How to play screen background
    GameObject* howToPlayScreen = factory.CreateGameObject("PauseScreen_HowToPlay");
    auto* howToPlayTransform = howToPlayScreen->AddComponent<TransformComponent>();
    howToPlayTransform->posX = 0.0f;
    howToPlayTransform->posY = 0.0f;
    howToPlayTransform->scaleX = 1920.0f * uiScale;
    howToPlayTransform->scaleY = 1080.0f * uiScale;
    auto* howToPlayRender = howToPlayScreen->AddComponent<RenderComponent>();
    howToPlayRender->SetTextureID(GraphicsSystem::LoadTexture("../Assets/Sprites/M5_Control_Panel.png"));
    howToPlayRender->SetColor(vec4(1.0f, 1.0f, 1.0f, 1.0f));
    howToPlayRender->SetLayer(601);

    GameObject* backBtn = factory.CreateGameObject("HowToPlayBackButton");
    auto* backBtnTransform = backBtn->AddComponent<TransformComponent>();
    backBtnTransform->posX = 625.0f * uiScale;
    backBtnTransform->posY = -235.0f * uiScale;
    backBtnTransform->scaleX = 200.0f * uiScale;
    backBtnTransform->scaleY = 180.0f * uiScale;
    auto* backBtnRender = backBtn->AddComponent<RenderComponent>();
    backBtnRender->SetTextureID(GraphicsSystem::LoadTexture("../Assets/Sprites/Icons/Back_button.png"));
    backBtnRender->SetColor(vec4(1.0f, 1.0f, 1.0f, 1.0f));
    backBtnRender->SetLayer(602);

    hasCreatedHowToPlayUI = true;

    // Initially hide UI
    SetHowToPlayVisible(false);
}

/**
 * @brief Destroys how to play UI elements.
 */
void PauseMenuLogicSystem::DestroyHowToPlayUI()
{
    if (!hasCreatedHowToPlayUI)
        return;

    auto& factory = GameObjectFactory::Instance();

    std::vector<std::string> howToPlayElements = {
        "PauseScreen_HowToPlay",
        "HowToPlayBackButton"
    };

    for (const auto& name : howToPlayElements)
    {
        GameObject* obj = factory.FindGameObject(name);
        if (obj)
        {
            factory.DestroyGameObject(obj->GetID());
        }
    }

    hasCreatedHowToPlayUI = false;
    std::cout << "[PauseMenu] How To Play UI destroyed" << std::endl;
}

/**
 * @brief Sets visibility of how to play UI elements.
 */
void PauseMenuLogicSystem::SetHowToPlayVisible(bool show)
{
    auto& factory = GameObjectFactory::Instance();

    std::vector<std::string> howToPlayElements = {
        "PauseScreen_HowToPlay",
        "HowToPlayBackButton"
    };

    for (const auto& name : howToPlayElements)
    {
        GameObject* obj = factory.FindGameObject(name);
        if (obj)
        {
            obj->SetActive(show);
        }
    }
}

/**
 * @brief Scans the replay directory and fills m_ghostReplayNames / m_ghostReplayLevels.
 */
void PauseMenuLogicSystem::ScanReplaysForGhost()
{
    m_ghostReplayNames.clear();
    m_ghostReplayLevels.clear();
    namespace fs = std::filesystem;

    const std::string replayDir = "../Assets/Replay/";
    if (!fs::exists(replayDir))
        return;

    std::vector<std::pair<fs::file_time_type, std::string>> files;
    for (const auto& entry : fs::directory_iterator(replayDir))
    {
        if (entry.is_regular_file() && entry.path().extension() == ".json")
            files.push_back({entry.last_write_time(), entry.path().stem().string()});
    }

    std::sort(files.begin(), files.end(),
              [](const auto& a, const auto& b) { return a.first > b.first; });

    for (size_t i = 0; i < std::min(files.size(), size_t(3)); ++i)
    {
        const std::string& stem = files[i].second;
        m_ghostReplayNames.push_back(stem);

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
        m_ghostReplayLevels.push_back(levelName);
    }
}

/**
 * @brief Creates the Ghost menu screen UI.
 */
void PauseMenuLogicSystem::CreateGhostMenuUI()
{
    if (hasCreatedGhostMenuUI)
        return;

    ScanReplaysForGhost();

    auto& factory = GameObjectFactory::Instance();
    const float uiScale = 2.0f;
    const std::string font = "../Assets/FrederickatheGreat-Regular.ttf";
    const int baseLayer = 601;

    // Background board (reuse pause menu texture)
    GameObject* board = factory.CreateGameObject("PauseGhostBoard");
    auto* boardT = board->AddComponent<TransformComponent>();
    boardT->posX = 0.0f;
    boardT->posY = 0.0f;
    boardT->scaleX = 1920.0f * uiScale;
    boardT->scaleY = 1080.0f * uiScale;
    auto* boardR = board->AddComponent<RenderComponent>();
    boardR->SetTextureID(GraphicsSystem::LoadTexture("../Assets/Sprites/Pause_Menu_New.png"));
    boardR->SetColor(vec4(1.0f, 1.0f, 1.0f, 1.0f));
    boardR->SetLayer(baseLayer);

    // Title text
    GameObject* titleObj = factory.CreateGameObject("PauseGhostTitle");
    auto* titleT = titleObj->AddComponent<TransformComponent>();
    titleT->posX = 0.0f;
    titleT->posY = 250.0f * uiScale;
    titleT->scaleX = 420.0f * uiScale;
    titleT->scaleY = 100.0f * uiScale;
    auto* titleR = titleObj->AddComponent<RenderComponent>();
    titleR->SetTextureID(GraphicsSystem::LoadTexture("../Assets/Sprites/Icons/blankbtnL.png"));
    titleR->SetColor(vec4(1.0f, 1.0f, 1.0f, 1.0f));
    titleR->SetLayer(baseLayer + 1);
    titleObj->AddComponent<TextComponent>(
        font.c_str(), static_cast<int>(42 * uiScale), "GHOST", vec4(0.0f, 0.0f, 0.0f, 1.0f), false);

    // Ghost ON/OFF toggle button
    GhostSystem* ghost = CORE ? CORE->GetGhostSystem() : nullptr;
    bool ghostVisible = ghost ? ghost->IsGhostVisible() : false;
    std::string toggleLabel = std::string("Ghost: ") + (ghostVisible ? "ON" : "OFF");

    GameObject* toggleBtn = factory.CreateGameObject("PauseGhost_Toggle");
    auto* toggleT = toggleBtn->AddComponent<TransformComponent>();
    toggleT->posX = 0.0f;
    toggleT->posY = 100.0f * uiScale;
    toggleT->scaleX = 420.0f * uiScale;
    toggleT->scaleY = 100.0f * uiScale;
    auto* toggleR = toggleBtn->AddComponent<RenderComponent>();
    toggleR->SetTextureID(GraphicsSystem::LoadTexture("../Assets/Sprites/Icons/blankbtnL.png"));
    toggleR->SetColor(vec4(1.0f, 1.0f, 1.0f, 1.0f));
    toggleR->SetLayer(baseLayer + 1);
    toggleBtn->AddComponent<TextComponent>(
        font.c_str(), static_cast<int>(38 * uiScale), toggleLabel, vec4(0.0f, 0.0f, 0.0f, 1.0f), false);

    // 3 replay slots
    const float slotStartY = -30.0f * uiScale;
    const float slotSpacing = 120.0f * uiScale;
    for (int i = 0; i < 3; ++i)
    {
        std::string slotName = "PauseGhost_Slot" + std::to_string(i);
        bool hasReplay = (i < static_cast<int>(m_ghostReplayNames.size()));
        std::string displayName;
        if (hasReplay)
        {
            displayName = m_ghostReplayNames[i];
            if (i < static_cast<int>(m_ghostReplayLevels.size()) && !m_ghostReplayLevels[i].empty())
                displayName += "  [" + m_ghostReplayLevels[i] + "]";
        }
        else
        {
            displayName = "(empty)";
        }

        GameObject* slot = factory.CreateGameObject(slotName);
        auto* slotT = slot->AddComponent<TransformComponent>();
        slotT->posX = 0.0f;
        slotT->posY = slotStartY - i * slotSpacing;
        slotT->scaleX = 420.0f * uiScale;
        slotT->scaleY = 100.0f * uiScale;
        auto* slotR = slot->AddComponent<RenderComponent>();
        slotR->SetTextureID(GraphicsSystem::LoadTexture("../Assets/Sprites/Icons/blankbtnL.png"));
        slotR->SetColor(hasReplay ? vec4(1.0f, 1.0f, 1.0f, 1.0f) : vec4(0.55f, 0.55f, 0.55f, 1.0f));
        slotR->SetLayer(baseLayer + 1);
        vec4 textColor = hasReplay ? vec4(0.0f, 0.0f, 0.0f, 1.0f) : vec4(0.3f, 0.3f, 0.3f, 1.0f);
        slot->AddComponent<TextComponent>(
            font.c_str(), static_cast<int>(30 * uiScale), displayName, textColor, false);
    }

    // Back button
    GameObject* backBtn = factory.CreateGameObject("PauseGhost_Back");
    auto* backT = backBtn->AddComponent<TransformComponent>();
    backT->posX = 650.0f * uiScale;
    backT->posY = -250.0f * uiScale;
    backT->scaleX = 200.0f * uiScale;
    backT->scaleY = 180.0f * uiScale;
    auto* backR = backBtn->AddComponent<RenderComponent>();
    backR->SetTextureID(GraphicsSystem::LoadTexture("../Assets/Sprites/Icons/Back_button.png"));
    backR->SetColor(vec4(1.0f, 1.0f, 1.0f, 1.0f));
    backR->SetLayer(baseLayer + 1);

    hasCreatedGhostMenuUI = true;

    // Initially hidden
    SetGhostMenuVisible(false);
}

/**
 * @brief Destroys all Ghost menu UI elements.
 */
void PauseMenuLogicSystem::DestroyGhostMenuUI()
{
    if (!hasCreatedGhostMenuUI)
        return;

    auto& factory = GameObjectFactory::Instance();
    for (const std::string& name : {
        "PauseGhostBoard", "PauseGhostTitle",
        "PauseGhost_Toggle",
        "PauseGhost_Slot0", "PauseGhost_Slot1", "PauseGhost_Slot2",
        "PauseGhost_Back"})
    {
        GameObject* obj = factory.FindGameObject(name);
        if (obj) factory.DestroyGameObject(obj->GetID());
    }

    hasCreatedGhostMenuUI = false;
    m_ghostReplayNames.clear();
    m_ghostReplayLevels.clear();
}

/**
 * @brief Shows or hides Ghost menu UI elements.
 */
void PauseMenuLogicSystem::SetGhostMenuVisible(bool show)
{
    auto& factory = GameObjectFactory::Instance();
    for (const std::string& name : {
        "PauseGhostBoard", "PauseGhostTitle",
        "PauseGhost_Toggle",
        "PauseGhost_Slot0", "PauseGhost_Slot1", "PauseGhost_Slot2",
        "PauseGhost_Back"})
    {
        GameObject* obj = factory.FindGameObject(name);
        if (obj) obj->SetActive(show);
    }
}
