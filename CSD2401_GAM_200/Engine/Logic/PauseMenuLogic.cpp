/* Start Header ************************************************************************/
/*!
\file       PauseMenuLogic.cpp
\author     Leck Xin Yee [100% - 309 lines]
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
#include <iostream>

/**
 * @brief Initializes all pause menu state variables.
 */
void PauseMenuLogicSystem::Initialize()
{
    isPauseMenuActive = false;
    hasCreatedUI = false;
    hoveredButtonName = "";
    pendingSceneChange = false;
    isPauseSettingsOpen = false;
    hasCreatedSettingsUI = false;
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

    // update pause menu position to follow camera
    if (isPauseMenuActive || hasCreatedUI)
    {
        vec2 camPos = CameraManager::Instance().GetCameraPosition();
        auto& factory = GameObjectFactory::Instance();

        std::vector<std::string> uiElements = {
            "PauseOverlay",
            "PauseBoard",
            "PauseResumeButton",
            "PauseRestartButton",
            "PauseSettingsButton",
            "PauseMainMenuButton",
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
                TogglePauseMenu(); // close menu and unpause
            }
            else if (hoveredButtonName == "PauseRestartButton")
            {
                std::cout << "[PauseMenu] Restart clicked" << std::endl;
                // close menu first
                if (isPauseMenuActive)
                {
                    SetPauseMenuVisible(false);
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
            }
            else if (hoveredButtonName == "PauseMainMenuButton")
            {
                std::cout << "[PauseMenu] Main Menu clicked" << std::endl;

                // Start audio fade out before going to main menu
                AudioManager* audioManager = CORE->GetAudioManager();
                if (audioManager)
                {
                    audioManager->FadeOutCategory("music", 0.5f);
                    std::cout << "[PauseMenu] Started music fade out" << std::endl;
                }

                // close menu first
                if (isPauseMenuActive)
                {
                    SetPauseMenuVisible(false);
                    isPauseMenuActive = false;
                    CORE->SetGamePaused(false);
                }
                // back to main menu
                CORE->RequestSceneLoad("../Assets/Scenes/MainMenu.json", true);
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

    auto& factory = GameObjectFactory::Instance();

    // Handle pause settings screen visibility
    if (isPauseSettingsOpen)
    {
        // Show settings UI
        SetPauseSettingsVisible(true);

        // Hide pause menu buttons
        if (auto* resumeBtn = factory.FindGameObject("PauseResumeButton"))
            resumeBtn->SetActive(false);
        if (auto* restartBtn = factory.FindGameObject("PauseRestartButton"))
            restartBtn->SetActive(false);
        if (auto* settingsBtn = factory.FindGameObject("PauseSettingsButton"))
            settingsBtn->SetActive(false);
        if (auto* mainMenuBtn = factory.FindGameObject("PauseMainMenuButton"))
            mainMenuBtn->SetActive(false);

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
                SetButtonTexture(hoveredButtonName, false);
            }

            if (!currentHover.empty())
            {
                SetButtonTexture(currentHover, true);
                AudioManager* audioManager = CORE->GetAudioManager();
                if (audioManager)
                {
                    audioManager->TriggerEvent("UI_Hover");
                }
            }

            hoveredButtonName = currentHover;
        }

        // Handle clicks
        if (isClicked && !hoveredButtonName.empty())
        {
            AudioManager* audioManager = CORE->GetAudioManager();
            if (audioManager)
            {
                audioManager->TriggerEvent("UI_Click");
            }

            if (hoveredButtonName == "PauseBackButton")
            {
                std::cout << "[PauseMenu] Back button clicked - closing settings" << std::endl;
                isPauseSettingsOpen = false;
                hoveredButtonName.clear();
            }
            else if (hoveredButtonName == "PauseMasterMuteButton")
            {
                if (audioManager)
                {
                    audioManager->ToggleMasterMute();
                }
                SetButtonTexture("PauseMasterMuteButton", true);
            }
            else if (hoveredButtonName == "PauseMusicMuteButton")
            {
                if (audioManager)
                {
                    audioManager->ToggleCategoryMute("music");
                }
                SetButtonTexture("PauseMusicMuteButton", true);
            }
            else if (hoveredButtonName == "PauseSFXMuteButton")
            {
                if (audioManager)
                {
                    audioManager->ToggleCategoryMute("sfx");
                }
                SetButtonTexture("PauseSFXMuteButton", true);
            }
        }

        return; // Don't process normal pause menu interactions
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
    }

    double mouseX = InputManager::GetMouseWorldX();
    double mouseY = InputManager::GetMouseWorldY();
    bool isClicked = InputManager::IsMouseLeftClicked();

    std::string currentHover = "";

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

    if (currentHover != hoveredButtonName)
    {
        if (!hoveredButtonName.empty())
        {
            SetButtonTexture(hoveredButtonName, false);
        }

        if (!currentHover.empty())
        {
            SetButtonTexture(currentHover, true);
            AudioManager* audioManager = CORE->GetAudioManager();
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
        // play click sound
        AudioManager* audioManager = CORE->GetAudioManager();
        if (audioManager)
        {
            audioManager->TriggerEvent("UI_Click");
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

    // create semi-transparent overlay background
    GameObject* overlay = factory.CreateGameObject("PauseOverlay");
    auto* overlayTransform = overlay->AddComponent<TransformComponent>();
    overlayTransform->posX = 0.0f;
    overlayTransform->posY = 0.0f;
    overlayTransform->scaleX = 5000.0f;
    overlayTransform->scaleY = 5000.0f;
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
    boardTransform->scaleX = 1920.0f;
    boardTransform->scaleY = 1080.0f;
    auto* boardRender = board->AddComponent<RenderComponent>();
    boardRender->SetTextureID(GraphicsSystem::LoadTexture("../Assets/Sprites/Pause_Menu_New.png"));
    boardRender->SetColor(vec4(1.0f, 1.0f, 1.0f, 1.0f));
    boardRender->SetLayer(601); // above overlay

    // paused button
    GameObject* pausedBtn = factory.CreateGameObject("PauseResumeButton");
    auto* pausedTransform = pausedBtn->AddComponent<TransformComponent>();
    pausedTransform->posX = 650.0f;
    pausedTransform->posY = -275.0f;
    pausedTransform->scaleX = 200.0f;
    pausedTransform->scaleY = 175.0f;
    auto* pausedRender = pausedBtn->AddComponent<RenderComponent>();
    pausedRender->SetTextureID(GraphicsSystem::LoadTexture("../Assets/Sprites/Icons/paused_button.png"));
    pausedRender->SetColor(vec4(1.0f, 1.0f, 1.0f, 1.0f));
    pausedRender->SetLayer(602);

    // restart button
    GameObject* restartBtn = factory.CreateGameObject("PauseRestartButton");
    auto* restartTransform = restartBtn->AddComponent<TransformComponent>();
    restartTransform->posX = 0.0f;
    restartTransform->posY = -80.0f;
    restartTransform->scaleX = 420.0f;
    restartTransform->scaleY = 100.0f;
    auto* restartRender = restartBtn->AddComponent<RenderComponent>();
    restartRender->SetTextureID(GraphicsSystem::LoadTexture("../Assets//Sprites/Icons/blankbtnL.png"));
    restartRender->SetColor(vec4(1.0f, 1.0f, 1.0f, 1.0f));
    restartRender->SetLayer(602);
    restartBtn->AddComponent<TextComponent>(
        "../Assets/FrederickatheGreat-Regular.ttf", 38, "Restart", vec4(0.0f, 0.0f, 0.0f, 1.0f), false);
    //auto* restartText = restartBtn->AddComponent<TextComponent>(
    //    "../Assets/FrederickatheGreat-Regular.ttf", 32, "Restart", vec4(0.0f, 0.0f, 0.0f, 1.0f), false);

    // settings button
    GameObject* settingsBtn = factory.CreateGameObject("PauseSettingsButton");
    auto* settingsTransform = settingsBtn->AddComponent<TransformComponent>();
    settingsTransform->posX = 0.0f;
    settingsTransform->posY = 40.0f;
    settingsTransform->scaleX = 420.0f;
    settingsTransform->scaleY = 100.0f;
    auto* settingsRender = settingsBtn->AddComponent<RenderComponent>();
    settingsRender->SetTextureID(GraphicsSystem::LoadTexture("../Assets/Sprites/Icons/blankbtnL.png"));
    settingsRender->SetColor(vec4(1.0f, 1.0f, 1.0f, 1.0f));
    settingsRender->SetLayer(602);
    settingsBtn->AddComponent<TextComponent>(
        "../Assets/FrederickatheGreat-Regular.ttf", 38, "Settings", vec4(0.0f, 0.0f, 0.0f, 1.0f), false);
    //auto* settingsText = settingsBtn->AddComponent<TextComponent>(
    //    "../Assets/FrederickatheGreat-Regular.ttf", 32, "Settings", vec4(0.0f, 0.0f, 0.0f, 1.0f), false);

    // quit button
    GameObject* mainMenuBtn = factory.CreateGameObject("PauseMainMenuButton");
    auto* mainMenuTransform = mainMenuBtn->AddComponent<TransformComponent>();
    mainMenuTransform->posX = 0.0f;
    mainMenuTransform->posY = -200.0f;
    mainMenuTransform->scaleX = 420.0f;
    mainMenuTransform->scaleY = 100.0f;
    auto* mainMenuRender = mainMenuBtn->AddComponent<RenderComponent>();
    mainMenuRender->SetTextureID(GraphicsSystem::LoadTexture("../Assets/Sprites/Icons/blankbtnL.png"));
    mainMenuRender->SetColor(vec4(1.0f, 1.0f, 1.0f, 1.0f));
    mainMenuRender->SetLayer(602);
    mainMenuBtn->AddComponent<TextComponent>(
        "../Assets/FrederickatheGreat-Regular.ttf", 38, "Quit", vec4(0.0f, 0.0f, 0.0f, 1.0f), false);
    //auto* mainMenuText = mainMenuBtn->AddComponent<TextComponent>(
    //    "../Assets/FrederickatheGreat-Regular.ttf", 32, "Main Menu", vec4(0.0f, 0.0f, 0.0f, 1.0f), false);

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

    // Settings screen background
    GameObject* settingsScreen = factory.CreateGameObject("PauseScreen_Settings");
    auto* settingsTransform = settingsScreen->AddComponent<TransformComponent>();
    settingsTransform->posX = 0.0f;
    settingsTransform->posY = 0.0f;
    settingsTransform->scaleX = 1920.0f;
    settingsTransform->scaleY = 1080.0f;
    auto* settingsRender = settingsScreen->AddComponent<RenderComponent>();
    settingsRender->SetTextureID(GraphicsSystem::LoadTexture("../Assets/Sprites/Menu_settings.png"));
    settingsRender->SetColor(vec4(1.0f, 1.0f, 1.0f, 1.0f));
    settingsRender->SetLayer(601);

    // Master Text
    GameObject* masterText = factory.CreateGameObject("PauseMasterText");
    auto* masterTextTransform = masterText->AddComponent<TransformComponent>();
    masterTextTransform->posX = -350.0f;
    masterTextTransform->posY = 50.0f;
    masterTextTransform->scaleX = 1.0f;
    masterTextTransform->scaleY = 1.0f;
    auto* masterTextRender = masterText->AddComponent<RenderComponent>();
    masterTextRender->SetColor(vec4(1.0f, 0.5f, 0.5f, 1.0f));
    masterTextRender->SetLayer(603);
    masterText->AddComponent<TextComponent>(
        "../Assets/FrederickatheGreat-Regular.ttf", 48, "Master", vec4(0.0f, 0.0f, 0.0f, 1.0f), false);

    // Music Text
    GameObject* musicText = factory.CreateGameObject("PauseMusicText");
    auto* musicTextTransform = musicText->AddComponent<TransformComponent>();
    musicTextTransform->posX = -340.0f;
    musicTextTransform->posY = -75.0f;
    musicTextTransform->scaleX = 1.0f;
    musicTextTransform->scaleY = 1.0f;
    auto* musicTextRender = musicText->AddComponent<RenderComponent>();
    musicTextRender->SetColor(vec4(1.0f, 0.5f, 0.5f, 1.0f));
    musicTextRender->SetLayer(603);
    musicText->AddComponent<TextComponent>(
        "../Assets/FrederickatheGreat-Regular.ttf", 48, "Music", vec4(0.0f, 0.0f, 0.0f, 1.0f), false);

    // SFX Text
    GameObject* sfxText = factory.CreateGameObject("PauseSFXText");
    auto* sfxTextTransform = sfxText->AddComponent<TransformComponent>();
    sfxTextTransform->posX = -320.0f;
    sfxTextTransform->posY = -200.0f;
    sfxTextTransform->scaleX = 1.0f;
    sfxTextTransform->scaleY = 1.0f;
    auto* sfxTextRender = sfxText->AddComponent<RenderComponent>();
    sfxTextRender->SetColor(vec4(1.0f, 0.5f, 0.5f, 1.0f));
    sfxTextRender->SetLayer(603);
    sfxText->AddComponent<TextComponent>(
        "../Assets/FrederickatheGreat-Regular.ttf", 48, "SFX", vec4(0.0f, 0.0f, 0.0f, 1.0f), false);

    // Master Slider - Bar
    GameObject* masterBar = factory.CreateGameObject("PauseSlider_Bar_Master");
    auto* masterBarTransform = masterBar->AddComponent<TransformComponent>();
    masterBarTransform->posX = 200.0f;
    masterBarTransform->posY = 50.0f;
    masterBarTransform->scaleX = 670.0f;
    masterBarTransform->scaleY = 110.0f;
    auto* masterBarRender = masterBar->AddComponent<RenderComponent>();
    masterBarRender->SetTextureID(GraphicsSystem::LoadTexture("../Assets/Sprites/Icons/Slider_Bar.png"));
    masterBarRender->SetColor(vec4(1.0f, 1.0f, 1.0f, 1.0f));
    masterBarRender->SetLayer(602);

    // Master Slider - Back
    GameObject* masterBarBack = factory.CreateGameObject("PauseSlider_Bar_Back_Master");
    auto* masterBarBackTransform = masterBarBack->AddComponent<TransformComponent>();
    masterBarBackTransform->posX = 200.0f;
    masterBarBackTransform->posY = 50.0f;
    masterBarBackTransform->scaleX = 670.0f;
    masterBarBackTransform->scaleY = 110.0f;
    auto* masterBarBackRender = masterBarBack->AddComponent<RenderComponent>();
    masterBarBackRender->SetTextureID(GraphicsSystem::LoadTexture("../Assets/Sprites/Icons/Slider_Bar_Back.png"));
    masterBarBackRender->SetColor(vec4(1.0f, 1.0f, 1.0f, 1.0f));
    masterBarBackRender->SetLayer(601);

    // Master Slider - Knob
    GameObject* masterKnob = factory.CreateGameObject("PauseSlider_Knob_Master");
    auto* masterKnobTransform = masterKnob->AddComponent<TransformComponent>();
    masterKnobTransform->posX = 200.0f;
    masterKnobTransform->posY = 50.0f;
    masterKnobTransform->scaleX = 670.0f;
    masterKnobTransform->scaleY = 110.0f;
    auto* masterKnobRender = masterKnob->AddComponent<RenderComponent>();
    masterKnobRender->SetTextureID(GraphicsSystem::LoadTexture("../Assets/Sprites/Icons/Slider_Icon.png"));
    masterKnobRender->SetColor(vec4(1.0f, 1.0f, 1.0f, 1.0f));
    masterKnobRender->SetLayer(603);

    // Music Slider - Bar
    GameObject* musicBar = factory.CreateGameObject("PauseSlider_Bar_Music");
    auto* musicBarTransform = musicBar->AddComponent<TransformComponent>();
    musicBarTransform->posX = 200.0f;
    musicBarTransform->posY = -75.0f;
    musicBarTransform->scaleX = 670.0f;
    musicBarTransform->scaleY = 110.0f;
    auto* musicBarRender = musicBar->AddComponent<RenderComponent>();
    musicBarRender->SetTextureID(GraphicsSystem::LoadTexture("../Assets/Sprites/Icons/Slider_Bar.png"));
    musicBarRender->SetColor(vec4(1.0f, 1.0f, 1.0f, 1.0f));
    musicBarRender->SetLayer(602);

    // Music Slider - Back
    GameObject* musicBarBack = factory.CreateGameObject("PauseSlider_Bar_Back_Music");
    auto* musicBarBackTransform = musicBarBack->AddComponent<TransformComponent>();
    musicBarBackTransform->posX = 200.0f;
    musicBarBackTransform->posY = -75.0f;
    musicBarBackTransform->scaleX = 670.0f;
    musicBarBackTransform->scaleY = 110.0f;
    auto* musicBarBackRender = musicBarBack->AddComponent<RenderComponent>();
    musicBarBackRender->SetTextureID(GraphicsSystem::LoadTexture("../Assets/Sprites/Icons/Slider_Bar_Back.png"));
    musicBarBackRender->SetColor(vec4(1.0f, 1.0f, 1.0f, 1.0f));
    musicBarBackRender->SetLayer(601);

    // Music Slider - Knob
    GameObject* musicKnob = factory.CreateGameObject("PauseSlider_Knob_Music");
    auto* musicKnobTransform = musicKnob->AddComponent<TransformComponent>();
    musicKnobTransform->posX = 200.0f;
    musicKnobTransform->posY = -75.0f;
    musicKnobTransform->scaleX = 670.0f;
    musicKnobTransform->scaleY = 110.0f;
    auto* musicKnobRender = musicKnob->AddComponent<RenderComponent>();
    musicKnobRender->SetTextureID(GraphicsSystem::LoadTexture("../Assets/Sprites/Icons/Slider_Icon.png"));
    musicKnobRender->SetColor(vec4(1.0f, 1.0f, 1.0f, 1.0f));
    musicKnobRender->SetLayer(603);

    // SFX Slider - Bar
    GameObject* sfxBar = factory.CreateGameObject("PauseSlider_Bar_SFX");
    auto* sfxBarTransform = sfxBar->AddComponent<TransformComponent>();
    sfxBarTransform->posX = 200.0f;
    sfxBarTransform->posY = -200.0f;
    sfxBarTransform->scaleX = 670.0f;
    sfxBarTransform->scaleY = 110.0f;
    auto* sfxBarRender = sfxBar->AddComponent<RenderComponent>();
    sfxBarRender->SetTextureID(GraphicsSystem::LoadTexture("../Assets/Sprites/Icons/Slider_Bar.png"));
    sfxBarRender->SetColor(vec4(1.0f, 1.0f, 1.0f, 1.0f));
    sfxBarRender->SetLayer(602);

    // SFX Slider - Back
    GameObject* sfxBarBack = factory.CreateGameObject("PauseSlider_Bar_Back_SFX");
    auto* sfxBarBackTransform = sfxBarBack->AddComponent<TransformComponent>();
    sfxBarBackTransform->posX = 200.0f;
    sfxBarBackTransform->posY = -200.0f;
    sfxBarBackTransform->scaleX = 670.0f;
    sfxBarBackTransform->scaleY = 110.0f;
    auto* sfxBarBackRender = sfxBarBack->AddComponent<RenderComponent>();
    sfxBarBackRender->SetTextureID(GraphicsSystem::LoadTexture("../Assets/Sprites/Icons/Slider_Bar_Back.png"));
    sfxBarBackRender->SetColor(vec4(1.0f, 1.0f, 1.0f, 1.0f));
    sfxBarBackRender->SetLayer(601);

    // SFX Slider - Knob
    GameObject* sfxKnob = factory.CreateGameObject("PauseSlider_Knob_SFX");
    auto* sfxKnobTransform = sfxKnob->AddComponent<TransformComponent>();
    sfxKnobTransform->posX = 200.0f;
    sfxKnobTransform->posY = -200.0f;
    sfxKnobTransform->scaleX = 670.0f;
    sfxKnobTransform->scaleY = 110.0f;
    auto* sfxKnobRender = sfxKnob->AddComponent<RenderComponent>();
    sfxKnobRender->SetTextureID(GraphicsSystem::LoadTexture("../Assets/Sprites/Icons/Slider_Icon.png"));
    sfxKnobRender->SetColor(vec4(1.0f, 1.0f, 1.0f, 1.0f));
    sfxKnobRender->SetLayer(603);

    // Master Mute Button
    GameObject* masterMute = factory.CreateGameObject("PauseMasterMuteButton");
    auto* masterMuteTransform = masterMute->AddComponent<TransformComponent>();
    masterMuteTransform->posX = -200.0f;
    masterMuteTransform->posY = 50.5f;
    masterMuteTransform->scaleX = 125.0f;
    masterMuteTransform->scaleY = 100.0f;
    auto* masterMuteRender = masterMute->AddComponent<RenderComponent>();
    masterMuteRender->SetTextureID(GraphicsSystem::LoadTexture("../Assets/Sprites/Icons/Volume_Unmute.png"));
    masterMuteRender->SetColor(vec4(1.0f, 1.0f, 1.0f, 1.0f));
    masterMuteRender->SetLayer(601);

    // Music Mute Button
    GameObject* musicMute = factory.CreateGameObject("PauseMusicMuteButton");
    auto* musicMuteTransform = musicMute->AddComponent<TransformComponent>();
    musicMuteTransform->posX = -200.0f;
    musicMuteTransform->posY = -74.5f;
    musicMuteTransform->scaleX = 125.0f;
    musicMuteTransform->scaleY = 100.0f;
    auto* musicMuteRender = musicMute->AddComponent<RenderComponent>();
    musicMuteRender->SetTextureID(GraphicsSystem::LoadTexture("../Assets/Sprites/Icons/Volume_Unmute.png"));
    musicMuteRender->SetColor(vec4(1.0f, 1.0f, 1.0f, 1.0f));
    musicMuteRender->SetLayer(601);

    // SFX Mute Button
    GameObject* sfxMute = factory.CreateGameObject("PauseSFXMuteButton");
    auto* sfxMuteTransform = sfxMute->AddComponent<TransformComponent>();
    sfxMuteTransform->posX = -200.0f;
    sfxMuteTransform->posY = -199.5f;
    sfxMuteTransform->scaleX = 125.0f;
    sfxMuteTransform->scaleY = 100.0f;
    auto* sfxMuteRender = sfxMute->AddComponent<RenderComponent>();
    sfxMuteRender->SetTextureID(GraphicsSystem::LoadTexture("../Assets/Sprites/Icons/Volume_Unmute.png"));
    sfxMuteRender->SetColor(vec4(1.0f, 1.0f, 1.0f, 1.0f));
    sfxMuteRender->SetLayer(601);

    // Back Button
    GameObject* backBtn = factory.CreateGameObject("PauseBackButton");
    auto* backBtnTransform = backBtn->AddComponent<TransformComponent>();
    backBtnTransform->posX = 650.0f;
    backBtnTransform->posY = -250.0f;
    backBtnTransform->scaleX = 200.0f;
    backBtnTransform->scaleY = 180.0f;
    auto* backBtnRender = backBtn->AddComponent<RenderComponent>();
    backBtnRender->SetTextureID(GraphicsSystem::LoadTexture("../Assets/Sprites/Icons/Back_Button.png"));
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
        "PauseMainMenuButton"
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
        "PauseMainMenuButton"
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

    isPauseMenuActive = !isPauseMenuActive;
    SetPauseMenuVisible(isPauseMenuActive);

    // Reset pause settings state when closing pause menu
    if (!isPauseMenuActive)
    {
        isPauseSettingsOpen = false;
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
 * @brief Updates button texture based on hover state.
 */
void PauseMenuLogicSystem::SetButtonTexture(std::string buttonName, bool isHovered)
{
    auto& factory = GameObjectFactory::Instance();
    GameObject* btn = factory.FindGameObject(buttonName);
    if (!btn) return;

    auto* rc = btn->GetComponent<RenderComponent>();
    if (!rc) return;

    // swap between normal and hover button textures
    std::string texturePath;

    if (buttonName == "PauseResumeButton")
    {
        texturePath = isHovered ? "../Assets/Sprites/Icons/playing_button.png" : "../Assets/Sprites/Icons/paused_button.png";
    }
    else if (buttonName == "PauseMasterMuteButton")
    {
        bool isMuted = false;
        AudioManager* audioManager = CORE->GetAudioManager();
        if (audioManager)
        {
            isMuted = audioManager->IsMuted();
        }
        texturePath = isMuted ? "../Assets/Sprites/Icons/Volume_Mute.png" : "../Assets/Sprites/Icons/Volume_Unmute.png";
    }
    else if (buttonName == "PauseMusicMuteButton")
    {
        bool isMuted = false;
        AudioManager* audioManager = CORE->GetAudioManager();
        if (audioManager)
        {
            isMuted = audioManager->IsCategoryMuted("music");
        }
        texturePath = isMuted ? "../Assets/Sprites/Icons/Volume_Mute.png" : "../Assets/Sprites/Icons/Volume_Unmute.png";
    }
    else if (buttonName == "PauseSFXMuteButton")
    {
        bool isMuted = false;
        AudioManager* audioManager = CORE->GetAudioManager();
        if (audioManager)
        {
            isMuted = audioManager->IsCategoryMuted("sfx");
        }
        texturePath = isMuted ? "../Assets/Sprites/Icons/Volume_Mute.png" : "../Assets/Sprites/Icons/Volume_Unmute.png";
    }
    else if (buttonName == "PauseBackButton")
    {
        texturePath = isHovered ? "../Assets/Sprites/Icons/Back_button_dark.png" : "../Assets/Sprites/Icons/Back_button.png";
    }
    else
    {
        texturePath = isHovered ? "../Assets/Sprites/Icons/hoveringbtnL.png" : "../Assets/Sprites/Icons/blankbtnL.png";
    }

    unsigned int newtex = GraphicsSystem::LoadTexture(texturePath.c_str());
    rc->SetTextureID(newtex);
}
