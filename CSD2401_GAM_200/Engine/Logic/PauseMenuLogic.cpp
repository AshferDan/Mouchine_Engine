#include "PauseMenuLogic.h"
#include "core.h"
#include "LevelEditor.h"
#include <iostream>

void PauseMenuLogicSystem::Initialize()
{
    isPauseMenuActive = false;
    hasCreatedUI = false;
    hoveredButtonName = "";
    pendingSceneChange = false;
}

void PauseMenuLogicSystem::Update(float dt)
{
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
                // Restart the current level
                if (!currentScenePath.empty())
                {
                    CORE->RequestSceneLoad(currentScenePath, true);
                }
            }
            else if (hoveredButtonName == "PauseSettingsButton")
            {
                std::cout << "[PauseMenu] Settings clicked (not implemented yet)" << std::endl;
                // Settings not implemented yet
            }
            else if (hoveredButtonName == "PauseMainMenuButton")
            {
                std::cout << "[PauseMenu] Main Menu clicked" << std::endl;
                // Close menu first
                if (isPauseMenuActive)
                {
                    SetPauseMenuVisible(false);
                    isPauseMenuActive = false;
                    CORE->SetGamePaused(false);
                }
                // Go back to main menu
                CORE->RequestSceneLoad("../Assets/Scenes/MainMenu.json", true);
            }

            pendingSceneChange = false;
            hoveredButtonName.clear();
        }
        return;
    }



    // Only process menu interactions if pause menu is active
    if (!isPauseMenuActive)
    {
        return;
    }

    // Get mouse input
    double mouseX = InputManager::GetMouseWorldX();
    double mouseY = InputManager::GetMouseWorldY();
    bool isClicked = InputManager::IsMouseLeftClicked();

    auto& factory = GameObjectFactory::Instance();
    std::string currentHover = "";

    // Check all pause menu buttons for hover
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

    // if the hovered button has changed since last frame
    if (currentHover != hoveredButtonName)
    {
        // Rrvert the PREVIOUS button texture to normal
        if (!hoveredButtonName.empty())
        {
            SetButtonTexture(hoveredButtonName, false);
        }

        if (!currentHover.empty())
        {
            SetButtonTexture(currentHover, true);
            // Play hover sound
            AudioManager* audioManager = CORE->GetAudioManager();
            if (audioManager)
            {
                audioManager->TriggerEvent("UI_Hover");
            }
        }

        // Update tracked state
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
            std::cout << "[PauseMenu] Button click sound triggered" << std::endl;
        }

        // start delay before executing action
        pendingSceneChange = true;
        pendingTimer = 0.20f; // 200ms delay
    }
}

void PauseMenuLogicSystem::CreatePauseMenuUI()
{
    if (hasCreatedUI)
        return;

    auto& factory = GameObjectFactory::Instance();

    // create semi-transparent overlay background
    GameObject* overlay = factory.CreateGameObject("PauseOverlay");
    auto* overlayTransform = overlay->AddComponent<TransformComponent>();
    overlayTransform->posX = 0.0f;
    overlayTransform->posY = 215.0f;
    overlayTransform->scaleX = 1980.0f;
    overlayTransform->scaleY = 942.0f;
    auto* overlayRender = overlay->AddComponent<RenderComponent>();
    overlayRender->SetColor(vec4(0.0f, 0.0f, 0.0f, 0.7f)); // dark semi-transparent
    overlayRender->SetLayer(100);

    // pause title text
    GameObject* title = factory.CreateGameObject("PauseTitle");
    auto* titleTransform = title->AddComponent<TransformComponent>();
    titleTransform->posX = 0.0f;
    titleTransform->posY = 300.0f;
    titleTransform->scaleX = 400.0f;
    titleTransform->scaleY = 100.0f;
    auto* titleText = title->AddComponent<TextComponent>("../Assets/FrederickatheGreat-Regular.ttf", 48, "PAUSED", vec4(1.0f, 1.0f, 1.0f, 1.0f), true);

    // resume button
    GameObject* resumeBtn = factory.CreateGameObject("PauseResumeButton");
    auto* resumeTransform = resumeBtn->AddComponent<TransformComponent>();
    resumeTransform->posX = 0.0f;
    resumeTransform->posY = 100.0f;
    resumeTransform->scaleX = 300.0f;
    resumeTransform->scaleY = 80.0f;
    auto* resumeRender = resumeBtn->AddComponent<RenderComponent>();
    resumeRender->SetColor(vec4(0.2f, 0.6f, 0.2f, 1.0f)); // Green
    resumeRender->SetLayer(101);
    auto* resumeText = resumeBtn->AddComponent<TextComponent>(
        "../Assets/FrederickatheGreat-Regular.ttf",
        32,
        "Resume",
        vec4(1.0f, 1.0f, 1.0f, 1.0f),
        true);

    // restart button
    GameObject* restartBtn = factory.CreateGameObject("PauseRestartButton");
    auto* restartTransform = restartBtn->AddComponent<TransformComponent>();
    restartTransform->posX = 0.0f;
    restartTransform->posY = 0.0f;
    restartTransform->scaleX = 300.0f;
    restartTransform->scaleY = 80.0f;
    auto* restartRender = restartBtn->AddComponent<RenderComponent>();
    restartRender->SetColor(vec4(0.6f, 0.6f, 0.2f, 1.0f)); // Yellow
    restartRender->SetLayer(101);
    auto* restartText = restartBtn->AddComponent<TextComponent>(
        "../Assets/FrederickatheGreat-Regular.ttf",
        32,
        "Restart",
        vec4(1.0f, 1.0f, 1.0f, 1.0f),
        true);

    // settings button
    GameObject* settingsBtn = factory.CreateGameObject("PauseSettingsButton");
    auto* settingsTransform = settingsBtn->AddComponent<TransformComponent>();
    settingsTransform->posX = 0.0f;
    settingsTransform->posY = -100.0f;
    settingsTransform->scaleX = 300.0f;
    settingsTransform->scaleY = 80.0f;
    auto* settingsRender = settingsBtn->AddComponent<RenderComponent>();
    settingsRender->SetColor(vec4(0.4f, 0.4f, 0.6f, 1.0f)); // blue
    settingsRender->SetLayer(101);
    auto* settingsText = settingsBtn->AddComponent<TextComponent>(
        "../Assets/FrederickatheGreat-Regular.ttf",
        32,
        "Settings",
        vec4(1.0f, 1.0f, 1.0f, 1.0f),
        true);

    // main menu button
    GameObject* mainMenuBtn = factory.CreateGameObject("PauseMainMenuButton");
    auto* mainMenuTransform = mainMenuBtn->AddComponent<TransformComponent>();
    mainMenuTransform->posX = 0.0f;
    mainMenuTransform->posY = -200.0f;
    mainMenuTransform->scaleX = 300.0f;
    mainMenuTransform->scaleY = 80.0f;
    auto* mainMenuRender = mainMenuBtn->AddComponent<RenderComponent>();
    mainMenuRender->SetColor(vec4(0.6f, 0.2f, 0.2f, 1.0f)); // red
    mainMenuRender->SetLayer(101);
    auto* mainMenuText = mainMenuBtn->AddComponent<TextComponent>(
        "../Assets/FrederickatheGreat-Regular.ttf",
        32,
        "Main Menu",
        vec4(1.0f, 1.0f, 1.0f, 1.0f),
        true);

    hasCreatedUI = true;
    std::cout << "[PauseMenu] UI created" << std::endl;

    // Initially hide the UI
    SetPauseMenuVisible(false);
}

void PauseMenuLogicSystem::DestroyPauseMenuUI()
{
    if (!hasCreatedUI)
        return;

    auto& factory = GameObjectFactory::Instance();

    // Destroy all pause menu UI elements
    std::vector<std::string> uiElements = {
        "PauseOverlay",
        "PauseTitle",
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

void PauseMenuLogicSystem::SetPauseMenuVisible(bool show)
{
    auto& factory = GameObjectFactory::Instance();

    std::vector<std::string> uiElements = {
        "PauseOverlay",
        "PauseTitle",
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
}

void PauseMenuLogicSystem::TogglePauseMenu()
{
    // create UI on first toggle if not created
    if (!hasCreatedUI)
    {
        CreatePauseMenuUI();
    }

    isPauseMenuActive = !isPauseMenuActive;
    SetPauseMenuVisible(isPauseMenuActive);

    // Pause/unpause the game
    CORE->SetGamePaused(isPauseMenuActive);
    //CORE->GameTimerSystem();

    std::cout << "[PauseMenu] Pause menu " << (isPauseMenuActive ? "opened" : "closed") << std::endl;
}

bool PauseMenuLogicSystem::IsMouseOver(GameObject* button, double mouseX, double mouseY)
{
    if (!button) return false;
    if (!button->IsActive()) return false; // Don't interact with inactive buttons

    auto* tx = button->GetComponent<TransformComponent>();
    if (!tx) return false;

    return IsAreaClicked(tx->posX, tx->posY, tx->scaleX, tx->scaleY, mouseX, mouseY);
}

void PauseMenuLogicSystem::SetButtonTexture(std::string buttonName, bool isHovered)
{
    auto& factory = GameObjectFactory::Instance();
    GameObject* btn = factory.FindGameObject(buttonName);
    if (!btn) return;

    auto* rc = btn->GetComponent<RenderComponent>();
    if (!rc) return;

    // Change button brightness on hover
    if (buttonName == "PauseResumeButton")
    {
        if (isHovered)
            rc->SetColor(vec4(0.3f, 0.8f, 0.3f, 1.0f)); // Brighter green
        else
            rc->SetColor(vec4(0.2f, 0.6f, 0.2f, 1.0f)); // Normal green
    }
    else if (buttonName == "PauseRestartButton")
    {
        if (isHovered)
            rc->SetColor(vec4(0.8f, 0.8f, 0.3f, 1.0f)); // brighter yellow
        else
            rc->SetColor(vec4(0.6f, 0.6f, 0.2f, 1.0f)); // Normal yellow
    }
    else if (buttonName == "PauseSettingsButton")
    {
        if (isHovered)
            rc->SetColor(vec4(0.6f, 0.6f, 0.8f, 1.0f)); // brighter blue
        else
            rc->SetColor(vec4(0.4f, 0.4f, 0.6f, 1.0f)); // blue
    }
    else if (buttonName == "PauseMainMenuButton")
    {
        if (isHovered)
            rc->SetColor(vec4(0.8f, 0.3f, 0.3f, 1.0f)); // brighter red
        else
            rc->SetColor(vec4(0.6f, 0.2f, 0.2f, 1.0f)); // red
    }
}
