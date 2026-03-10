/* Start Header ************************************************************************/
/*!
\file       WinLoseLogic.cpp
\author     Leck Xin Yee [100% - 594 lines]
\par        xinyee.leck@digipen.edu
\date       November, 29, 2025

Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*/
/* End Header **************************************************************************/

#include "WinLoseLogic.h"
#include "GhostSystem.h"
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
    isPlayingCredits = false;
    m_playerInSafeZone = true;
    m_recordingActive  = false;
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
        isPlayingCredits = false;
        m_playerInSafeZone = true;
        m_recordingActive  = false;
    }

    totalFrames++;

    // Update audio fade
    AudioManager* audioManager = CORE->GetAudioManager();
    if (audioManager)
    {
        audioManager->UpdateFade(dt);
    }

    // Check if credits are playing and have finished
    if (isPlayingCredits)
    {
        CutsceneSystem* cutscene = CORE->GetCutsceneSystem();
        if (cutscene && !cutscene->IsPlaying())
        {
            std::cout << "[WinZone] Credits finished - Loading main menu" << std::endl;
            isPlayingCredits = false;
            CORE->RequestSceneLoad("../Assets/Scenes/MainMenu.json", true);

            hasPlayerWon = false;
            isWaitingForWin = false;
            hasCreatedWinScreen = false;
            hasCreatedLoseScreen = false;
            winDelayTimer = 0.0f;
        }
        return;
    }

    if (pendingSceneChange)
    {
        pendingSceneTimer -= dt;
        if (pendingSceneTimer <= 0.0f)
        {
            // Check if fade is still active - if so, wait a bit longer
            AudioManager* audioManager = CORE->GetAudioManager();
            if (audioManager && audioManager->IsAnyFadeActive())
            {
                std::cout << "[WinLoseLogic] Fade still active, waiting..." << std::endl;
                pendingSceneTimer = 0.1f; // Wait another 100ms and check again
                return;
            }

            pendingSceneChange = false;
            std::cout << "[WinLoseLogic] Fade complete! Loading scene..." << std::endl;
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

    CheckSafeZoneExit();
    CheckWinZoneCollision();
}

/**
 * @brief Detects when the player leaves the SafeZone and starts the ghost recording.
 *        Recording begins the first time the player exits the SafeZone AABB.
 */
void WinLoseLogicSystem::CheckSafeZoneExit()
{
    // Only need to check while the player is still considered to be in the safe zone
    if (!m_playerInSafeZone)
        return;

    auto& factory = GameObjectFactory::Instance();
    GameObject* safeZone = factory.FindGameObject("SafeZone");
    GameObject* player   = factory.FindGameObject("Hero");

    if (!safeZone || !player || !safeZone->IsActive() || !player->IsActive())
        return;

    auto* szCollider = safeZone->GetComponent<ColliderComponent>();
    auto* plCollider = player->GetComponent<ColliderComponent>();

    if (!szCollider || !plCollider)
        return;

    AABB szAABB = szCollider->GetAABB();
    AABB plAABB = plCollider->GetAABB();

    float szCX = (szAABB.min.x + szAABB.max.x) / 2.0f;
    float szCY = (szAABB.min.y + szAABB.max.y) / 2.0f;
    float plCX = (plAABB.min.x + plAABB.max.x) / 2.0f;
    float plCY = (plAABB.min.y + plAABB.max.y) / 2.0f;

    float halfW = (szAABB.max.x - szAABB.min.x) / 2.0f + (plAABB.max.x - plAABB.min.x) / 2.0f;
    float halfH = (szAABB.max.y - szAABB.min.y) / 2.0f + (plAABB.max.y - plAABB.min.y) / 2.0f;

    bool overlapping = (std::abs(plCX - szCX) < halfW) && (std::abs(plCY - szCY) < halfH);

    if (!overlapping)
    {
        // Player has left the SafeZone — start recording
        m_playerInSafeZone = false;
        m_recordingActive  = true;

        GhostSystem* ghost = CORE ? CORE->GetGhostSystem() : nullptr;
        if (ghost)
        {
            ghost->StartRecording("Hero");
            std::cout << "[WinLoseLogic] Player left SafeZone — recording started.\n";
        }
    }
}

/**
 * @brief Stops and discards any in-progress recording (player lost or restarted).
 */
void WinLoseLogicSystem::AbandonRecording()
{
    if (!m_recordingActive)
        return;

    GhostSystem* ghost = CORE ? CORE->GetGhostSystem() : nullptr;
    if (ghost)
    {
        ghost->StopRecording();
        ghost->ResetRecording();
    }

    m_recordingActive  = false;
    m_playerInSafeZone = true;  // next load starts fresh
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

            // Stop recording and save if a run was being recorded
            if (m_recordingActive)
            {
                GhostSystem* ghost = CORE ? CORE->GetGhostSystem() : nullptr;
                if (ghost)
                {
                    ghost->StopRecording();
                    ghost->SaveCurrentRun("Level1_testing");
                    std::cout << "[WinLoseLogic] Recording stopped and saved as Level1_testing.json\n";
                }
                m_recordingActive = false;
            }

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

    // Hide timer UI and pause button
    GameObject* timerBg = factory.FindGameObject("LevelTimerBarBackground");
    GameObject* timerBar = factory.FindGameObject("LevelTimerBar");
    GameObject* timerFrame = factory.FindGameObject("LevelTimerBarFrame");
    GameObject* pauseBtn = factory.FindGameObject("InGamePauseButton");

    if (timerBg) timerBg->SetActive(false);
    if (timerBar) timerBar->SetActive(false);
    if (timerFrame) timerFrame->SetActive(false);
    if (pauseBtn) pauseBtn->SetActive(false);

    // stop all music and play victory sound with fade in
    AudioManager* audioManager = CORE->GetAudioManager();
    if (audioManager)
    {
        audioManager->StopSound("bgm_channel");
        audioManager->SetCategoryVolume("music", 0.0f); // Start at 0
        audioManager->PlaySound("victory");
        audioManager->FadeInCategory("music", 2.0f); // Fade in over 2 second
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
            std::cout << "[WinZone] Next clicked - Playing credits" << std::endl;
            if (audioManager)
            {
                audioManager->TriggerEvent("UI_Press");
                // Fade out music before credits
                audioManager->FadeOutCategory("music", 1.5f);
                std::cout << "[WinZone] Started music fade out (1.5 seconds)" << std::endl;
            }

            // Play credits cutscene with audio
            CutsceneSystem* cutscene = CORE->GetCutsceneSystem();
            if (cutscene && !cutscene->IsPlaying()) {
                cutscene->PlayVideo("../Assets/Credits.mpeg", "CreditsAudio");
                isPlayingCredits = true;
            }

            DestroyWinScreen();
            // Don't unpause - let the cutscene system handle game pause state
        }
        else if (hoveredButtonName == "ReplayButton")
        {
            std::cout << "[WinZone] Replay clicked - Restarting level" << std::endl;

            AbandonRecording();

            if (audioManager)
            {
                audioManager->TriggerEvent("UI_Restart");
                // Fade out music before restarting
                audioManager->FadeOutCategory("music", 1.5f);
                std::cout << "[WinZone] Started music fade out (1.5 seconds)" << std::endl;
            }

            DestroyWinScreen();
            CORE->SetGamePaused(false);

            pendingSceneTimer = 1.7f;  // Wait for 1.5s fade + small buffer
            pendingScenePath = currentScenePath;
            pendingSceneChange = true;
        }
        else if (hoveredButtonName == "QuitButton")
        {
            std::cout << "[WinZone] Quit clicked - Going to main menu" << std::endl;

            if (audioManager)
            {
                audioManager->TriggerEvent("UI_Quit");
                // Fade out music before going to main menu
                audioManager->FadeOutCategory("music", 2.0f);
                std::cout << "[WinZone] Started music fade out (2.0 seconds)" << std::endl;
            }

            DestroyWinScreen();
            pendingSceneTimer = 2.2f;  // Wait for 2.0s fade + small buffer
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

    // Re-show timer UI and pause button
    GameObject* timerBg = factory.FindGameObject("LevelTimerBarBackground");
    GameObject* timerBar = factory.FindGameObject("LevelTimerBar");
    GameObject* timerFrame = factory.FindGameObject("LevelTimerBarFrame");
    GameObject* pauseBtn = factory.FindGameObject("InGamePauseButton");

    if (timerBg) timerBg->SetActive(true);
    if (timerBar) timerBar->SetActive(true);
    if (timerFrame) timerFrame->SetActive(true);
    if (pauseBtn) pauseBtn->SetActive(true);

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

    AbandonRecording();

    auto& factory = GameObjectFactory::Instance();
    vec2 camPos = CameraManager::Instance().GetCameraPosition();

    // Use a scale multiplier to match pause menu UI scaling
    float uiScale = 2.0f;

    // pause game first
    CORE->SetGamePaused(true);

    // Hide timer UI and pause button
    GameObject* timerBg = factory.FindGameObject("LevelTimerBarBackground");
    GameObject* timerBar = factory.FindGameObject("LevelTimerBar");
    GameObject* timerFrame = factory.FindGameObject("LevelTimerBarFrame");
    GameObject* pauseBtn = factory.FindGameObject("InGamePauseButton");

    if (timerBg) timerBg->SetActive(false);
    if (timerBar) timerBar->SetActive(false);
    if (timerFrame) timerFrame->SetActive(false);
    if (pauseBtn) pauseBtn->SetActive(false);

    // stop all music and play game over sound with fade in
    AudioManager* audioManager = CORE->GetAudioManager();
    if (audioManager)
    {
        audioManager->StopSound("bgm_channel");
        audioManager->SetCategoryVolume("music", 0.0f); // Start at 0
        audioManager->PlaySound("gameover");
        audioManager->FadeInCategory("music", 2.0f); // Fade in over 1 second
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

            AbandonRecording();

            if (audioManager)
            {
                audioManager->TriggerEvent("UI_Click");
                // Fade out music before restarting
                audioManager->FadeOutCategory("music", 1.5f);
                std::cout << "[WinLoseLogic] Started music fade out (1.5 seconds)" << std::endl;
            }

            DestroyLoseScreen();
            CORE->SetGamePaused(false);

            pendingSceneTimer = 1.7f;  // Wait for 1.5s fade + small buffer
            pendingScenePath = currentScenePath;
            pendingSceneChange = true;
        }
        else if (hoveredButtonName == "LoseQuitButton")
        {
            std::cout << "[WinLoseLogic] Quit clicked - Going to main menu" << std::endl;

            if (audioManager)
            {
                audioManager->TriggerEvent("UI_Click");
                // Fade out music before going to main menu
                audioManager->FadeOutCategory("music", 2.0f);
                std::cout << "[WinLoseLogic] Started music fade out (2.0 seconds)" << std::endl;
            }

            DestroyLoseScreen();
            pendingSceneTimer = 2.2f;  // Wait for 2.0s fade + small buffer
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

    // Re-show timer UI and pause button
    GameObject* timerBg = factory.FindGameObject("LevelTimerBarBackground");
    GameObject* timerBar = factory.FindGameObject("LevelTimerBar");
    GameObject* timerFrame = factory.FindGameObject("LevelTimerBarFrame");
    GameObject* pauseBtn = factory.FindGameObject("InGamePauseButton");

    if (timerBg) timerBg->SetActive(true);
    if (timerBar) timerBar->SetActive(true);
    if (timerFrame) timerFrame->SetActive(true);
    if (pauseBtn) pauseBtn->SetActive(true);

    hasCreatedLoseScreen = false;
    hoveredButtonName.clear();

    std::cout << "[WinLoseLogic] Lose screen destroyed and state reset" << std::endl;
}
