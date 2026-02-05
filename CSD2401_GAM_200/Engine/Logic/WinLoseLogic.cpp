/* Start Header ************************************************************************/
/*!
\file       WinLoseLogic.cpp
\author     Leck Xin Yee [100% - 553 lines]
\par        xinyee.leck@digipen.edu
\date       November, 29, 2025

Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*/
/* End Header **************************************************************************/

#include "WinLoseLogic.h"
#include "LevelEditor.h"
#include "CameraManager.h"
#include "InputManager.h"
#include "AudioManager.h"
#include "GraphicsSystem.h"
#include "MathLib.h"
#include "ColliderComponent.h"
#include "Collision.h"
#include <iostream>

/**
 * @brief Initializes all win/lose state variables.
 */
void WinLoseLogicSystem::Initialize()
{
    hasPlayerWon = false;
    winDelayTimer = 0.0f;
    isWaitingForWin = false;
    hasCreatedWinScreen = false;
    hasCreatedLoseScreen = false;
    pendingSceneChange = false;
    pendingSceneTimer = 0.0f;
    pendingScenePath = "";
}

/**
 * @brief Updates the win/lose logic, UI screens, and button interactions.
 */
void WinLoseLogicSystem::Update(float dt)
{
    static int totalFrames = 0;
    static bool firstUpdate = true;

    if (firstUpdate)
    {
        firstUpdate = false;
        hasPlayerWon = false;
        isWaitingForWin = false;
        hasCreatedWinScreen = false;
        hasCreatedLoseScreen = false;
        winDelayTimer = 0.0f;
        pendingSceneChange = false;
    }

    totalFrames++;

    // Update audio fade
    AudioManager* audioManager = CORE->GetAudioManager();
    if (audioManager)
    {
        audioManager->UpdateFade(dt);
    }

    if (pendingSceneChange)
    {
        pendingSceneTimer -= dt;
        if (pendingSceneTimer <= 0.0f)
        {
            pendingSceneChange = false;
            // unpause right before loading to avoid showing game world
            CORE->SetGamePaused(false);
            CORE->RequestSceneLoad(pendingScenePath, true);
            pendingScenePath = "";

            hasPlayerWon = false;
            isWaitingForWin = false;
            hasCreatedWinScreen = false;
            hasCreatedLoseScreen = false;
            winDelayTimer = 0.0f;
        }
        return;
    }

    // check if in play mode
    LevelEditor* editor = CORE ? CORE->GetLevelEditor() : nullptr;
    if (!editor)
    {
        return;
    }

    EditorMode mode = editor->GetEditorMode();
    if (mode != EditorMode::Play)
    {
        return;
    }

    if (isWaitingForWin)
    {
        winDelayTimer -= dt;
        if (winDelayTimer <= 0.0f)
        {
            TriggerWin();
            isWaitingForWin = false;
        }
        return;
    }

    // update win screen position to follow camera
    if (hasCreatedWinScreen)
    {
        vec2 camPos = CameraManager::Instance().GetCameraPosition();
        auto& factory = GameObjectFactory::Instance();

        // use a scale multiplier to match pause menu UI scaling
        float uiScale = 2.0f;

        // update all win screen elements to follow camera
        std::vector<std::pair<std::string, vec2>> uiElements = {
            {"WinScreen", vec2(0.0f, 0.0f)},
            {"NextLevelLogo", vec2(550.0f * uiScale, 250.0f * uiScale)},
            {"NextButton", vec2(550.0f * uiScale, 50.0f * uiScale)},
            {"ReplayButton", vec2(550.0f * uiScale, -100.0f * uiScale)},
            {"QuitButton", vec2(550.0f * uiScale, -250.0f * uiScale)}
        };

        for (const auto& element : uiElements)
        {
            GameObject* obj = factory.FindGameObject(element.first);
            if (obj)
            {
                auto* transform = obj->GetComponent<TransformComponent>();
                if (transform)
                {
                    transform->posX = camPos.x + element.second.x;
                    transform->posY = camPos.y + element.second.y;
                }
            }
        }

        HandleWinScreenButtons();
        return;
    }

    // update lose screen position to follow camera
    if (hasCreatedLoseScreen)
    {
        vec2 camPos = CameraManager::Instance().GetCameraPosition();
        auto& factory = GameObjectFactory::Instance();

        float uiScale = 2.0f;

        // update all lose screen elements to follow camera
        std::vector<std::pair<std::string, vec2>> uiElements = {
            {"LoseScreen", vec2(0.0f, 0.0f)},
            {"LoseReplayLogo", vec2(550.0f * uiScale, 125.0f * uiScale)},
            {"YesButton", vec2(550.0f * uiScale, -50.0f * uiScale)},
            {"LoseQuitButton", vec2(550.0f * uiScale, -200.0f * uiScale)}
        };

        for (const auto& element : uiElements)
        {
            GameObject* obj = factory.FindGameObject(element.first);
            if (obj)
            {
                auto* transform = obj->GetComponent<TransformComponent>();
                if (transform)
                {
                    transform->posX = camPos.x + element.second.x;
                    transform->posY = camPos.y + element.second.y;
                }
            }
        }

        HandleLoseScreenButtons();
        return;
    }

    if (hasPlayerWon)
    {
        return;
    }

    CheckWinZoneCollision();
}

/**
 * @brief Checks for WinZone collision using AABB every frame
 */
void WinLoseLogicSystem::CheckWinZoneCollision()
{
    auto& factory = GameObjectFactory::Instance();
    GameObject* winZone = factory.FindGameObject("WinZone");
    GameObject* player = factory.FindGameObject("Hero");

    if (!winZone || !player || !winZone->IsActive() || !player->IsActive())
    {
        return;
    }

    auto* winZoneTransform = winZone->GetComponent<TransformComponent>();
    auto* playerTransform = player->GetComponent<TransformComponent>();
    auto* winZoneCollider = winZone->GetComponent<ColliderComponent>();
    auto* playerCollider = player->GetComponent<ColliderComponent>();

    if (!winZoneTransform || !playerTransform || !winZoneCollider || !playerCollider)
    {
        return;
    }

    // Use the ColliderComponent's AABB for accurate collision detection
    AABB winAABB = winZoneCollider->GetAABB();
    AABB playerAABB = playerCollider->GetAABB();

    // Get center positions
    float winX = (winAABB.min.x + winAABB.max.x) / 2.0f;
    float winY = (winAABB.min.y + winAABB.max.y) / 2.0f;
    float playerX = (playerAABB.min.x + playerAABB.max.x) / 2.0f;
    float playerY = (playerAABB.min.y + playerAABB.max.y) / 2.0f;

    // Get half dimensions from AABB
    float winHalfWidth = (winAABB.max.x - winAABB.min.x) / 2.0f;
    float winHalfHeight = (winAABB.max.y - winAABB.min.y) / 2.0f;
    float playerHalfWidth = (playerAABB.max.x - playerAABB.min.x) / 2.0f;
    float playerHalfHeight = (playerAABB.max.y - playerAABB.min.y) / 2.0f;

    float distX = std::abs(playerX - winX);
    float distY = std::abs(playerY - winY);
    float thresholdX = playerHalfWidth + winHalfWidth;
    float thresholdY = playerHalfHeight + winHalfHeight;

    bool colliding = (distX < thresholdX) && (distY < thresholdY);

    static bool lastFrameColliding = false;

    if (colliding && !lastFrameColliding)
    {
        if (!hasPlayerWon && !isWaitingForWin)
        {
            std::cout << "[WinZone] Player reached the win zone!" << std::endl;
            hasPlayerWon = true;
            isWaitingForWin = true;
            winDelayTimer = 0.5f;
        }
    }

    lastFrameColliding = colliding;
}

// ============ WIN ============
/**
 * @brief Creates win UI elements and pauses the game.
 */
void WinLoseLogicSystem::TriggerWin()
{
    //std::cout << "you win!!!" << std::endl;
    auto& factory = GameObjectFactory::Instance();
    vec2 camPos = CameraManager::Instance().GetCameraPosition();

    float uiScale = 2.0f;

    // pause game first
    CORE->SetGamePaused(true);

    // stop all music and play victory sound
    AudioManager* audioManager = CORE->GetAudioManager();
    if (audioManager)
    {
        audioManager->StopSound("bgm_channel");
        audioManager->PlaySound("victory");
    }

    // win screen background
    GameObject* board = factory.CreateGameObject("WinScreen");
    auto* boardTransform = board->AddComponent<TransformComponent>();
    boardTransform->posX = camPos.x + 0.0f;
    boardTransform->posY = camPos.y + 0.0f;
    boardTransform->scaleX = 1920.0f * uiScale;
    boardTransform->scaleY = 1080.0f * uiScale;
    auto* boardRender = board->AddComponent<RenderComponent>();
    boardRender->SetTextureID(GraphicsSystem::LoadTexture("../Assets/Sprites/Game_win_screen.png"));
    boardRender->SetColor(vec4(1.0f, 1.0f, 1.0f, 1.0f));
    boardRender->SetLayer(101);

    // next level logo
    GameObject* bigLogo = factory.CreateGameObject("NextLevelLogo");
    auto* logoTransform = bigLogo->AddComponent<TransformComponent>();
    logoTransform->posX = camPos.x + 550.0f * uiScale;
    logoTransform->posY = camPos.y + 250.0f * uiScale;
    logoTransform->scaleX = 600.0f * uiScale;
    logoTransform->scaleY = 175.0f * uiScale;
    auto* logoRender = bigLogo->AddComponent<RenderComponent>();
    logoRender->SetTextureID(GraphicsSystem::LoadTexture("../Assets/Sprites/Icons/win_lose_bracket.png"));
    logoRender->SetColor(vec4(1.0f, 1.0f, 1.0f, 1.0f));
    logoRender->SetLayer(102);
    bigLogo->AddComponent<TextComponent>(
        "../Assets/FrederickatheGreat-Regular.ttf", static_cast<int>(72 * uiScale), "Next Level?", vec4(0.0f, 0.0f, 0.0f, 1.0f), false);

    // next button which goes to next level
    GameObject* nextBtn = factory.CreateGameObject("NextButton");
    auto* nextTransform = nextBtn->AddComponent<TransformComponent>();
    nextTransform->posX = camPos.x + 550.0f * uiScale;
    nextTransform->posY = camPos.y + 50.0f * uiScale;
    nextTransform->scaleX = 550.0f * uiScale;
    nextTransform->scaleY = 140.0f * uiScale;
    auto* nextRender = nextBtn->AddComponent<RenderComponent>();
    nextRender->SetTextureID(GraphicsSystem::LoadTexture("../Assets/Sprites/Icons/win_lose_button_dark.png"));
    nextRender->SetColor(vec4(1.0f, 1.0f, 1.0f, 1.0f));
    nextRender->SetLayer(102);
    nextBtn->AddComponent<TextComponent>(
        "../Assets/FrederickatheGreat-Regular.ttf", static_cast<int>(50 * uiScale), "Next", vec4(0.0f, 0.0f, 0.0f, 1.0f), false);

    // replay button which restarts level
    GameObject* replayBtn = factory.CreateGameObject("ReplayButton");
    auto* replayTransform = replayBtn->AddComponent<TransformComponent>();
    replayTransform->posX = camPos.x + 550.0f * uiScale;
    replayTransform->posY = camPos.y + -100.0f * uiScale;
    replayTransform->scaleX = 550.0f * uiScale;
    replayTransform->scaleY = 140.0f * uiScale;
    auto* replayRender = replayBtn->AddComponent<RenderComponent>();
    replayRender->SetTextureID(GraphicsSystem::LoadTexture("../Assets/Sprites/Icons/win_lose_button_dark.png"));
    replayRender->SetColor(vec4(1.0f, 1.0f, 1.0f, 1.0f));
    replayRender->SetLayer(102);
    replayBtn->AddComponent<TextComponent>(
        "../Assets/FrederickatheGreat-Regular.ttf", static_cast<int>(50 * uiScale), "Replay", vec4(0.0f, 0.0f, 0.0f, 1.0f), false);

    // quit button which goes bk to main menu
    GameObject* quitBtn = factory.CreateGameObject("QuitButton");
    auto* quitTransform = quitBtn->AddComponent<TransformComponent>();
    quitTransform->posX = camPos.x + 550.0f * uiScale;
    quitTransform->posY = camPos.y + -250.0f * uiScale;
    quitTransform->scaleX = 550.0f * uiScale;
    quitTransform->scaleY = 140.0f * uiScale;
    auto* quitRender = quitBtn->AddComponent<RenderComponent>();
    quitRender->SetTextureID(GraphicsSystem::LoadTexture("../Assets/Sprites/Icons/win_lose_button_dark.png"));
    quitRender->SetColor(vec4(1.0f, 1.0f, 1.0f, 1.0f));
    quitRender->SetLayer(102);
    quitBtn->AddComponent<TextComponent>(
        "../Assets/FrederickatheGreat-Regular.ttf", static_cast<int>(50 * uiScale), "Quit", vec4(0.0f, 0.0f, 0.0f, 1.0f), false);

    hasCreatedWinScreen = true;
}

/**
 * @brief Handles hover and click states for win screen buttons.
 */
void WinLoseLogicSystem::HandleWinScreenButtons()
{
    double mouseX = InputManager::GetMouseWorldX();
    double mouseY = InputManager::GetMouseWorldY();
    bool isClicked = InputManager::IsMouseLeftClicked();

    auto& factory = GameObjectFactory::Instance();
    std::string currentHover = "";

    if (IsMouseOver(factory.FindGameObject("NextButton"), mouseX, mouseY))
    {
        currentHover = "NextButton";
    }
    else if (IsMouseOver(factory.FindGameObject("ReplayButton"), mouseX, mouseY))
    {
        currentHover = "ReplayButton";
    }
    else if (IsMouseOver(factory.FindGameObject("QuitButton"), mouseX, mouseY))
    {
        currentHover = "QuitButton";
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

    if (isClicked && !hoveredButtonName.empty() && !pendingSceneChange)
    {
        AudioManager* audioManager = CORE->GetAudioManager();

        if (hoveredButtonName == "NextButton")
        {
            std::cout << "[WinZone] Next clicked - Going to main menu for now (since no next level)" << std::endl;
            if (audioManager)
            {
                audioManager->TriggerEvent("UI_Press");
            }

            if (audioManager)
            {
                audioManager->SetCategoryVolume("music", 1.0f);
            }

            // TODO: Implement next level logic
            DestroyWinScreen();
            CORE->SetGamePaused(false);

            pendingSceneTimer = 0.2f;  // delay for press sound to play
            pendingScenePath = "../Assets/Scenes/MainMenu.json"; // i set this to main menu for now since no next level
            pendingSceneChange = true;
        }
        else if (hoveredButtonName == "ReplayButton")
        {
            std::cout << "[WinZone] Replay clicked - Restarting level" << std::endl;

            if (audioManager)
            {
                audioManager->TriggerEvent("UI_Restart");
            }

            // restore music volume before restarting
            if (audioManager)
            {
                audioManager->SetCategoryVolume("music", 1.0f);
            }

            DestroyWinScreen();
            CORE->SetGamePaused(false);

            pendingSceneTimer = 0.2f;  // Delay for sound to play
            pendingScenePath = currentScenePath;
            pendingSceneChange = true;
        }
        else if (hoveredButtonName == "QuitButton")
        {
            std::cout << "[WinZone] Quit clicked - Going to main menu" << std::endl;

            if (audioManager)
            {
                audioManager->TriggerEvent("UI_Quit");
            }

            // restore music volume before going to main menu
            if (audioManager)
            {
                audioManager->SetCategoryVolume("music", 1.0f);
            }

            DestroyWinScreen();
            pendingSceneTimer = 0.2f;  // Delay for sound to play
            pendingScenePath = "../Assets/Scenes/MainMenu.json";
            pendingSceneChange = true;
        }
    }
}

/**
 * @brief Deletes win UI elements and resets win state.
 */
void WinLoseLogicSystem::DestroyWinScreen()
{
    if (!hasCreatedWinScreen)
        return;

    auto& factory = GameObjectFactory::Instance();

    std::vector<std::string> uiElements = {
        "WinScreen",
        "NextLevelLogo",
        "NextButton",
        "ReplayButton",
        "QuitButton"
    };

    for (const auto& name : uiElements)
    {
        GameObject* obj = factory.FindGameObject(name);
        if (obj)
        {
            factory.DestroyGameObject(obj->GetID());
        }
    }

    hasCreatedWinScreen = false;
    hasPlayerWon = false;
    isWaitingForWin = false;
    hoveredButtonName.clear();
}

/**
 * @brief Checks if mouse cursor is overlapping a UI button.
 */
bool WinLoseLogicSystem::IsMouseOver(GameObject* button, double mouseX, double mouseY)
{
    if (!button) return false;
    if (!button->IsActive()) return false;

    auto* tx = button->GetComponent<TransformComponent>();
    if (!tx) return false;

    return IsAreaClicked(tx->posX, tx->posY, tx->scaleX, tx->scaleY, static_cast<float>(mouseX), static_cast<float>(mouseY));
}

/**
 * @brief Changes button texture depending on hover state.
 */
void WinLoseLogicSystem::SetButtonTexture(std::string buttonName, bool isHovered)
{
    auto& factory = GameObjectFactory::Instance();
    GameObject* btn = factory.FindGameObject(buttonName);
    if (!btn) return;

    auto* rc = btn->GetComponent<RenderComponent>();
    if (!rc) return;

    std::string texturePath;
    if (buttonName == "NextButton" || buttonName == "ReplayButton" || buttonName == "QuitButton" ||
        buttonName == "YesButton" || buttonName == "LoseQuitButton")
    {
        texturePath = isHovered ? "../Assets/Sprites/Icons/win_lose_button.png" : "../Assets/Sprites/Icons/win_lose_button_dark.png";
    }
    else
    {
        return;
    }

    unsigned int newtex = GraphicsSystem::LoadTexture(texturePath.c_str());
    rc->SetTextureID(newtex);
}

// ============ LOSE ============
/**
 * @brief Creates lose UI elements and pauses the game.
 */
void WinLoseLogicSystem::TriggerLose()
{
    std::cout << "[WinLoseLogic] YOU LOSE!" << std::endl;

    auto& factory = GameObjectFactory::Instance();
    vec2 camPos = CameraManager::Instance().GetCameraPosition();

    // Use a scale multiplier to match pause menu UI scaling
    float uiScale = 2.0f;

    // pause game first
    CORE->SetGamePaused(true);

    // stop all music and play game over sound
    AudioManager* audioManager = CORE->GetAudioManager();
    if (audioManager)
    {
        audioManager->StopSound("bgm_channel");
        audioManager->PlaySound("gameover");
    }

    // lose screen background
    GameObject* board = factory.CreateGameObject("LoseScreen");
    auto* boardTransform = board->AddComponent<TransformComponent>();
    boardTransform->posX = camPos.x + 0.0f;
    boardTransform->posY = camPos.y + 0.0f;
    boardTransform->scaleX = 1920.0f * uiScale;
    boardTransform->scaleY = 1080.0f * uiScale;
    auto* boardRender = board->AddComponent<RenderComponent>();
    boardRender->SetTextureID(GraphicsSystem::LoadTexture("../Assets/Sprites/Game_over_screen.png"));
    boardRender->SetColor(vec4(1.0f, 1.0f, 1.0f, 1.0f));
    boardRender->SetLayer(101);

    // retry level logo
    GameObject* loseLogo = factory.CreateGameObject("LoseReplayLogo");
    auto* loseTransform = loseLogo->AddComponent<TransformComponent>();
    loseTransform->posX = camPos.x + 550.0f * uiScale;
    loseTransform->posY = camPos.y + 125.0f * uiScale;
    loseTransform->scaleX = 600.0f * uiScale;
    loseTransform->scaleY = 175.0f * uiScale;
    auto* loseRender = loseLogo->AddComponent<RenderComponent>();
    loseRender->SetTextureID(GraphicsSystem::LoadTexture("../Assets/Sprites/Icons/win_lose_bracket.png"));
    loseRender->SetColor(vec4(1.0f, 1.0f, 1.0f, 1.0f));
    loseRender->SetLayer(102);
    loseLogo->AddComponent<TextComponent>(
        "../Assets/FrederickatheGreat-Regular.ttf", static_cast<int>(72 * uiScale), "Retry Level?", vec4(0.0f, 0.0f, 0.0f, 1.0f), false);

    // yes button which restarts level
    GameObject* yesBtn = factory.CreateGameObject("YesButton");
    auto* yesTransform = yesBtn->AddComponent<TransformComponent>();
    yesTransform->posX = camPos.x + 550.0f * uiScale;
    yesTransform->posY = camPos.y + -50.0f * uiScale;
    yesTransform->scaleX = 550.0f * uiScale;
    yesTransform->scaleY = 140.0f * uiScale;
    auto* yesRender = yesBtn->AddComponent<RenderComponent>();
    yesRender->SetTextureID(GraphicsSystem::LoadTexture("../Assets/Sprites/Icons/win_lose_button_dark.png"));
    yesRender->SetColor(vec4(1.0f, 1.0f, 1.0f, 1.0f));
    yesRender->SetLayer(102);
    yesBtn->AddComponent<TextComponent>(
        "../Assets/FrederickatheGreat-Regular.ttf", static_cast<int>(50 * uiScale), "Yes", vec4(0.0f, 0.0f, 0.0f, 1.0f), false);

    // quit button which goes back to main menu
    GameObject* loseQuitBtn = factory.CreateGameObject("LoseQuitButton");
    auto* loseQuitTransform = loseQuitBtn->AddComponent<TransformComponent>();
    loseQuitTransform->posX = camPos.x + 550.0f * uiScale;
    loseQuitTransform->posY = camPos.y + -200.0f * uiScale;
    loseQuitTransform->scaleX = 550.0f * uiScale;
    loseQuitTransform->scaleY = 140.0f * uiScale;
    auto* loseQuitRender = loseQuitBtn->AddComponent<RenderComponent>();
    loseQuitRender->SetTextureID(GraphicsSystem::LoadTexture("../Assets/Sprites/Icons/win_lose_button_dark.png"));
    loseQuitRender->SetColor(vec4(1.0f, 1.0f, 1.0f, 1.0f));
    loseQuitRender->SetLayer(102);
    loseQuitBtn->AddComponent<TextComponent>(
        "../Assets/FrederickatheGreat-Regular.ttf", static_cast<int>(50 * uiScale), "Quit", vec4(0.0f, 0.0f, 0.0f, 1.0f), false);

    hasCreatedLoseScreen = true;
}

/**
 * @brief Handles hover and click states for lose screen buttons.
 */
void WinLoseLogicSystem::HandleLoseScreenButtons()
{
    double mouseX = InputManager::GetMouseWorldX();
    double mouseY = InputManager::GetMouseWorldY();
    bool isClicked = InputManager::IsMouseLeftClicked();

    auto& factory = GameObjectFactory::Instance();
    std::string currentHover = "";

    if (IsMouseOver(factory.FindGameObject("YesButton"), mouseX, mouseY))
    {
        currentHover = "YesButton";
    }
    else if (IsMouseOver(factory.FindGameObject("LoseQuitButton"), mouseX, mouseY))
    {
        currentHover = "LoseQuitButton";
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

    if (isClicked && !hoveredButtonName.empty() && !pendingSceneChange)
    {
        AudioManager* audioManager = CORE->GetAudioManager();

        if (hoveredButtonName == "YesButton")
        {
            std::cout << "[WinLoseLogic] Yes clicked - Restarting level" << std::endl;

            if (audioManager)
            {
                audioManager->TriggerEvent("UI_Click");
            }

            // Restore music volume before restarting
            if (audioManager)
            {
                audioManager->SetCategoryVolume("music", 1.0f);
            }

            DestroyLoseScreen();
            CORE->SetGamePaused(false);

            pendingSceneTimer = 0.2f;  // Delay for sound to play
            pendingScenePath = currentScenePath;
            pendingSceneChange = true;
        }
        else if (hoveredButtonName == "LoseQuitButton")
        {
            std::cout << "[WinLoseLogic] Quit clicked - Going to main menu" << std::endl;

            if (audioManager)
            {
                audioManager->TriggerEvent("UI_Click");
            }

            // Restore music volume before going to main menu
            if (audioManager)
            {
                audioManager->SetCategoryVolume("music", 1.0f);
            }

            DestroyLoseScreen();
            pendingSceneTimer = 0.2f;  // Delay for sound to play
            pendingScenePath = "../Assets/Scenes/MainMenu.json";
            pendingSceneChange = true;
        }
    }
}

/**
 * @brief Deletes lose UI elements and resets lose state.
 */
void WinLoseLogicSystem::DestroyLoseScreen()
{
    if (!hasCreatedLoseScreen)
        return;

    auto& factory = GameObjectFactory::Instance();

    std::vector<std::string> uiElements = {
        "LoseScreen",
        "LoseReplayLogo",
        "YesButton",
        "LoseQuitButton"
    };

    for (const auto& name : uiElements)
    {
        GameObject* obj = factory.FindGameObject(name);
        if (obj)
        {
            factory.DestroyGameObject(obj->GetID());
        }
    }

    hasCreatedLoseScreen = false;
    hoveredButtonName.clear();

    std::cout << "[WinLoseLogic] Lose screen destroyed and state reset" << std::endl;
}
