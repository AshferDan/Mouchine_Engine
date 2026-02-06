/* Start Header ************************************************************************/
/*!
\file       PauseMenuLogic.cpp
\author     Leck Xin Yee [100% - 793 lines]
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
#include "LevelEditor.h"
#include "CameraManager.h"
#include "WinLoseLogic.h"
#include <iostream>

/**
 * @brief Initializes all pause menu state variables.
 */
void PauseMenuLogicSystem::Initialize()
{
    // Clean up any existing UI before resetting
    DestroyHowToPlayUI();
    DestroyPauseSettingsUI();
    DestroyPauseMenuUI();

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
    if (CORE && CORE->GetWinLoseLogic())
    {
        WinLoseLogicSystem* winLoseSystem = CORE->GetWinLoseLogic();
        isWinLoseScreenActive = winLoseSystem->IsWinScreenActive() || winLoseSystem->IsLoseScreenActive();
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
                AudioManager* audioManager = CORE->GetAudioManager();
                if (audioManager)
                {
                    audioManager->TriggerEvent("UI_Click");
                }

                // Toggle pause menu
                TogglePauseMenu();
                std::cout << "[InGamePauseButton] CLICKED! Opening pause menu" << std::endl;
                }
                else if (isHoveringPauseBtn)
                {
                // Hover state (light)
                rc->SetTextureID(GraphicsSystem::LoadTexture("../Assets/Sprites/Icons/Paused_Button_light.png"));

                // Play hover sound only on hover state change
                if (!wasHoveringLastFrame)
                {
                    AudioManager* audioManager = CORE->GetAudioManager();
                    if (audioManager)
                    {
                        audioManager->TriggerEvent("UI_Hover");
                    }
                    std::cout << "[InGamePauseButton] HOVER START" << std::endl;
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
    if (CORE && CORE->GetWinLoseLogic())
    {
        WinLoseLogicSystem* winLoseSystem = CORE->GetWinLoseLogic();
        if (winLoseSystem->IsWinScreenActive() || winLoseSystem->IsLoseScreenActive())
        {
            shouldHidePauseButton = true;
        }
    }

    if (pauseBtn)
    {
        pauseBtn->SetActive(!shouldHidePauseButton);
    }

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
            "HowToPlayBackButton"
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
            if (hoveredButtonName == "PauseResumeButton")
            {
                std::cout << "[PauseMenu] Resume clicked" << std::endl;
                // Reset button texture before closing
                SetButtonTexture("PauseResumeButton", ButtonState::Normal);
                TogglePauseMenu(); // close menu and unpause
            }
            else if (hoveredButtonName == "PauseRestartButton")
            {
                std::cout << "[PauseMenu] Restart clicked" << std::endl;
                // Destroy UI before scene change
                DestroyHowToPlayUI();
                DestroyPauseSettingsUI();
                DestroyPauseMenuUI();

                // close menu first
                if (isPauseMenuActive)
                {
                    isPauseMenuActive = false;
                    CORE->SetGamePaused(false);
                }
                // restart the current level
                if (!currentScenePath.empty())
                {
                    CORE->RequestSceneLoad(currentScenePath, true);
                }
            }
            else if (hoveredButtonName == "PauseSettingsButton")
            {
                std::cout << "[PauseMenu] Settings clicked - opening settings" << std::endl;

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
                std::cout << "[PauseMenu] Main Menu clicked" << std::endl;

                // Start audio fade out before going to main menu
                audioManager = CORE->GetAudioManager();
                if (audioManager)
                {
                    audioManager->FadeOutCategory("music", 0.5f);
                    std::cout << "[PauseMenu] Started music fade out" << std::endl;
                }

                // Destroy UI before scene change
                DestroyHowToPlayUI();
                DestroyPauseSettingsUI();
                DestroyPauseMenuUI();

                // close menu first
                if (isPauseMenuActive)
                {
                    isPauseMenuActive = false;
                    CORE->SetGamePaused(false);
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

            // Show how to play UI
            SetHowToPlayVisible(true);

            // Reset button textures to normal before hiding them
            SetButtonTexture("PauseResumeButton", ButtonState::Normal);
            SetButtonTexture("PauseRestartButton", ButtonState::Normal);
            SetButtonTexture("PauseSettingsButton", ButtonState::Normal);
            SetButtonTexture("PauseMainMenuButton", ButtonState::Normal);
            SetButtonTexture("PauseHowToPlayButton", ButtonState::Normal);

            // Hide pause menu buttons
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
        }
        else
        {
            // Hide how to play UI
            SetHowToPlayVisible(false);

            // Reset how to play button texture to normal
            SetButtonTexture("PauseHowToPlayButton", ButtonState::Normal);

            // Show pause menu buttons
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
        }
        wasHowToPlayOpenLastFrame = isHowToPlayOpen;
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

            // Hide pause menu buttons
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
        }
        else
        {
            // Hide settings UI
            SetPauseSettingsVisible(false);

            // Show pause menu buttons
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
        }
        wasSettingsOpenLastFrame = isPauseSettingsOpen;
    }

    // Handle pause settings screen interactions
    if (isPauseSettingsOpen)
    {
        // Handle pending settings close (back button was clicked)
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
        if (glfwGetKey(glfwGetCurrentContext(), GLFW_KEY_ESCAPE) == GLFW_PRESS)
        {
            static bool escWasPressed = false;
            if (!escWasPressed)
            {
                std::cout << "[PauseMenu] ESC pressed - closing how to play" << std::endl;
                pendingHowToPlayClose = true;
                howToPlayCloseTimer = 0.15f;
                escWasPressed = true;
            }
        }
        else
        {
            static bool escWasPressed = false;
            escWasPressed = false;
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
            pendingHowToPlayClose = true;
            howToPlayCloseTimer = 0.15f;
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
            }
            else
            {
                audioManager->TriggerEvent("UI_Click");
            }
        }

        // start delay before executing action
        pendingSceneChange = true;
        pendingTimer = 0.20f; // 200ms delay
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
    //auto* restartText = restartBtn->AddComponent<TextComponent>(
    //    "../Assets/FrederickatheGreat-Regular.ttf", 32, "Restart", vec4(0.0f, 0.0f, 0.0f, 1.0f), false);

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
    //auto* settingsText = settingsBtn->AddComponent<TextComponent>(
    //    "../Assets/FrederickatheGreat-Regular.ttf", 32, "Settings", vec4(0.0f, 0.0f, 0.0f, 1.0f), false);

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
    settingsTransform->scaleX = 1755.0f * uiScale;
    settingsTransform->scaleY = 945.0f * uiScale;
    auto* settingsRender = settingsScreen->AddComponent<RenderComponent>();
    settingsRender->SetTextureID(GraphicsSystem::LoadTexture("../Assets/Sprites/Settings.png"));
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
        "PauseHowToPlayButton"
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
        "PauseHowToPlayButton"
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

    isPauseMenuActive = !isPauseMenuActive;
    SetPauseMenuVisible(isPauseMenuActive);

    // Reset pause settings and how to play state when closing pause menu
    if (!isPauseMenuActive)
    {
        isPauseSettingsOpen = false;
        wasSettingsOpenLastFrame = false;
        isHowToPlayOpen = false;
        wasHowToPlayOpenLastFrame = false;

        // Reset all button textures when closing pause menu
        SetButtonTexture("PauseResumeButton", ButtonState::Normal);
        SetButtonTexture("PauseRestartButton", ButtonState::Normal);
        SetButtonTexture("PauseSettingsButton", ButtonState::Normal);
        SetButtonTexture("PauseMainMenuButton", ButtonState::Normal);
        SetButtonTexture("PauseHowToPlayButton", ButtonState::Normal);
    }

    // pause/unpause the game
    CORE->SetGamePaused(isPauseMenuActive);
    //CORE->GameTimerSystem();

    std::cout << "[PauseMenu] Pause menu " << (isPauseMenuActive ? "opened" : "closed") << std::endl;
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
    howToPlayTransform->scaleX = 1755.0f * uiScale;
    howToPlayTransform->scaleY = 945.0f * uiScale;
    auto* howToPlayRender = howToPlayScreen->AddComponent<RenderComponent>();
    howToPlayRender->SetTextureID(GraphicsSystem::LoadTexture("../Assets/ControlPanelm4.png"));
    howToPlayRender->SetColor(vec4(1.0f, 1.0f, 1.0f, 1.0f));
    howToPlayRender->SetLayer(601);

    GameObject* backBtn = factory.CreateGameObject("HowToPlayBackButton");
    auto* backBtnTransform = backBtn->AddComponent<TransformComponent>();
    backBtnTransform->posX = 750.0f * uiScale;
    backBtnTransform->posY = -275.0f * uiScale;
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
