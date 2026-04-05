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
#include "stb_image.h"
#include "TextComponent.h"
#include <iostream>
#include <filesystem>
#include <algorithm>
#include <fstream>
#include <cmath>
#include <cfloat>
#include <nlohmann/json.hpp>
#include "ColliderComponent.h"
#include "Collision.h"

/**
 * @brief Initializes all pause menu state variables.
 */
void PauseMenuLogicSystem::Initialize()
{
    // Clean up any existing UI before resetting
    DestroyLeaderboardUI();
    DestroyHowToPlayUI();
    DestroyGhostMenuUI();
    DestroyPauseSettingsUI();
    DestroyPauseMenuUI();

    // Remove any lingering low pass filter from previous scene/restart
    AudioManager* audioManager = CORE->GetAudioManager();
    if (audioManager)
    {
        audioManager->SetLowPassFilter("bgm", false);
        //std::cout << "[PauseMenu] Initialize - low pass filter cleaned up" << std::endl;
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

    isLeaderboardOpen = false;
    wasLeaderboardOpenLastFrame = false;
    hasCreatedLeaderboardUI = false;
    pendingLeaderboardClose = false;
    leaderboardCloseTimer = 0.0f;
    m_lbLevel = 1;
    m_lbData.clear();

    CORE->GetMessageBus().Subscribe<WindowFocusMessage>(static_cast<IMessageHandler*>(this));
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

            float screenWidth = (float)graphics->GetWindowWidth();
            float screenHeight = (float)graphics->GetWindowHeight();
            float uiScale = screenHeight / 1080.0f;

            // Convert percentage pos to actual pixels (Y is bottom-left origin, flip for GLFW top-left)
            float btnX = (transform->posX / 100.0f) * screenWidth;
            float btnY = screenHeight - ((transform->posY / 100.0f) * screenHeight);
            float halfWidth = (transform->scaleX * uiScale) / 2.0f;
            float halfHeight = (transform->scaleY * uiScale) / 2.0f;

            bool isHoveringPauseBtn = (mouseX >= btnX - halfWidth && mouseX <= btnX + halfWidth &&
                mouseY >= btnY - halfHeight && mouseY <= btnY + halfHeight);

            static bool wasHoveringLastFrame = false;

            if (isClicked && isHoveringPauseBtn && !pendingSceneChange)
            {
                // Pressed state (dark)
                SetButtonTexture("InGamePauseButton", ButtonState::Pressed);

                // Play click sound
                //AudioManager* audioManager = CORE->GetAudioManager();
                if (audioManager)
                {
                    audioManager->TriggerEvent("UI_Click");
                }

                // Set pending flag to delay pause menu opening
                pendingSceneChange = true;
                hoveredButtonName = "InGamePauseButton";
                pendingTimer = 0.15f; // 150ms delay to show pressed state
                //std::cout << "[InGamePauseButton] CLICKED! Opening pause menu after delay" << std::endl;
                }
                else if (isHoveringPauseBtn && !pendingSceneChange)
                {
                // Hover state (light)
                SetButtonTexture("InGamePauseButton", ButtonState::Hover);

                // Play hover sound only on hover state change
                if (!wasHoveringLastFrame)
                {
                    //AudioManager* audioManager = CORE->GetAudioManager();
                    if (audioManager)
                    {
                        audioManager->TriggerEvent("UI_Hover");
                    }
                }
                wasHoveringLastFrame = true;
                }
            else if (!pendingSceneChange)
            {
                // Normal state
                SetButtonTexture("InGamePauseButton", ButtonState::Normal);
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
            "PauseLeaderboardButton",
            "PauseScreen_Settings",
            "PauseMasterText",
            "PauseMusicText",
            "PauseSFXText",
            //"PauseSlider_Bar_Master",
            "PauseSlider_Bar_Back_Master",  // background bars follow camera normally
            //"PauseSlider_Knob_Master",
            //"PauseSlider_Bar_Music",
            "PauseSlider_Bar_Back_Music",
            //"PauseSlider_Knob_Music",
            //"PauseSlider_Bar_SFX",
            "PauseSlider_Bar_Back_SFX",
            //"PauseSlider_Knob_SFX",
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
            "PauseGhost_Back",
            "PauseLB_Board",
            "PauseLB_LevelLabel",
            "PauseLB_RowTab_1", "PauseLB_RowTab_2", "PauseLB_RowTab_3",
            "PauseLB_Name_1", "PauseLB_Name_2", "PauseLB_Name_3",
            "PauseLB_Time_1", "PauseLB_Time_2", "PauseLB_Time_3",
            "PauseLB_Rating_1", "PauseLB_Rating_2", "PauseLB_Rating_3",
            "PauseLB_Circle_1", "PauseLB_Circle_2", "PauseLB_Circle_3",
            "PauseLB_CircleNum_1", "PauseLB_CircleNum_2", "PauseLB_CircleNum_3",
            "PauseLB_Back",
            "PauseLB_Replay", "PauseLB_VsGhost"
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

            if (hoveredButtonName == "InGamePauseButton")
            {
                // Open pause menu after delay
                TogglePauseMenu();
                pendingSceneChange = false;
                hoveredButtonName.clear();
            }
            else if (hoveredButtonName == "PauseResumeButton")
            {
                // Reset button texture before closing
                SetButtonTexture("PauseResumeButton", ButtonState::Normal);
                TogglePauseMenu(); // close menu and unpause
            }
            else if (hoveredButtonName == "PauseRestartButton")
            {
                //std::cout << "[PauseMenu] Restart - fade complete, reloading level" << std::endl;

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

                // load current volumes from Audio Manager
                audioManager = CORE->GetAudioManager();
                if (audioManager)
                {
                    savedMasterVolume = audioManager->GetMasterVolume();
                    savedMusicVolume = audioManager->GetBaseCategoryVolume("music");
                    savedSfxVolume = audioManager->GetBaseCategoryVolume("sfx");
                    std::cout << "[PauseMenu] Opening Settings - Loaded from AudioManager: Master=" << savedMasterVolume
                              << ", Music=" << savedMusicVolume << ", SFX=" << savedSfxVolume << std::endl;
                }

                isPauseSettingsOpen = true;

                // Clear the pending state immediately for settings
                pendingSceneChange = false;
                hoveredButtonName.clear();
            }
            else if (hoveredButtonName == "PauseMainMenuButton")
            {
                //std::cout << "[PauseMenu] Main Menu - fade complete, loading scene" << std::endl;

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
            else if (hoveredButtonName == "PauseLeaderboardButton")
            {
                if (!hasCreatedLeaderboardUI)
                    CreateLeaderboardUI();

                // Default to the current level's tab
                m_lbLevel = 1;
                if (currentScenePath.find("Level_2") != std::string::npos) m_lbLevel = 2;
                else if (currentScenePath.find("Level_3") != std::string::npos) m_lbLevel = 3;

                ScanLBData(m_lbLevel);
                PopulateLB();

                // Update level label text to match current level
                {
                    auto& lbFactory = GameObjectFactory::Instance();
                    GameObject* lbl = lbFactory.FindGameObject("PauseLB_LevelLabel");
                    if (lbl) { auto* tc = lbl->GetComponent<TextComponent>(); if (tc) tc->SetText("Level " + std::to_string(m_lbLevel)); }
                }

                // Reset selection to first rank; reset tab textures to grey
                m_lbSelectedRank = 0;
                {
                    auto& lbFactory = GameObjectFactory::Instance();
                    const char* rowTabNames[3] = { "PauseLB_RowTab_1", "PauseLB_RowTab_2", "PauseLB_RowTab_3" };
                    for (int i = 0; i < 3; ++i)
                    {
                        GameObject* tab = lbFactory.FindGameObject(rowTabNames[i]);
                        if (tab)
                        {
                            auto* rc = tab->GetComponent<RenderComponent>();
                            if (rc) rc->SetTextureID(GraphicsSystem::LoadTexture("../Assets/Leaderboards_tab_grey.png"));
                        }
                    }
                }

                isLeaderboardOpen = true;

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
                audioManager->SetLowPassFilter("bgm", true, 800.0f, 1.0f);
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
            if (auto* ghostBtn = factory.FindGameObject("PauseLeaderboardButton"))
                ghostBtn->SetActive(false);
            if (auto* board = factory.FindGameObject("PauseBoard"))
                board->SetActive(false);
        }
        else
        {
            // restore low pass if returning to pause menu, otherwise remove it
            if (audioManager)
            {
                if (isPauseMenuActive)
                    audioManager->SetLowPassFilter("bgm", true, 800.0f, 1.0f);
                else
                    audioManager->SetLowPassFilter("bgm", false);
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
            if (auto* ghostBtn = factory.FindGameObject("PauseLeaderboardButton"))
                ghostBtn->SetActive(true);
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
            SetButtonTexture("PauseLeaderboardButton", ButtonState::Normal);

            if (auto* resumeBtn = factory.FindGameObject("PauseResumeButton"))   resumeBtn->SetActive(false);
            if (auto* restartBtn = factory.FindGameObject("PauseRestartButton")) restartBtn->SetActive(false);
            if (auto* settingsBtn = factory.FindGameObject("PauseSettingsButton")) settingsBtn->SetActive(false);
            if (auto* mainMenuBtn = factory.FindGameObject("PauseMainMenuButton")) mainMenuBtn->SetActive(false);
            if (auto* howToPlayBtn = factory.FindGameObject("PauseHowToPlayButton")) howToPlayBtn->SetActive(false);
            if (auto* ghostBtn = factory.FindGameObject("PauseLeaderboardButton")) ghostBtn->SetActive(false);
            if (auto* board = factory.FindGameObject("PauseBoard")) board->SetActive(false);
            if (auto* overlay = factory.FindGameObject("PauseOverlay")) overlay->SetActive(false);
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
            if (auto* ghostBtn = factory.FindGameObject("PauseLeaderboardButton")) ghostBtn->SetActive(true);
            if (auto* board = factory.FindGameObject("PauseBoard")) board->SetActive(true);
            if (auto* overlay = factory.FindGameObject("PauseOverlay")) overlay->SetActive(true);
        }
        wasGhostMenuOpenLastFrame = isGhostMenuOpen;
    }

    // Handle leaderboard overlay visibility - ONLY update when state changes
    if (isLeaderboardOpen != wasLeaderboardOpenLastFrame)
    {
        if (isLeaderboardOpen)
        {
            hoveredButtonName.clear();

            SetLeaderboardVisible(true);

            SetButtonTexture("PauseResumeButton", ButtonState::Normal);
            SetButtonTexture("PauseRestartButton", ButtonState::Normal);
            SetButtonTexture("PauseSettingsButton", ButtonState::Normal);
            SetButtonTexture("PauseMainMenuButton", ButtonState::Normal);
            SetButtonTexture("PauseHowToPlayButton", ButtonState::Normal);
            SetButtonTexture("PauseLeaderboardButton", ButtonState::Normal);

            if (auto* r = factory.FindGameObject("PauseResumeButton"))    r->SetActive(false);
            if (auto* r = factory.FindGameObject("PauseRestartButton"))   r->SetActive(false);
            if (auto* r = factory.FindGameObject("PauseSettingsButton"))  r->SetActive(false);
            if (auto* r = factory.FindGameObject("PauseMainMenuButton"))  r->SetActive(false);
            if (auto* r = factory.FindGameObject("PauseHowToPlayButton")) r->SetActive(false);
            if (auto* r = factory.FindGameObject("PauseLeaderboardButton")) r->SetActive(false);
            if (auto* r = factory.FindGameObject("PauseBoard"))           r->SetActive(false);
            if (auto* r = factory.FindGameObject("PauseOverlay"))         r->SetActive(false);
        }
        else
        {
            SetLeaderboardVisible(false);

            if (auto* r = factory.FindGameObject("PauseResumeButton"))    r->SetActive(true);
            if (auto* r = factory.FindGameObject("PauseRestartButton"))   r->SetActive(true);
            if (auto* r = factory.FindGameObject("PauseSettingsButton"))  r->SetActive(true);
            if (auto* r = factory.FindGameObject("PauseMainMenuButton"))  r->SetActive(true);
            if (auto* r = factory.FindGameObject("PauseHowToPlayButton")) r->SetActive(true);
            if (auto* r = factory.FindGameObject("PauseLeaderboardButton")) r->SetActive(true);
            if (auto* r = factory.FindGameObject("PauseBoard"))           r->SetActive(true);
            if (auto* r = factory.FindGameObject("PauseOverlay"))         r->SetActive(true);
        }
        wasLeaderboardOpenLastFrame = isLeaderboardOpen;
    }

    // Handle pause settings screen visibility - ONLY update when state changes
    if (isPauseSettingsOpen != wasSettingsOpenLastFrame)
    {
        if (isPauseSettingsOpen)
        {
            // Clear any hover state from previous menu
            hoveredButtonName.clear();

            // load current volume values from audio manager
            if (audioManager)
            {
                std::cout << "[PauseMenu Init] Loaded volumes - Master: " << savedMasterVolume
                          << ", Music: " << savedMusicVolume << ", SFX: " << savedSfxVolume << std::endl;

                // Set slider values directly from loaded volumes (no inference needed)
                masterSlider.currentVolume = savedMasterVolume;
                musicSlider.currentVolume = savedMusicVolume;
                sfxSlider.currentVolume = savedSfxVolume;

                std::cout << "[PauseMenu Init] Slider values - Master: " << masterSlider.currentVolume
                          << ", Music: " << musicSlider.currentVolume << ", SFX: " << sfxSlider.currentVolume << std::endl;

                // only initialize if not already initialized
                if (masterSlider.originalScaleX <= 0.0f)
                {
                    masterSlider.uvBoundsReady = false;
                    musicSlider.uvBoundsReady = false;
                    sfxSlider.uvBoundsReady = false;

                    // update slider visuals to match saved volumes
                    UpdateSlider("PauseSlider_Bar_Master", masterSlider, savedMasterVolume);
                    UpdateSlider("PauseSlider_Bar_Music", musicSlider, savedMusicVolume);
                    UpdateSlider("PauseSlider_Bar_SFX", sfxSlider, savedSfxVolume);
                }
                else
                {
                    UpdateSlider("PauseSlider_Bar_Master", masterSlider, savedMasterVolume);
                    UpdateSlider("PauseSlider_Bar_Music", musicSlider, savedMusicVolume);
                    UpdateSlider("PauseSlider_Bar_SFX", sfxSlider, savedSfxVolume);
                }

                auto updateKnobPosition = [&factory](const std::string& barName, const std::string& knobName) {
                    std::string barBackName = barName;
                    size_t pos = barBackName.find("_Bar_");
                    if (pos != std::string::npos) 
                    {
                        barBackName.insert(pos + 5, "Back_");
                    }

                    GameObject* sliderBar = factory.FindGameObject(barName);
                    GameObject* barBack = factory.FindGameObject(barBackName);
                    GameObject* knob = factory.FindGameObject(knobName);

                    if (sliderBar && barBack && knob)
                    {
                        auto* sliderTransform = sliderBar->GetComponent<TransformComponent>();
                        auto* knobTransform = knob->GetComponent<TransformComponent>();

                        if (sliderTransform && knobTransform)
                        {
                            float sliderCenterX, sliderCenterY, sliderScaleX, sliderScaleY;
                            sliderTransform->GetPosition(sliderCenterX, sliderCenterY);
                            sliderTransform->GetScale(sliderScaleX, sliderScaleY);

                            float knobX = sliderCenterX + sliderScaleX * 0.5f - 570.0f;
                            knobTransform->SetPosition(knobX, sliderCenterY);
                        }
                    }
                };

                updateKnobPosition("PauseSlider_Bar_Master", "PauseSlider_Knob_Master");
                updateKnobPosition("PauseSlider_Bar_Music", "PauseSlider_Knob_Music");
                updateKnobPosition("PauseSlider_Bar_SFX", "PauseSlider_Knob_SFX");
            }

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
            if (auto* mainMenuBtn = factory.FindGameObject("PauseLeaderboardButton"))
                mainMenuBtn->SetActive(false);
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
            if (auto* mainMenuBtn = factory.FindGameObject("PauseLeaderboardButton"))
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
                //std::cout << "[PauseMenu] Closing settings after delay" << std::endl;
                // reset back button texture to normal before closing
                SetButtonTexture("PauseBackButton", ButtonState::Normal);

                isPauseSettingsOpen = false;
                pendingSettingsClose = false;
                hoveredButtonName.clear();
            }
            return; // Don't process interactions during pending close
        }

        // Check if back button is clicked to close settings
        InputManager* inp = CORE->GetInputManager();
        float renderW = (float)CORE->GetGraphicsSystem()->GetWindowWidth();
        float renderH = (float)CORE->GetGraphicsSystem()->GetWindowHeight();
        float ndcX = static_cast<float>(inp->GetMouseX() / renderW) * 2.0f - 1.0f;
        float ndcY = 1.0f - static_cast<float>(inp->GetMouseY() / renderH) * 2.0f;
        float resScale = std::min(renderW / 1920.0f, renderH / 1080.0f);
        float virtualW = renderW / resScale;
        float virtualH = renderH / resScale;
        auto* _cam = CameraManager::Instance().GetActiveCamera();
        float _zoom = (_cam && _cam->getZoom() != 0.0f) ? _cam->getZoom() : 1.0f;
        vec2 _camPos = CameraManager::Instance().GetCameraPosition();
        double mouseX = _camPos.x + ndcX * (virtualW / (2.0f * _zoom));
        double mouseY = _camPos.y + ndcY * (virtualH / (2.0f * _zoom));
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

        // Keyboard navigation for settings sub-menu
        // Rows: 0=Master, 1=Music, 2=SFX, 3=Back
        static const char* kSettingsButtons[] = {
            "PauseMasterMuteButton", "PauseMusicMuteButton", "PauseSFXMuteButton", "PauseBackButton"
        };
        static const int   kSettingsCount  = 4;
        static int         kSettingsIdx    = 0;
        static bool        kSettingsActive = false;
        // Key-repeat state for LEFT/RIGHT slider adjustment
        static float       kSliderRepeatTimer  = 0.0f;
        static const float kSliderRepeatDelay  = 0.3f;  // seconds before repeat starts
        static const float kSliderRepeatRate   = 0.08f; // seconds between repeat steps
        bool settingsSpacePressed = false;
        {
            InputManager* inp = CORE->GetInputManager();
            if (hoveredButtonName.empty()) { kSettingsActive = false; kSettingsIdx = 0; } // reset on fresh open
            if (inp)
            {
                if (inp->IsKeyPressed(GLFW_KEY_DOWN)) { kSettingsIdx = (kSettingsIdx + 1) % kSettingsCount; kSettingsActive = true; }
                if (inp->IsKeyPressed(GLFW_KEY_UP))   { kSettingsIdx = ((kSettingsIdx - 1) + kSettingsCount) % kSettingsCount; kSettingsActive = true; }
                if (inp->IsGamepadConnected())
                {
                    if (inp->IsGamepadButtonPressed(GLFW_GAMEPAD_BUTTON_DPAD_DOWN)) { kSettingsIdx = (kSettingsIdx + 1) % kSettingsCount;                         kSettingsActive = true; }
                    if (inp->IsGamepadButtonPressed(GLFW_GAMEPAD_BUTTON_DPAD_UP))   { kSettingsIdx = ((kSettingsIdx - 1) + kSettingsCount) % kSettingsCount;     kSettingsActive = true; }
                }
                if (isClicked && !currentHover.empty()) kSettingsActive = false;
                if (kSettingsActive) currentHover = kSettingsButtons[kSettingsIdx];
                settingsSpacePressed = kSettingsActive && (inp->IsKeyPressed(GLFW_KEY_SPACE) ||
                    (inp->IsGamepadConnected() && inp->IsGamepadButtonPressed(GLFW_GAMEPAD_BUTTON_A)));

                // LEFT/RIGHT: adjust slider on the current row (works without needing UP/DOWN first)
                bool leftHeld  = inp->IsKeyHeld(GLFW_KEY_LEFT)  || inp->IsKeyPressed(GLFW_KEY_LEFT)
                               || (inp->IsGamepadConnected() && inp->IsGamepadButtonHeld(GLFW_GAMEPAD_BUTTON_DPAD_LEFT));
                bool rightHeld = inp->IsKeyHeld(GLFW_KEY_RIGHT) || inp->IsKeyPressed(GLFW_KEY_RIGHT)
                               || (inp->IsGamepadConnected() && inp->IsGamepadButtonHeld(GLFW_GAMEPAD_BUTTON_DPAD_RIGHT));
                bool leftJust  = inp->IsKeyPressed(GLFW_KEY_LEFT)
                               || (inp->IsGamepadConnected() && inp->IsGamepadButtonPressed(GLFW_GAMEPAD_BUTTON_DPAD_LEFT));
                bool rightJust = inp->IsKeyPressed(GLFW_KEY_RIGHT)
                               || (inp->IsGamepadConnected() && inp->IsGamepadButtonPressed(GLFW_GAMEPAD_BUTTON_DPAD_RIGHT));

                // Activate keyboard mode on any LEFT/RIGHT press so slider row defaults to 0 (Master)
                if ((leftJust || rightJust) && !kSettingsActive) kSettingsActive = true;

                int sliderRow = kSettingsIdx; // row the user has selected
                if (kSettingsActive && sliderRow <= 2)
                {
                    // Determine if we should step this frame (initial press or repeat)
                    bool shouldStep = false;
                    if (leftJust || rightJust)
                    {
                        shouldStep = true;
                        kSliderRepeatTimer = 0.0f;
                    }
                    else if (leftHeld || rightHeld)
                    {
                        kSliderRepeatTimer += dt;
                        if (kSliderRepeatTimer >= kSliderRepeatDelay)
                        {
                            // Use modulo to fire at repeat rate
                            float excess = kSliderRepeatTimer - kSliderRepeatDelay;
                            float prevExcess = excess - dt;
                            if (prevExcess < 0.0f || static_cast<int>(excess / kSliderRepeatRate) > static_cast<int>(prevExcess / kSliderRepeatRate))
                                shouldStep = true;
                        }
                    }
                    else
                    {
                        kSliderRepeatTimer = 0.0f;
                    }

                    if (shouldStep)
                    {
                        const float kStep = 0.05f;
                        float delta = rightHeld ? kStep : -kStep;

                        SliderState* sliderState = nullptr;
                        std::string barName, knobName, category;

                        if (sliderRow == 0)
                        {
                            sliderState = &masterSlider;
                            barName  = "PauseSlider_Bar_Master";
                            knobName = "PauseSlider_Knob_Master";
                            category = "master";
                        }
                        else if (sliderRow == 1)
                        {
                            sliderState = &musicSlider;
                            barName  = "PauseSlider_Bar_Music";
                            knobName = "PauseSlider_Knob_Music";
                            category = "music";
                        }
                        else
                        {
                            sliderState = &sfxSlider;
                            barName  = "PauseSlider_Bar_SFX";
                            knobName = "PauseSlider_Knob_SFX";
                            category = "sfx";
                        }

                        float newVol = std::clamp(sliderState->currentVolume + delta, 0.0f, 1.0f);
                        sliderState->currentVolume = newVol;
                        UpdateSlider(barName, *sliderState, newVol);

                        // Reposition knob
                        auto& sliderFactory = GameObjectFactory::Instance();
                        GameObject* sliderBar = sliderFactory.FindGameObject(barName);
                        GameObject* knob      = sliderFactory.FindGameObject(knobName);
                        if (sliderBar && knob)
                        {
                            auto* st = sliderBar->GetComponent<TransformComponent>();
                            auto* kt = knob->GetComponent<TransformComponent>();
                            if (st && kt)
                            {
                                float cx, cy, sx, sy;
                                st->GetPosition(cx, cy);
                                st->GetScale(sx, sy);
                                kt->SetPosition(cx + sx * 0.5f - 570.0f, cy);
                            }
                        }

                        // Apply to audio
                        AudioManager* sliderAudio = CORE->GetAudioManager();
                        if (sliderAudio)
                        {
                            if (category == "master")
                            {
                                savedMasterVolume = newVol;
                                sliderAudio->SetMasterVolume(savedMasterVolume);
                                sliderAudio->SetCategoryVolume("music", savedMusicVolume * savedMasterVolume);
                                sliderAudio->SetCategoryVolume("sfx",   savedSfxVolume   * savedMasterVolume);
                            }
                            else if (category == "music")
                            {
                                savedMusicVolume = newVol;
                                sliderAudio->SetBaseCategoryVolume("music", newVol);
                                sliderAudio->SetCategoryVolume("music", newVol * savedMasterVolume);
                            }
                            else
                            {
                                savedSfxVolume = newVol;
                                sliderAudio->SetBaseCategoryVolume("sfx", newVol);
                                sliderAudio->SetCategoryVolume("sfx", newVol * savedMasterVolume);
                            }
                        }
                    }
                }
                else
                {
                    kSliderRepeatTimer = 0.0f;
                }
            }
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
        if ((isClicked || settingsSpacePressed) && !hoveredButtonName.empty())
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
                //std::cout << "[PauseMenu] Back button clicked - starting close delay" << std::endl;
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

        // handle slider input — clear keyboard selection when mouse starts dragging
        bool isMouseDown = glfwGetMouseButton(glfwGetCurrentContext(), GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS;
        if (isMouseDown && isDraggingSlider)
        {
            hoveredButtonName.clear();
        }
        HandleSliderInput(mouseX, mouseY, isMouseDown);

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
                //std::cout << "[PauseMenu] Closing how to play after delay" << std::endl;

                // Reset back button texture to normal before closing
                SetButtonTexture("HowToPlayBackButton", ButtonState::Normal);

                isHowToPlayOpen = false;
                pendingHowToPlayClose = false;
                hoveredButtonName.clear();
            }
            return;
        }

        // Check for ESC key or back button click to close
        InputManager* inp = CORE->GetInputManager();
        float renderW = (float)CORE->GetGraphicsSystem()->GetWindowWidth();
        float renderH = (float)CORE->GetGraphicsSystem()->GetWindowHeight();
        float ndcX = static_cast<float>(inp->GetMouseX() / renderW) * 2.0f - 1.0f;
        float ndcY = 1.0f - static_cast<float>(inp->GetMouseY() / renderH) * 2.0f;
        float resScale = std::min(renderW / 1920.0f, renderH / 1080.0f);
        float virtualW = renderW / resScale;
        float virtualH = renderH / resScale;
        auto* _cam = CameraManager::Instance().GetActiveCamera();
        float _zoom = (_cam && _cam->getZoom() != 0.0f) ? _cam->getZoom() : 1.0f;
        vec2 _camPos = CameraManager::Instance().GetCameraPosition();
        double mouseX = _camPos.x + ndcX * (virtualW / (2.0f * _zoom));
        double mouseY = _camPos.y + ndcY * (virtualH / (2.0f * _zoom));
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

        // Keyboard: SPACE confirms the back button
        bool htpSpacePressed = false;
        {
            InputManager* inp = CORE->GetInputManager();
            if (inp)
            {
                bool gpAny = inp->IsGamepadConnected() && (
                    inp->IsGamepadButtonPressed(GLFW_GAMEPAD_BUTTON_DPAD_DOWN) ||
                    inp->IsGamepadButtonPressed(GLFW_GAMEPAD_BUTTON_DPAD_UP)   ||
                    inp->IsGamepadButtonPressed(GLFW_GAMEPAD_BUTTON_A));
                if (inp->IsKeyPressed(GLFW_KEY_DOWN) || inp->IsKeyPressed(GLFW_KEY_UP) || inp->IsKeyPressed(GLFW_KEY_SPACE) || gpAny)
                    currentHover = "HowToPlayBackButton";
                htpSpacePressed = inp->IsKeyPressed(GLFW_KEY_SPACE) ||
                    (inp->IsGamepadConnected() && inp->IsGamepadButtonPressed(GLFW_GAMEPAD_BUTTON_A));
            }
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
        if ((isClicked || htpSpacePressed) && hoveredButtonName == "HowToPlayBackButton")
        {
            SetButtonTexture("HowToPlayBackButton", ButtonState::Pressed);
            audioManager = CORE->GetAudioManager();
            if (audioManager)
            {
                audioManager->TriggerEvent("UI_Click");
            }

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

        InputManager* inp = CORE->GetInputManager();
        float renderW = (float)CORE->GetGraphicsSystem()->GetWindowWidth();
        float renderH = (float)CORE->GetGraphicsSystem()->GetWindowHeight();
        float ndcX = static_cast<float>(inp->GetMouseX() / renderW) * 2.0f - 1.0f;
        float ndcY = 1.0f - static_cast<float>(inp->GetMouseY() / renderH) * 2.0f;
        float resScale = std::min(renderW / 1920.0f, renderH / 1080.0f);
        float virtualW = renderW / resScale;
        float virtualH = renderH / resScale;
        auto* _cam = CameraManager::Instance().GetActiveCamera();
        float _zoom = (_cam && _cam->getZoom() != 0.0f) ? _cam->getZoom() : 1.0f;
        vec2 _camPos = CameraManager::Instance().GetCameraPosition();
        double mouseX = _camPos.x + ndcX * (virtualW / (2.0f * _zoom));
        double mouseY = _camPos.y + ndcY * (virtualH / (2.0f * _zoom));

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

        // Keyboard navigation for ghost sub-menu
        static const char* kGhostButtons[] = {
            "PauseGhost_Toggle", "PauseGhost_Slot0", "PauseGhost_Slot1", "PauseGhost_Slot2", "PauseGhost_Back"
        };
        static const int kGhostCount = 5;
        static int  kGhostIdx    = 0;
        static bool kGhostActive = false;
        bool ghostSpacePressed = false;
        {
            InputManager* inp = CORE->GetInputManager();
            if (hoveredButtonName.empty()) kGhostActive = false; // reset on fresh open
            if (inp)
            {
                if (inp->IsKeyPressed(GLFW_KEY_DOWN)) { kGhostIdx = (kGhostIdx + 1) % kGhostCount;                     kGhostActive = true; }
                if (inp->IsKeyPressed(GLFW_KEY_UP))   { kGhostIdx = ((kGhostIdx - 1) + kGhostCount) % kGhostCount;     kGhostActive = true; }
                if (inp->IsGamepadConnected())
                {
                    if (inp->IsGamepadButtonPressed(GLFW_GAMEPAD_BUTTON_DPAD_DOWN)) { kGhostIdx = (kGhostIdx + 1) % kGhostCount;                     kGhostActive = true; }
                    if (inp->IsGamepadButtonPressed(GLFW_GAMEPAD_BUTTON_DPAD_UP))   { kGhostIdx = ((kGhostIdx - 1) + kGhostCount) % kGhostCount;     kGhostActive = true; }
                }
                if (isClicked && !currentHover.empty()) kGhostActive = false;
                if (kGhostActive) currentHover = kGhostButtons[kGhostIdx];
                ghostSpacePressed = kGhostActive && (inp->IsKeyPressed(GLFW_KEY_SPACE) ||
                    (inp->IsGamepadConnected() && inp->IsGamepadButtonPressed(GLFW_GAMEPAD_BUTTON_A)));
            }
        }

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
        if ((isClicked || ghostSpacePressed) && !hoveredButtonName.empty())
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

    // Handle leaderboard overlay interactions
    if (isLeaderboardOpen)
    {
        if (pendingLeaderboardClose)
        {
            leaderboardCloseTimer -= dt;
            if (leaderboardCloseTimer <= 0.0f)
            {
                isLeaderboardOpen = false;
                pendingLeaderboardClose = false;
                hoveredButtonName.clear();
            }
            return;
        }

        InputManager* inp = CORE->GetInputManager();
        float renderW = (float)CORE->GetGraphicsSystem()->GetWindowWidth();
        float renderH = (float)CORE->GetGraphicsSystem()->GetWindowHeight();
        float ndcX = static_cast<float>(inp->GetMouseX() / renderW) * 2.0f - 1.0f;
        float ndcY = 1.0f - static_cast<float>(inp->GetMouseY() / renderH) * 2.0f;
        float resScale = std::min(renderW / 1920.0f, renderH / 1080.0f);
        float virtualW = renderW / resScale;
        float virtualH = renderH / resScale;
        auto* _cam = CameraManager::Instance().GetActiveCamera();
        float _zoom = (_cam && _cam->getZoom() != 0.0f) ? _cam->getZoom() : 1.0f;
        vec2 _camPos = CameraManager::Instance().GetCameraPosition();
        double mouseX = _camPos.x + ndcX * (virtualW / (2.0f * _zoom));
        double mouseY = _camPos.y + ndcY * (virtualH / (2.0f * _zoom));
        bool isClicked = InputManager::IsMouseLeftClicked();

        // ESC closes the leaderboard and returns to the pause main menu
        static bool escWasPressedLeaderboard = false;
        if (glfwGetKey(glfwGetCurrentContext(), GLFW_KEY_ESCAPE) == GLFW_PRESS)
        {
            if (!escWasPressedLeaderboard)
            {
                pendingLeaderboardClose = true;
                leaderboardCloseTimer = 0.15f;
                escWasPressedLeaderboard = true;
            }
        }
        else
        {
            escWasPressedLeaderboard = false;
        }

        std::string currentHover = "";
        if (IsMouseOver(factory.FindGameObject("PauseLB_Back"), mouseX, mouseY))
            currentHover = "PauseLB_Back";
        else if (IsMouseOver(factory.FindGameObject("PauseLB_Replay"), mouseX, mouseY))
            currentHover = "PauseLB_Replay";
        else if (IsMouseOver(factory.FindGameObject("PauseLB_VsGhost"), mouseX, mouseY))
            currentHover = "PauseLB_VsGhost";

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

        // Row tab click — selects rank (highlight yellow)
        const char* rowTabNames[3] = { "PauseLB_RowTab_1", "PauseLB_RowTab_2", "PauseLB_RowTab_3" };
        if (isClicked)
        {
            for (int i = 0; i < 3; ++i)
            {
                GameObject* rt = factory.FindGameObject(rowTabNames[i]);
                if (rt && IsMouseOver(rt, mouseX, mouseY) && i < static_cast<int>(m_lbData.size()))
                {
                    m_lbSelectedRank = i;
                    AudioManager* localAudio = CORE->GetAudioManager();
                    if (localAudio) localAudio->TriggerEvent("UI_Click");
                    // Update tab textures: yellow for selected, grey for others
                    for (int j = 0; j < 3; ++j)
                    {
                        GameObject* tab = factory.FindGameObject(rowTabNames[j]);
                        if (tab)
                        {
                            auto* rc = tab->GetComponent<RenderComponent>();
                            if (rc)
                            {
                                const char* tex = (j == m_lbSelectedRank)
                                    ? "../Assets/Leaderboards_tab_yellow.png"
                                    : "../Assets/Leaderboards_tab_grey.png";
                                rc->SetTextureID(GraphicsSystem::LoadTexture(tex));
                            }
                        }
                    }
                    break;
                }
            }
        }

        if (isClicked && hoveredButtonName == "PauseLB_Back")
        {
            SetButtonTexture("PauseLB_Back", ButtonState::Pressed);
            AudioManager* localAudio = CORE->GetAudioManager();
            if (localAudio) localAudio->TriggerEvent("UI_Click");
            pendingLeaderboardClose = true;
            leaderboardCloseTimer = 0.15f;
        }
        else if (isClicked && hoveredButtonName == "PauseLB_Replay")
        {
            SetButtonTexture("PauseLB_Replay", ButtonState::Pressed);
            AudioManager* localAudio = CORE->GetAudioManager();
            if (localAudio) localAudio->TriggerEvent("UI_Click");

            // Load selected recording and restart scene in Watch Replay mode
            if (m_lbSelectedRank < static_cast<int>(m_lbData.size()))
            {
                GhostSystem* ghost = CORE ? CORE->GetGhostSystem() : nullptr;
                if (ghost) ghost->RequestReplayOnLoad(m_lbData[m_lbSelectedRank].name);
            }
            isLeaderboardOpen = false;
            hoveredButtonName.clear();
            DestroyLeaderboardUI();
            DestroyGhostMenuUI();
            DestroyPauseSettingsUI();
            DestroyHowToPlayUI();
            DestroyPauseMenuUI();
            if (isPauseMenuActive)
            {
                isPauseMenuActive = false;
                CORE->SetGamePaused(false, false);
            }
            if (!currentScenePath.empty())
                CORE->RequestSceneLoad(currentScenePath, true);
        }
        else if (isClicked && hoveredButtonName == "PauseLB_VsGhost")
        {
            SetButtonTexture("PauseLB_VsGhost", ButtonState::Pressed);
            AudioManager* localAudio = CORE->GetAudioManager();
            if (localAudio) localAudio->TriggerEvent("UI_Click");

            GhostSystem* ghost = CORE ? CORE->GetGhostSystem() : nullptr;
            GameObject* existingGhost = factory.FindGameObject("GhostHero");

            if (existingGhost)
            {
                // Ghost is already in the scene — remove it and its VFX, then resume
                if (ghost)
                {
                    ghost->StopPlayback();
                    ghost->ResetGhostToggle();
                }
                factory.QueueDestroyGameObject(existingGhost->GetID());
                GameObject* existingVfx = factory.FindGameObject("GhostHero_vfx");
                if (existingVfx) factory.QueueDestroyGameObject(existingVfx->GetID());
            }
            else if (ghost && m_lbSelectedRank < static_cast<int>(m_lbData.size()))
            {
                // No ghost in scene — create one from the selected leaderboard recording
                ghost->Configure("Hero", "GhostHero", "Hero_vfx", "GhostHero_vfx");

                // Background layers vary by level
                if (m_lbLevel == 1)
                    ghost->ConfigureBackground("Parallax_BG_2", "MIdlayer_BackGround2", "Frontlayer_BackGround2");
                else if (m_lbLevel == 2)
                    ghost->ConfigureBackground("Parallax_BG", "Pillar_BackGround");

                ghost->LoadRecording(m_lbData[m_lbSelectedRank].name);
                ghost->ResetGhostToggle();      // ensure flag is clean before toggling ON
                ghost->ToggleGhostVisible();    // clones Hero → GhostHero, parks beside SafeZone (semi-transparent)
                ghost->SetVsGhostMode(true);    // prevent WinLoseLogic from overwriting this recording on restart

                // If the player is already past the SafeZone, kick off playback immediately.
                // Otherwise WinLoseLogic::CheckSafeZoneExit() will start it on exit.
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
            }

            // Close all UI and resume — no scene reload needed
            isLeaderboardOpen = false;
            hoveredButtonName.clear();
            DestroyLeaderboardUI();
            DestroyGhostMenuUI();
            DestroyPauseSettingsUI();
            DestroyHowToPlayUI();
            DestroyPauseMenuUI();
            if (isPauseMenuActive)
            {
                isPauseMenuActive = false;
                CORE->SetGamePaused(false, false);
            }
        }

        return; // Don't process normal pause menu interactions
    }

    // Normal pause menu button handling
    InputManager* inp = CORE->GetInputManager();
    float renderW = (float)CORE->GetGraphicsSystem()->GetWindowWidth();
    float renderH = (float)CORE->GetGraphicsSystem()->GetWindowHeight();
    float ndcX = static_cast<float>(inp->GetMouseX() / renderW) * 2.0f - 1.0f;
    float ndcY = 1.0f - static_cast<float>(inp->GetMouseY() / renderH) * 2.0f;
    float resScale = std::min(renderW / 1920.0f, renderH / 1080.0f);
    float virtualW = renderW / resScale;
    float virtualH = renderH / resScale;
    auto* _cam = CameraManager::Instance().GetActiveCamera();
    float _zoom = (_cam && _cam->getZoom() != 0.0f) ? _cam->getZoom() : 1.0f;
    vec2 _camPos = CameraManager::Instance().GetCameraPosition();
    double mouseX = _camPos.x + ndcX * (virtualW / (2.0f * _zoom));
    double mouseY = _camPos.y + ndcY * (virtualH / (2.0f * _zoom));
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
        else if (IsMouseOver(factory.FindGameObject("PauseLeaderboardButton"), mouseX, mouseY))
        {
            currentHover = "PauseLeaderboardButton";
        }
    }

    // Keyboard selection: UP/DOWN to move between buttons, SPACE to confirm
    static const char* kPauseButtons[] = {
        "PauseResumeButton", "PauseSettingsButton", "PauseRestartButton",
        "PauseMainMenuButton", "PauseHowToPlayButton", "PauseLeaderboardButton"
    };
    static const int kPauseCount = 6;
    static int  kPauseIdx    = 0;
    static bool kPauseActive = false;
    bool spacePressed = false;
    {
        InputManager* inp = CORE->GetInputManager();
        if (hoveredButtonName.empty()) kPauseActive = false; // reset on fresh open
        if (inp && !pendingSceneChange)
        {
            if (inp->IsKeyPressed(GLFW_KEY_DOWN)) { kPauseIdx = (kPauseIdx + 1) % kPauseCount;                     kPauseActive = true; }
            if (inp->IsKeyPressed(GLFW_KEY_UP))   { kPauseIdx = ((kPauseIdx - 1) + kPauseCount) % kPauseCount;     kPauseActive = true; }
            if (inp->IsGamepadConnected())
            {
                if (inp->IsGamepadButtonPressed(GLFW_GAMEPAD_BUTTON_DPAD_DOWN)) { kPauseIdx = (kPauseIdx + 1) % kPauseCount;                     kPauseActive = true; }
                if (inp->IsGamepadButtonPressed(GLFW_GAMEPAD_BUTTON_DPAD_UP))   { kPauseIdx = ((kPauseIdx - 1) + kPauseCount) % kPauseCount;     kPauseActive = true; }
            }
            if (isClicked && !currentHover.empty()) kPauseActive = false;
            if (kPauseActive) currentHover = kPauseButtons[kPauseIdx];
            spacePressed = kPauseActive && (inp->IsKeyPressed(GLFW_KEY_SPACE) ||
                (inp->IsGamepadConnected() && inp->IsGamepadButtonPressed(GLFW_GAMEPAD_BUTTON_A)));
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
    if ((isClicked || spacePressed) && !hoveredButtonName.empty())
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
    overlayTransform->posY = -0.0f;
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
    boardTransform->scaleX = 2000.0f * uiScale;
    boardTransform->scaleY = 1100.0f * uiScale;
    auto* boardRender = board->AddComponent<RenderComponent>();
    boardRender->SetTextureID(GraphicsSystem::LoadTexture("../Assets/Sprites/Pause_Menu_New.png"));
    boardRender->SetColor(vec4(1.0f, 1.0f, 1.0f, 1.0f));
    boardRender->SetLayer(601); // above overlay

    // back button (resume) — top right
    GameObject* pausedBtn = factory.CreateGameObject("PauseResumeButton");
    auto* pausedTransform = pausedBtn->AddComponent<TransformComponent>();
    pausedTransform->posX = 650.0f * uiScale;
    pausedTransform->posY = 270.0f * uiScale;
    pausedTransform->scaleX = 200.0f * uiScale;
    pausedTransform->scaleY = 175.0f * uiScale;
    auto* pausedRender = pausedBtn->AddComponent<RenderComponent>();
    pausedRender->SetTextureID(GraphicsSystem::LoadTexture("../Assets/Sprites/Icons/back_button.png"));
    pausedRender->SetColor(vec4(1.0f, 1.0f, 1.0f, 1.0f));
    pausedRender->SetLayer(602);

    // restart button — left column, row 1
    GameObject* restartBtn = factory.CreateGameObject("PauseRestartButton");
    auto* restartTransform = restartBtn->AddComponent<TransformComponent>();
    restartTransform->posX = -200.0f * uiScale;
    restartTransform->posY = 45.0f * uiScale;
    restartTransform->scaleX = 380.0f * uiScale;
    restartTransform->scaleY = 100.0f * uiScale;
    auto* restartRender = restartBtn->AddComponent<RenderComponent>();
    restartRender->SetTextureID(GraphicsSystem::LoadTexture("../Assets//Sprites/Icons/blankbtnL.png"));
    restartRender->SetColor(vec4(1.0f, 1.0f, 1.0f, 1.0f));
    restartRender->SetLayer(602);
    restartBtn->AddComponent<TextComponent>(
        "../Assets/FrederickatheGreat-Regular.ttf", static_cast<int>(31 * uiScale), "Restart Level", vec4(0.0f, 0.0f, 0.0f, 1.0f), false);

    // how to play button — right column, row 1
    GameObject* howToPlayBtn = factory.CreateGameObject("PauseHowToPlayButton");
    auto* howToPlayTransform = howToPlayBtn->AddComponent<TransformComponent>();
    howToPlayTransform->posX = 200.0f * uiScale;
    howToPlayTransform->posY = 45.0f * uiScale;
    howToPlayTransform->scaleX = 380.0f * uiScale;
    howToPlayTransform->scaleY = 100.0f * uiScale;
    auto* howToPlayRender = howToPlayBtn->AddComponent<RenderComponent>();
    howToPlayRender->SetTextureID(GraphicsSystem::LoadTexture("../Assets/Sprites/Icons/blankbtnL.png"));
    howToPlayRender->SetColor(vec4(1.0f, 1.0f, 1.0f, 1.0f));
    howToPlayRender->SetLayer(602);
    howToPlayBtn->AddComponent<TextComponent>(
        "../Assets/FrederickatheGreat-Regular.ttf", static_cast<int>(31 * uiScale), "How To Play", vec4(0.0f, 0.0f, 0.0f, 1.0f), false);

    // settings button — left column, row 2
    GameObject* settingsBtn = factory.CreateGameObject("PauseSettingsButton");
    auto* settingsTransform = settingsBtn->AddComponent<TransformComponent>();
    settingsTransform->posX = -200.0f * uiScale;
    settingsTransform->posY = -100.0f * uiScale;
    settingsTransform->scaleX = 380.0f * uiScale;
    settingsTransform->scaleY = 100.0f * uiScale;
    auto* settingsRender = settingsBtn->AddComponent<RenderComponent>();
    settingsRender->SetTextureID(GraphicsSystem::LoadTexture("../Assets/Sprites/Icons/blankbtnL.png"));
    settingsRender->SetColor(vec4(1.0f, 1.0f, 1.0f, 1.0f));
    settingsRender->SetLayer(602);
    settingsBtn->AddComponent<TextComponent>(
        "../Assets/FrederickatheGreat-Regular.ttf", static_cast<int>(31 * uiScale), "Settings", vec4(0.0f, 0.0f, 0.0f, 1.0f), false);

    // leaderboard button — right column, row 2
    GameObject* leaderboardBtn = factory.CreateGameObject("PauseLeaderboardButton");
    auto* leaderboardTransform = leaderboardBtn->AddComponent<TransformComponent>();
    leaderboardTransform->posX = 200.0f * uiScale;
    leaderboardTransform->posY = -100.0f * uiScale;
    leaderboardTransform->scaleX = 380.0f * uiScale;
    leaderboardTransform->scaleY = 100.0f * uiScale;
    auto* leaderboardRender = leaderboardBtn->AddComponent<RenderComponent>();
    leaderboardRender->SetTextureID(GraphicsSystem::LoadTexture("../Assets/Sprites/Icons/blankbtnL.png"));
    leaderboardRender->SetColor(vec4(1.0f, 1.0f, 1.0f, 1.0f));
    leaderboardRender->SetLayer(602);
    leaderboardBtn->AddComponent<TextComponent>(
        "../Assets/FrederickatheGreat-Regular.ttf", static_cast<int>(31 * uiScale), "Leaderboards", vec4(0.0f, 0.0f, 0.0f, 1.0f), false);

    // main menu button — row 3, centered
    GameObject* mainMenuBtn = factory.CreateGameObject("PauseMainMenuButton");
    auto* mainMenuTransform = mainMenuBtn->AddComponent<TransformComponent>();
    mainMenuTransform->posX = 0.0f;
    mainMenuTransform->posY = -250.0f * uiScale;
    mainMenuTransform->scaleX = 380.0f * uiScale;
    mainMenuTransform->scaleY = 100.0f * uiScale;
    auto* mainMenuRender = mainMenuBtn->AddComponent<RenderComponent>();
    mainMenuRender->SetTextureID(GraphicsSystem::LoadTexture("../Assets/Sprites/Icons/blankbtnL.png"));
    mainMenuRender->SetColor(vec4(1.0f, 1.0f, 1.0f, 1.0f));
    mainMenuRender->SetLayer(602);
    mainMenuBtn->AddComponent<TextComponent>(
        "../Assets/FrederickatheGreat-Regular.ttf", static_cast<int>(31 * uiScale), "Main Menu", vec4(0.0f, 0.0f, 0.0f, 1.0f), false);

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
    settingsTransform->scaleX = 192.0f * uiScale;
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

    // get camera position for initial positioning
    vec2 initialCamPos = CameraManager::Instance().GetCameraPosition();

    // Master Slider - Bar
    GameObject* masterBar = factory.CreateGameObject("PauseSlider_Bar_Master");
    auto* masterBarTransform = masterBar->AddComponent<TransformComponent>();
    masterBarTransform->posX = initialCamPos.x + 200.0f * uiScale;
    masterBarTransform->posY = initialCamPos.y + 50.0f * uiScale;
    masterBarTransform->scaleX = 670.0f * uiScale;
    masterBarTransform->scaleY = 110.0f * uiScale;
    auto* masterBarRender = masterBar->AddComponent<RenderComponent>();
    masterBarRender->SetTextureID(GraphicsSystem::LoadTexture("../Assets/Sprites/Icons/Slider_Bar.png"));
    masterBarRender->SetColor(vec4(1.0f, 1.0f, 1.0f, 1.0f));
    masterBarRender->SetLayer(602);
    masterBarRender->SetUVScale(vec2(1.0f, 1.0f));
    masterBarRender->SetUVOffset(vec2(0.0f, 0.0f));

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
    musicBarTransform->posX = initialCamPos.x + 200.0f * uiScale;
    musicBarTransform->posY = initialCamPos.y + -75.0f * uiScale;
    musicBarTransform->scaleX = 670.0f * uiScale;
    musicBarTransform->scaleY = 110.0f * uiScale;
    auto* musicBarRender = musicBar->AddComponent<RenderComponent>();
    musicBarRender->SetTextureID(GraphicsSystem::LoadTexture("../Assets/Sprites/Icons/Slider_Bar.png"));
    musicBarRender->SetColor(vec4(1.0f, 1.0f, 1.0f, 1.0f));
    musicBarRender->SetLayer(602);
    musicBarRender->SetUVScale(vec2(1.0f, 1.0f));
    musicBarRender->SetUVOffset(vec2(0.0f, 0.0f));

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
    sfxBarTransform->posX = initialCamPos.x + 200.0f * uiScale;
    sfxBarTransform->posY = initialCamPos.y + -200.0f * uiScale;
    sfxBarTransform->scaleX = 670.0f * uiScale;
    sfxBarTransform->scaleY = 110.0f * uiScale;
    auto* sfxBarRender = sfxBar->AddComponent<RenderComponent>();
    sfxBarRender->SetTextureID(GraphicsSystem::LoadTexture("../Assets/Sprites/Icons/Slider_Bar.png"));
    sfxBarRender->SetColor(vec4(1.0f, 1.0f, 1.0f, 1.0f));
    sfxBarRender->SetLayer(602);
    sfxBarRender->SetUVScale(vec2(1.0f, 1.0f));
    sfxBarRender->SetUVOffset(vec2(0.0f, 0.0f));

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
    backBtnTransform->posY = 270.0f * uiScale;
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
        "PauseLeaderboardButton"
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
    //std::cout << "[PauseMenu] Settings UI destroyed" << std::endl;
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
        "PauseLeaderboardButton"
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
                //std::cout << "[PauseMenu] Current scene: " << currentScenePath << ", isMainMenu: " << isMainMenu << std::endl;
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

    // create leaderboard UI on first toggle to avoid delays later
    if (!hasCreatedLeaderboardUI)
    {
        CreateLeaderboardUI();
    }

    isPauseMenuActive = !isPauseMenuActive;
    SetPauseMenuVisible(isPauseMenuActive);

    // Get audio manager for low pass filter and sound effects
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
        isLeaderboardOpen = false;
        wasLeaderboardOpenLastFrame = false;

        // Reset all button textures when closing pause menu
        SetButtonTexture("PauseResumeButton", ButtonState::Normal);
        SetButtonTexture("PauseRestartButton", ButtonState::Normal);
        SetButtonTexture("PauseSettingsButton", ButtonState::Normal);
        SetButtonTexture("PauseMainMenuButton", ButtonState::Normal);
        SetButtonTexture("PauseHowToPlayButton", ButtonState::Normal);
        SetButtonTexture("PauseLeaderboardButton", ButtonState::Normal);

        // Play unpause sound effect before removing low pass filter
        if (audioManager)
        {
            audioManager->PlaySound("game_unpause");
        }

        // remove low pass filter when pause menu closes
        if (audioManager)
        {
            // Remove low pass filter from BGM sources
            audioManager->SetLowPassFilter("bgm", false);
            //audioManager->SetLowPassFilter("MainMenuBGM", false);
        }
    }
    else
    {
        // Play pause sound effect before applying low pass filter
        if (audioManager)
        {
            audioManager->PlaySound("game_pause");
        }

        // apply low pass filter when pause menu opens
        if (audioManager)
        {
            audioManager->SetLowPassFilter("bgm", true, 800.0f, 1.0f);
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

    if (buttonName == "InGamePauseButton")
    {
        // In-game pause button: playing_button (normal), Playing button_light (hover), Playing button_dark (pressed)
        switch (state)
        {
            case ButtonState::Normal:
                texturePath = "../Assets/Sprites/Icons/playing_button.png";
                break;
            case ButtonState::Hover:
                texturePath = "../Assets/Sprites/Icons/Playing button_light.png";
                break;
            case ButtonState::Pressed:
                texturePath = "../Assets/Sprites/Icons/Playing_button_dark.png";
                break;
        }
    }
    else if (buttonName == "PauseResumeButton")
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
    else if (buttonName == "PauseLeaderboardButton" || buttonName == "PauseGhost_Toggle" ||
             buttonName == "PauseGhost_Slot0" || buttonName == "PauseGhost_Slot1" ||
             buttonName == "PauseGhost_Slot2")
    {
        // Ghost / leaderboard buttons: blank/light/dark
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
    else if (buttonName == "PauseLB_Back" || buttonName == "PauseGhost_Back")
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

    /*std::cout << "[SetButtonTexture] Button: " << buttonName << " | State: "
              << (state == ButtonState::Normal ? "Normal" : state == ButtonState::Hover ? "Hover" : "Pressed")
              << " | Texture: " << texturePath << std::endl;*/

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
        audioManager->SetLowPassFilter("bgm", false);
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
        audioManager->SetLowPassFilter("bgm", false);
        //std::cout << "[PauseMenu] Low pass filter disabled for How To Play" << std::endl;
    }

    isHowToPlayOpen = false;
    wasHowToPlayOpenLastFrame = false; // Reset state tracking

    // If How To Play was opened via TAB, close the entire pause menu
    if (howToPlayOpenedViaTab)
    {
        //std::cout << "[PauseMenu] How To Play closed via TAB - closing pause menu" << std::endl;
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
    //std::cout << "[PauseMenu] Closing settings screen (ESC pressed)" << std::endl;
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
    howToPlayTransform->posY = -0.55f;
    howToPlayTransform->scaleX = 2000.0f * uiScale;
    howToPlayTransform->scaleY = 1100.0f * uiScale;
    auto* howToPlayRender = howToPlayScreen->AddComponent<RenderComponent>();
    howToPlayRender->SetTextureID(GraphicsSystem::LoadTexture("../Assets/Sprites/M6_ControlPanel_Keyboard.png"));
    howToPlayRender->SetColor(vec4(1.0f, 1.0f, 1.0f, 1.0f));
    howToPlayRender->SetLayer(601);

    GameObject* backBtn = factory.CreateGameObject("HowToPlayBackButton");
    auto* backBtnTransform = backBtn->AddComponent<TransformComponent>();
    backBtnTransform->posX = 690.0f * uiScale;
    backBtnTransform->posY = 300.0f * uiScale;
    backBtnTransform->scaleX = 180.0f * uiScale;
    backBtnTransform->scaleY = 160.0f * uiScale;
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
    //std::cout << "[PauseMenu] How To Play UI destroyed" << std::endl;
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

    // Reset back button to normal state when showing How to Play
    if (show)
    {
        SetButtonTexture("HowToPlayBackButton", ButtonState::Normal);
    }
}

/**
 * @brief Computes the UV bounds for a slider bar by analyzing the texture.
 */
void PauseMenuLogicSystem::ComputeSliderUVBounds(RenderComponent* renderComp, SliderState& sliderState)
{
    std::string texturePath = renderComp->GetTexturePath();

    // if texture path is empty, use the known slider bar texture path
    if (texturePath.empty())
    {
        texturePath = "../Assets/Sprites/Icons/Slider_Bar.png";
    }

    int width = 0;
    int height = 0;
    int channels = 0;
    unsigned char* pixels = stbi_load(texturePath.c_str(), &width, &height, &channels, 4);

    if (pixels && width > 0 && height > 0)
    {
        int minX = width;
        int maxX = -1;
        int pixelsWithAlpha = 0;

        for (int y = 0; y < height; ++y)
        {
            for (int x = 0; x < width; ++x)
            {
                int index = (y * width + x) * 4;
                unsigned char alpha = pixels[index + 3];

                if (alpha > 0)
                {
                    pixelsWithAlpha++;
                    minX = std::min(minX, x);
                    maxX = std::max(maxX, x);
                }
            }
        }

        if (maxX >= minX)
        {
            sliderState.uvBarMin = static_cast<float>(minX) / static_cast<float>(width);
            sliderState.uvBarMax = static_cast<float>(maxX + 1) / static_cast<float>(width);
            sliderState.uvBarWidth = sliderState.uvBarMax - sliderState.uvBarMin;
        }
        else
        {
            sliderState.uvBarMin = 0.0f;
            sliderState.uvBarMax = 1.0f;
            sliderState.uvBarWidth = 1.0f;
        }

        stbi_image_free(pixels);
    }
    else
    {
        sliderState.uvBarMin = 0.0f;
        sliderState.uvBarMax = 1.0f;
        sliderState.uvBarWidth = 1.0f;
    }

    sliderState.uvBoundsReady = true;
}

/**
 * @brief Updates a slider's visual appearance based on volume percentage.
 */
void PauseMenuLogicSystem::UpdateSlider(const std::string& barName, SliderState& sliderState, float volumePercent)
{
    auto& factory = GameObjectFactory::Instance();
    GameObject* sliderBarGO = factory.FindGameObject(barName);

    if (!sliderBarGO)
    {
        return;
    }

    auto* renderComp = sliderBarGO->GetComponent<RenderComponent>();
    auto* transformComp = sliderBarGO->GetComponent<TransformComponent>();

    if (!renderComp || !transformComp)
    {
        return;
    }

    // get background bar to use its current position
    std::string barBackName = barName;
    size_t pos = barBackName.find("_Bar_");
    if (pos != std::string::npos) {
        barBackName.insert(pos + 5, "Back_");
    }
    GameObject* barBack = factory.FindGameObject(barBackName);

    if (!barBack)
    {
        return;
    }

    auto* barBackTransform = barBack->GetComponent<TransformComponent>();
    if (!barBackTransform)
    {
        return;
    }

    // compute the visible UV range once from the actual slider texture
    if (!sliderState.uvBoundsReady)
    {
        ComputeSliderUVBounds(renderComp, sliderState);
    }

    // get current background bar position and scale
    float barBackCenterX, barBackCenterY, barBackScaleX, barBackScaleY;
    barBackTransform->GetPosition(barBackCenterX, barBackCenterY);
    barBackTransform->GetScale(barBackScaleX, barBackScaleY);

    if (sliderState.originalScaleX <= 0.0f)
    {
        transformComp->GetScale(sliderState.originalScaleX, sliderState.originalScaleY);
    }

    float fillPercentage = std::clamp(volumePercent, 0.0f, 1.0f);
    float safeFill = std::max(fillPercentage, 0.001f);

    // calculate the usable slider width from the background bar
    float usableWidth = barBackScaleX * sliderState.uvBarWidth;

    // calculate new width based on fill percentage
    float newScaleX = usableWidth * safeFill;

    // calculate the FIXED left edge position based on background bar
    float leftEdge = (barBackCenterX - barBackScaleX * 0.5f) + (barBackScaleX * sliderState.uvBarMin);

    // center position is left edge + half of the NEW width
    float newCenterX = leftEdge + (newScaleX * 0.5f);

    transformComp->SetScale(newScaleX, sliderState.originalScaleY);
    transformComp->SetPosition(newCenterX, barBackCenterY);

    // crop inside the painted slider bar show from LEFT side of texture
    float croppedUWidth = sliderState.uvBarWidth * safeFill;
    renderComp->SetUVScale(vec2(croppedUWidth, 1.0f));
    renderComp->SetUVOffset(vec2(sliderState.uvBarMin, 0.0f));

    // update knob(scissors icon) position
    std::string knobName;
    if (barName == "PauseSlider_Bar_Master")
        knobName = "PauseSlider_Knob_Master";
    else if (barName == "PauseSlider_Bar_Music")
        knobName = "PauseSlider_Knob_Music";
    else if (barName == "PauseSlider_Bar_SFX")
        knobName = "PauseSlider_Knob_SFX";

    if (!knobName.empty())
    {
        GameObject* knob = factory.FindGameObject(knobName);
        if (knob)
        {
            auto* knobTransform = knob->GetComponent<TransformComponent>();
            if (knobTransform)
            {
                float knobX = newCenterX + newScaleX * 0.5f - 570.0f;
                knobTransform->SetPosition(knobX, barBackCenterY);
            }
        }
    }
}

/**
 * @brief Handles mouse interaction with audio sliders.
 */
void PauseMenuLogicSystem::HandleSliderInput(double mouseX, double mouseY, bool isMouseDown)
{
    auto& factory = GameObjectFactory::Instance();
    static bool wasMouseDown = false;
    bool isMouseClicked = isMouseDown && !wasMouseDown;

    if (isMouseClicked && !isDraggingSlider)
    {
        GameObject* masterBarBack = factory.FindGameObject("PauseSlider_Bar_Back_Master");
        GameObject* musicBarBack = factory.FindGameObject("PauseSlider_Bar_Back_Music");
        GameObject* sfxBarBack = factory.FindGameObject("PauseSlider_Bar_Back_SFX");

        if (IsMouseOver(masterBarBack, mouseX, mouseY))
        {
            isDraggingSlider = true;
            currentSliderCategory = "master";
        }
        else if (IsMouseOver(musicBarBack, mouseX, mouseY))
        {
            isDraggingSlider = true;
            currentSliderCategory = "music";
        }
        else if (IsMouseOver(sfxBarBack, mouseX, mouseY))
        {
            isDraggingSlider = true;
            currentSliderCategory = "sfx";
        }
    }

    wasMouseDown = isMouseDown;

    if (!isMouseDown)
    {
        isDraggingSlider = false;
        currentSliderCategory = "";
    }

    // update slider position while dragging
    if (isDraggingSlider && !currentSliderCategory.empty())
    {
        std::string barBackName;
        SliderState* sliderState = nullptr;
        std::string category;

        if (currentSliderCategory == "master")
        {
            barBackName = "PauseSlider_Bar_Back_Master";
            sliderState = &masterSlider;
            category = "";
        }
        else if (currentSliderCategory == "music")
        {
            barBackName = "PauseSlider_Bar_Back_Music";
            sliderState = &musicSlider;
            category = "music";
        }
        else if (currentSliderCategory == "sfx")
        {
            barBackName = "PauseSlider_Bar_Back_SFX";
            sliderState = &sfxSlider;
            category = "sfx";
        }

        if (sliderState)
        {
            // get background bar to determine the slider range
            GameObject* barBack = factory.FindGameObject(barBackName);
            if (barBack)
            {
                auto* barTransform = barBack->GetComponent<TransformComponent>();
                if (barTransform)
                {
                    float barCenterX, barCenterY, barScaleX, barScaleY;
                    barTransform->GetPosition(barCenterX, barCenterY);
                    barTransform->GetScale(barScaleX, barScaleY);

                    float leftBound = (barCenterX - barScaleX * 0.5f) + (barScaleX * sliderState->uvBarMin);
                    float usableWidth = barScaleX * sliderState->uvBarWidth;
                    float rightBound = leftBound + usableWidth;
                    float mouseXf = static_cast<float>(mouseX);

                    mouseXf = std::clamp(mouseXf, leftBound, rightBound);

                    // calculate volume based on mouse position within the usable slider area
                    float volumePercent = (mouseXf - leftBound) / usableWidth;
                    volumePercent = std::clamp(volumePercent, 0.0f, 1.0f);

                    sliderState->currentVolume = volumePercent;

                    // update the visual slider
                    std::string barName;
                    if (currentSliderCategory == "master")
                        barName = "PauseSlider_Bar_Master";
                    else if (currentSliderCategory == "music")
                        barName = "PauseSlider_Bar_Music";
                    else if (currentSliderCategory == "sfx")
                        barName = "PauseSlider_Bar_SFX";

                    UpdateSlider(barName, *sliderState, volumePercent);

                    // this for the scissors icon
                    std::string knobName;
                    if (currentSliderCategory == "master")
                        knobName = "PauseSlider_Knob_Master";
                    else if (currentSliderCategory == "music")
                        knobName = "PauseSlider_Knob_Music";
                    else if (currentSliderCategory == "sfx")
                        knobName = "PauseSlider_Knob_SFX";

                    // position knob at right edge of filled bar
                    GameObject* sliderBar = factory.FindGameObject(barName);
                    GameObject* knob = factory.FindGameObject(knobName);
                    if (sliderBar && knob)
                    {
                        auto* sliderTransform = sliderBar->GetComponent<TransformComponent>();
                        auto* knobTransform = knob->GetComponent<TransformComponent>();
                        if (sliderTransform && knobTransform)
                        {
                            float sliderCenterX, sliderCenterY, sliderScaleX, sliderScaleY;
                            sliderTransform->GetPosition(sliderCenterX, sliderCenterY);
                            sliderTransform->GetScale(sliderScaleX, sliderScaleY);

                            float knobX = sliderCenterX + sliderScaleX * 0.5f - 570.0f;
                            knobTransform->SetPosition(knobX, sliderCenterY);
                        }
                    }

                    // update audio volume and save it
                    AudioManager* audioManager = CORE->GetAudioManager();
                    if (audioManager)
                    {
                        if (currentSliderCategory == "master")
                        {
                            savedMasterVolume = volumePercent;
                            audioManager->SetMasterVolume(savedMasterVolume);

                            float finalMusicVol = savedMusicVolume * savedMasterVolume;
                            float finalSfxVol = savedSfxVolume * savedMasterVolume;

                            audioManager->SetCategoryVolume("music", finalMusicVol);
                            audioManager->SetCategoryVolume("sfx", finalSfxVol);

                            //std::cout << "[PauseMenu] Master slider - Final volumes: Music=" << finalMusicVol
                            //          << ", SFX=" << finalSfxVol << std::endl;
                        }
                        else
                        {
                            if (currentSliderCategory == "music")
                            {
                                savedMusicVolume = volumePercent;
                                audioManager->SetBaseCategoryVolume("music", volumePercent);
                            }
                            else if (currentSliderCategory == "sfx")
                            {
                                savedSfxVolume = volumePercent;
                                audioManager->SetBaseCategoryVolume("sfx", volumePercent);
                            }

                            float finalVolume = volumePercent * savedMasterVolume;
                            //std::cout << "[PauseMenu] " << currentSliderCategory << " slider - category='" << category
                            //          << "', base=" << volumePercent << ", master=" << savedMasterVolume
                            //          << ", final=" << finalVolume << std::endl;
                            audioManager->SetCategoryVolume(category, finalVolume);
                        }
                    }
                }
            }
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

    // Determine the level subfolder (Level_1, Level_2, Level_3) from the current scene path
    std::string subFolder;
    {
        std::string lvl = currentScenePath;
        size_t slash = lvl.find_last_of("/\\");
        if (slash != std::string::npos) lvl = lvl.substr(slash + 1);
        size_t dot = lvl.rfind('.');
        if (dot != std::string::npos) lvl = lvl.substr(0, dot);
        for (int i = 1; i <= 3; ++i)
        {
            std::string key = "Level_" + std::to_string(i);
            if (lvl.find(key) != std::string::npos)
            {
                subFolder = key;
                break;
            }
        }
    }

    const std::string replayDir = subFolder.empty()
        ? "../Assets/Replay/"
        : "../Assets/Replay/" + subFolder + "/";

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
    boardT->scaleX = 2080.0f * uiScale;
    boardT->scaleY = 1170.0f * uiScale;
    auto* boardR = board->AddComponent<RenderComponent>();
    boardR->SetTextureID(GraphicsSystem::LoadTexture("../Assets/Sprites/Pause_Menu_New.png"));
    boardR->SetColor(vec4(1.0f, 1.0f, 1.0f, 1.0f));
    boardR->SetLayer(baseLayer);

    // Title text
    GameObject* titleObj = factory.CreateGameObject("PauseGhostTitle");
    auto* titleT = titleObj->AddComponent<TransformComponent>();
    titleT->posX = 0.0f;
    titleT->posY = 250.0f * uiScale;
    titleT->scaleX = 380.0f * uiScale;
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
    toggleT->scaleX = 380.0f * uiScale;
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
        slotT->scaleX = 380.0f * uiScale;
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

void PauseMenuLogicSystem::HandleMessage(const WindowFocusMessage& msg) {
    bool isMenuScene = currentScenePath.empty() ||
        currentScenePath.find("MainMenu") != std::string::npos;

    // FOCUS LOST (Alt-Tab Out) --------------------------------------------------------
    if (!msg.hasFocus) {
        if (isMenuScene) {
            // Main menu: game is paused by CoreEngine, but don't show pause menu UI
            return;
        }

        // In-game: show pause menu when alt-tabbing out
        isPauseMenuActive = true;

        if (!hasCreatedUI)
            CreatePauseMenuUI();

        if (!isPauseSettingsOpen && !isGhostMenuOpen && !isHowToPlayOpen && !isLeaderboardOpen)
        {
            SetPauseMenuVisible(true);
        }

        return;
    }

    // FOCUS REGAINED (Alt-Tab Back In) --------------------------------------------------
    if (isMenuScene)
    {
        // Menu scene: game was paused by focus-lost, silently unpause with no menu shown
        CORE->SetGamePaused(false, false);
        return;
    }

    // In-game: Only close How to Play and return to pause menu root if it's open
    // Otherwise keep whatever pause menu screen is currently showing
    if (isHowToPlayOpen)
    {
        isHowToPlayOpen = false;
        wasHowToPlayOpenLastFrame = false;
        howToPlayOpenedViaTab = false;
        SetHowToPlayVisible(false);

        // Show the pause menu root
        isPauseMenuActive = true;

        if (!hasCreatedUI)
            CreatePauseMenuUI();

        SetPauseMenuVisible(true);
    }
}

/**
 * @brief Scans the replay directory for the given level and fills m_lbData (top 3, fastest first).
 */
void PauseMenuLogicSystem::ScanLBData(int level)
{
    m_lbData.clear();
    namespace fs = std::filesystem;

    const std::string dir = "../Assets/Replay/Level_" + std::to_string(level) + "/";
    if (!fs::exists(dir))
        return;

    for (const auto& entry : fs::directory_iterator(dir))
    {
        if (!entry.is_regular_file() || entry.path().extension() != ".json")
            continue;

        std::ifstream f(entry.path().string());
        if (!f) continue;

        try
        {
            nlohmann::json root;
            f >> root;
            float bestTime = root.value("bestTime", FLT_MAX);
            if (bestTime >= FLT_MAX) continue;
            float rating = root.value("rating", 0.0f);
            int shells = root.value("shellsCollected", 0);
            m_lbData.push_back({ entry.path().stem().string(), bestTime, rating, shells });
        }
        catch (...) {}
    }

    std::sort(m_lbData.begin(), m_lbData.end(),
        [](const PauseLBEntry& a, const PauseLBEntry& b) { return a.time < b.time; });
    if (m_lbData.size() > 3)
        m_lbData.resize(3);
}

/**
 * @brief Updates the leaderboard row text/icon objects from m_lbData.
 */
void PauseMenuLogicSystem::PopulateLB()
{
    auto& factory = GameObjectFactory::Instance();
    const char* nameObjs[]   = { "PauseLB_Name_1",   "PauseLB_Name_2",   "PauseLB_Name_3" };
    const char* timeObjs[]   = { "PauseLB_Time_1",   "PauseLB_Time_2",   "PauseLB_Time_3" };
    const char* ratingObjs[] = { "PauseLB_Rating_1", "PauseLB_Rating_2", "PauseLB_Rating_3" };

    for (int i = 0; i < 3; ++i)
    {
        GameObject* nameObj   = factory.FindGameObject(nameObjs[i]);
        GameObject* timeObj   = factory.FindGameObject(timeObjs[i]);
        GameObject* ratingObj = factory.FindGameObject(ratingObjs[i]);

        if (i < static_cast<int>(m_lbData.size()))
        {
            const PauseLBEntry& entry = m_lbData[i];

            // Format time as M:SS.ss (matching main menu format)
            int totalSec = static_cast<int>(entry.time);
            int frac     = static_cast<int>((entry.time - totalSec) * 100.0f);
            int minutes  = totalSec / 60;
            int seconds  = totalSec % 60;
            char timeStr[16];
            snprintf(timeStr, sizeof(timeStr), "%d:%02d.%02d", minutes, seconds, frac);

            if (nameObj) { auto* tc = nameObj->GetComponent<TextComponent>(); if (tc) tc->SetText(entry.name); nameObj->SetActive(true); }
            if (timeObj) { auto* tc = timeObj->GetComponent<TextComponent>(); if (tc) tc->SetText(timeStr); timeObj->SetActive(true); }

            if (ratingObj)
            {
                auto* rc = ratingObj->GetComponent<RenderComponent>();
                if (rc)
                {
                    std::string tex;
                    if (entry.rating >= 85.0f)      tex = "../Assets/Sprites/Icons/Letterscore_S.png";
                    else if (entry.rating >= 75.0f) tex = "../Assets/Sprites/Icons/Letterscore_A.png";
                    else if (entry.rating >= 65.0f) tex = "../Assets/Sprites/Icons/Letterscore_B.png";
                    else if (entry.rating >= 50.0f) tex = "../Assets/Sprites/Icons/Letterscore_C.png";
                    else                            tex = "../Assets/Sprites/Icons/Letterscore_dash.png";
                    rc->SetTextureID(GraphicsSystem::LoadTexture(tex.c_str()));
                }
                ratingObj->SetActive(true);
            }
        }
        else
        {
            if (nameObj)   { auto* tc = nameObj->GetComponent<TextComponent>();   if (tc) tc->SetText("---"); nameObj->SetActive(true); }
            if (timeObj)   { auto* tc = timeObj->GetComponent<TextComponent>();   if (tc) tc->SetText(""); timeObj->SetActive(false); }
            if (ratingObj) ratingObj->SetActive(false);
        }
    }
}

/**
 * @brief Creates the leaderboard overlay UI mirroring the main menu leaderboard layout.
 * Positions are the main menu values × uiScale (2).
 */
void PauseMenuLogicSystem::CreateLeaderboardUI()
{
    if (hasCreatedLeaderboardUI)
        return;

    auto& factory = GameObjectFactory::Instance();
    const float s = 2.0f;
    const std::string font = "../Assets/FrederickatheGreat-Regular.ttf";
    const int baseLayer = 601;

    // Background
    GameObject* board = factory.CreateGameObject("PauseLB_Board");
    {
        auto* t = board->AddComponent<TransformComponent>();
        t->posX = 0.0f; t->posY = 0.0f;
        t->scaleX = 2000.0f * s; t->scaleY = 1100.0f * s;
        auto* r = board->AddComponent<RenderComponent>();
        r->SetTextureID(GraphicsSystem::LoadTexture("../Assets/Leaderboards_menu.png"));
        r->SetColor(vec4(1,1,1,1)); r->SetLayer(baseLayer);
    }

    // Row background bands
    const float rowBandYs[3] = { 4.0f*s, -58.75f*s, -119.93f*s };
    const char* rowTabNames[3] = { "PauseLB_RowTab_1", "PauseLB_RowTab_2", "PauseLB_RowTab_3" };
    for (int i = 0; i < 3; ++i)
    {
        GameObject* rt = factory.CreateGameObject(rowTabNames[i]);
        auto* t = rt->AddComponent<TransformComponent>();
        t->posX = 1.88f*s; t->posY = rowBandYs[i];
        t->scaleX = 1050.0f*s; t->scaleY = 80.0f*s;
        auto* r = rt->AddComponent<RenderComponent>();
        r->SetTextureID(GraphicsSystem::LoadTexture("../Assets/Leaderboards_tab_grey.png"));
        r->SetColor(vec4(1,1,1,1)); r->SetLayer(baseLayer + 1);
    }

    // Level label
    {
        GameObject* lbl = factory.CreateGameObject("PauseLB_LevelLabel");
        auto* t = lbl->AddComponent<TransformComponent>();
        t->posX = 1.88f*s; t->posY = 115.0f*s;
        t->scaleX = 1050.0f*s; t->scaleY = 90.0*s;
        auto* r = lbl->AddComponent<RenderComponent>();
        r->SetTextureID(GraphicsSystem::LoadTexture("../Assets/Leaderboards_tab_grey.png"));
        r->SetColor(vec4(1,1,1,1)); r->SetLayer(baseLayer + 2);
        lbl->AddComponent<TextComponent>(
            font.c_str(), static_cast<int>(28*s), "Level 1", vec4(0,0,0,1), false);
    }

    // Name text objects
    const float nameYs[3] = { 4.0f*s, -59.97f*s, -120.13f*s };
    const char* nameNames[3] = { "PauseLB_Name_1", "PauseLB_Name_2", "PauseLB_Name_3" };
    for (int i = 0; i < 3; ++i)
    {
        GameObject* nm = factory.CreateGameObject(nameNames[i]);
        auto* t = nm->AddComponent<TransformComponent>();
        t->posX = -330.0f*s; t->posY = nameYs[i]; t->scaleX = 1; t->scaleY = 1;
        auto* r = nm->AddComponent<RenderComponent>();
        r->SetColor(vec4(1,0.5f,0.5f,1)); r->SetLayer(baseLayer + 3);
        nm->AddComponent<TextComponent>(
            font.c_str(), static_cast<int>(28*s), "---", vec4(0,0,0,1), false);
    }

    // Time text objects
    const float timeYs[3] = { 5.0f*s, -56.51f*s, -120.0f*s };
    const char* timeNames[3] = { "PauseLB_Time_1", "PauseLB_Time_2", "PauseLB_Time_3" };
    for (int i = 0; i < 3; ++i)
    {
        GameObject* tm = factory.CreateGameObject(timeNames[i]);
        auto* t = tm->AddComponent<TransformComponent>();
        t->posX = 0.0f; t->posY = timeYs[i]; t->scaleX = 1; t->scaleY = 1;
        auto* r = tm->AddComponent<RenderComponent>();
        r->SetColor(vec4(1,0.5f,0.5f,1)); r->SetLayer(baseLayer + 3);
        tm->AddComponent<TextComponent>(
            font.c_str(), static_cast<int>(28*s), "", vec4(0,0,0,1), false);
    }

    // Rating icons
    const float ratingYs[3] = { 7.37f*s, -60.86f*s, -126.0f*s };
    const char* ratingNames[3] = { "PauseLB_Rating_1", "PauseLB_Rating_2", "PauseLB_Rating_3" };
    for (int i = 0; i < 3; ++i)
    {
        GameObject* rt = factory.CreateGameObject(ratingNames[i]);
        auto* t = rt->AddComponent<TransformComponent>();
        t->posX = 424.58f*s; t->posY = ratingYs[i];
        t->scaleX = 40.0f*s; t->scaleY = 40.0f*s;
        auto* r = rt->AddComponent<RenderComponent>();
        r->SetTextureID(GraphicsSystem::LoadTexture("../Assets/Sprites/Icons/Letterscore_dash.png"));
        r->SetColor(vec4(1,1,1,1)); r->SetLayer(baseLayer + 3);
        rt->SetActive(false); // hidden until populated
    }

    // Number circles
    const float circle1Y = 4.0f*s;
    const float circle2Y = -59.97f*s;
    const float circle3Y = -120.13f*s;

    // 1st Circle
    { 
        GameObject* ci = factory.CreateGameObject("PauseLB_Circle_1");
        auto* t = ci->AddComponent<TransformComponent>();
        t->posX = -460.0f*s; t->posY = circle1Y;
        t->scaleX = 60.0f*s; t->scaleY = 55.0f*s;
        auto* r = ci->AddComponent<RenderComponent>();
        r->SetTextureID(GraphicsSystem::LoadTexture("../Assets/Leaderboards_number_circle.png"));
        r->SetColor(vec4(1,1,1,1)); r->SetLayer(baseLayer + 4);

        GameObject* cn = factory.CreateGameObject("PauseLB_CircleNum_1");
        auto* tn = cn->AddComponent<TransformComponent>();
        tn->posX = -460.0f*s; tn->posY = circle1Y; tn->scaleX = 1; tn->scaleY = 1;
        auto* rn = cn->AddComponent<RenderComponent>();
        rn->SetColor(vec4(0,0,0,1)); rn->SetLayer(baseLayer + 5);
        cn->AddComponent<TextComponent>(font.c_str(), static_cast<int>(28*s), "1", vec4(0,0,0,1), false);
    }

    // 2nd Circle
    { 
        GameObject* ci = factory.CreateGameObject("PauseLB_Circle_2");
        auto* t = ci->AddComponent<TransformComponent>();
        t->posX = -460.0f*s; t->posY = circle2Y;
        t->scaleX = 60.0f*s; t->scaleY = 55.0f*s;
        auto* r = ci->AddComponent<RenderComponent>();
        r->SetTextureID(GraphicsSystem::LoadTexture("../Assets/Leaderboards_number_circle.png"));
        r->SetColor(vec4(1,1,1,1)); r->SetLayer(baseLayer + 4);

        GameObject* cn = factory.CreateGameObject("PauseLB_CircleNum_2");
        auto* tn = cn->AddComponent<TransformComponent>();
        tn->posX = -460.0f*s; tn->posY = circle2Y; tn->scaleX = 1; tn->scaleY = 1;
        auto* rn = cn->AddComponent<RenderComponent>();
        rn->SetColor(vec4(0,0,0,1)); rn->SetLayer(baseLayer + 5);
        cn->AddComponent<TextComponent>(font.c_str(), static_cast<int>(28*s), "2", vec4(0,0,0,1), false);
    }

    // 3rd Circle 
    { 
        GameObject* ci = factory.CreateGameObject("PauseLB_Circle_3");
        auto* t = ci->AddComponent<TransformComponent>();
        t->posX = -460.0f*s; t->posY = circle3Y;
        t->scaleX = 60.0f*s; t->scaleY = 55.0f*s;
        auto* r = ci->AddComponent<RenderComponent>();
        r->SetTextureID(GraphicsSystem::LoadTexture("../Assets/Leaderboards_number_circle.png"));
        r->SetColor(vec4(1,1,1,1)); r->SetLayer(baseLayer + 4);

        GameObject* cn = factory.CreateGameObject("PauseLB_CircleNum_3");
        auto* tn = cn->AddComponent<TransformComponent>();
        tn->posX = -460.0f*s; tn->posY = circle3Y; tn->scaleX = 1; tn->scaleY = 1;
        auto* rn = cn->AddComponent<RenderComponent>();
        rn->SetColor(vec4(0,0,0,1)); rn->SetLayer(baseLayer + 5);
        cn->AddComponent<TextComponent>(font.c_str(), static_cast<int>(28*s), "3", vec4(0,0,0,1), false);
    }

    // Back button
    GameObject* backBtn = factory.CreateGameObject("PauseLB_Back");
    {
        auto* t = backBtn->AddComponent<TransformComponent>();
        t->posX = 650.0f*s; t->posY = 244.8f*s;
        t->scaleX = 200.0f*s; t->scaleY = 180.0f*s;
        auto* r = backBtn->AddComponent<RenderComponent>();
        r->SetTextureID(GraphicsSystem::LoadTexture("../Assets/Sprites/Icons/Back_button.png"));
        r->SetColor(vec4(1,1,1,1)); r->SetLayer(baseLayer + 2);
    }

    // Watch Replay button
    GameObject* replayBtn = factory.CreateGameObject("PauseLB_Replay");
    {
        auto* t = replayBtn->AddComponent<TransformComponent>();
        t->posX = -220.0f*s; t->posY = -255.0f*s;
        t->scaleX = 450.0f*s; t->scaleY = 90.0f*s;
        auto* r = replayBtn->AddComponent<RenderComponent>();
        r->SetTextureID(GraphicsSystem::LoadTexture("../Assets/Sprites/Icons/blankbtn.png"));
        r->SetColor(vec4(1,1,1,1)); r->SetLayer(baseLayer + 2);
        replayBtn->AddComponent<TextComponent>(
            font.c_str(), static_cast<int>(28*s), "Watch Replay", vec4(0.1f, 0.05f, 0.0f, 1.0f), false);
    }

    // VS Ghost button
    GameObject* vsGhostBtn = factory.CreateGameObject("PauseLB_VsGhost");
    {
        auto* t = vsGhostBtn->AddComponent<TransformComponent>();
        t->posX = 235.0f*s; t->posY = -255.0f*s;
        t->scaleX = 450.0f*s; t->scaleY = 90.0f*s;
        auto* r = vsGhostBtn->AddComponent<RenderComponent>();
        r->SetTextureID(GraphicsSystem::LoadTexture("../Assets/Sprites/Icons/blankbtn.png"));
        r->SetColor(vec4(1,1,1,1)); r->SetLayer(baseLayer + 2);
        vsGhostBtn->AddComponent<TextComponent>(
            font.c_str(), static_cast<int>(28*s), "Vs Ghost", vec4(0.1f, 0.05f, 0.0f, 1.0f), false);
    }

    hasCreatedLeaderboardUI = true;
    SetLeaderboardVisible(false);
}

/**
 * @brief Destroys all leaderboard overlay UI elements.
 */
void PauseMenuLogicSystem::DestroyLeaderboardUI()
{
    if (!hasCreatedLeaderboardUI)
        return;

    auto& factory = GameObjectFactory::Instance();
    for (const std::string& name : {
        "PauseLB_Board",
        "PauseLB_LevelLabel",
        "PauseLB_RowTab_1", "PauseLB_RowTab_2", "PauseLB_RowTab_3",
        "PauseLB_Name_1", "PauseLB_Name_2", "PauseLB_Name_3",
        "PauseLB_Time_1", "PauseLB_Time_2", "PauseLB_Time_3",
        "PauseLB_Rating_1", "PauseLB_Rating_2", "PauseLB_Rating_3",
        "PauseLB_Circle_1", "PauseLB_Circle_2", "PauseLB_Circle_3",
        "PauseLB_CircleNum_1", "PauseLB_CircleNum_2", "PauseLB_CircleNum_3",
        "PauseLB_Back", "PauseLB_Replay", "PauseLB_VsGhost"})
    {
        GameObject* obj = factory.FindGameObject(name);
        if (obj) factory.DestroyGameObject(obj->GetID());
    }

    hasCreatedLeaderboardUI = false;
    m_lbData.clear();
}

/**
 * @brief Shows or hides leaderboard overlay UI elements.
 */
void PauseMenuLogicSystem::SetLeaderboardVisible(bool show)
{
    auto& factory = GameObjectFactory::Instance();
    for (const std::string& name : {
        "PauseLB_Board",
        "PauseLB_LevelLabel",
        "PauseLB_RowTab_1", "PauseLB_RowTab_2", "PauseLB_RowTab_3",
        "PauseLB_Name_1", "PauseLB_Name_2", "PauseLB_Name_3",
        "PauseLB_Time_1", "PauseLB_Time_2", "PauseLB_Time_3",
        "PauseLB_Circle_1", "PauseLB_Circle_2", "PauseLB_Circle_3",
        "PauseLB_CircleNum_1", "PauseLB_CircleNum_2", "PauseLB_CircleNum_3",
        "PauseLB_Back", "PauseLB_Replay", "PauseLB_VsGhost"})
    {
        GameObject* obj = factory.FindGameObject(name);
        if (obj) obj->SetActive(show);
    }
    // Rating icons
    if (!show)
    {
        for (const std::string& name : { "PauseLB_Rating_1", "PauseLB_Rating_2", "PauseLB_Rating_3" })
        {
            GameObject* obj = factory.FindGameObject(name);
            if (obj) obj->SetActive(false);
        }
    }
}
