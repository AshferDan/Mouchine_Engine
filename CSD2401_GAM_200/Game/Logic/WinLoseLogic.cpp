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
#include "DoorAnimationLogic.h"
#include "GhostSystem.h"
#include "LevelEditor.h"
#include "CameraManager.h"
#include "InputManager.h"
#include "AudioManager.h"
#include "GraphicsSystem.h"
#include "MathLib.h"
#include "ColliderComponent.h"
#include "Collision.h"
#include "TextInputSystem.h"
#include <iostream>
#include <cmath>
#include <map>
#include <algorithm>
#include <random>

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
    m_recordingActive = false;
    m_elapsedTime = 0.0f;
    m_timerRunning = false;

    // Re-park the ghost at frame 0 if it was toggled ON before the restart.
    // The old GhostHero object was destroyed by the scene reload, so we reset
    // the toggle (clears the stale visible state) then re-enable it to recreate
    // and park the ghost.  WinLoseLogic::CheckSafeZoneExit() will start playback
    // when the player leaves the SafeZone.
    GhostSystem* ghost = CORE ? CORE->GetGhostSystem() : nullptr;
    if (ghost)
    {
        bool wasVisible = ghost->IsGhostVisible();
        ghost->ResetGhostToggle(); // marks ghost as hidden (m_ghostVisible = false)
        ghost->StopPlayback();     // stop any lingering playback

        if (wasVisible && ghost->IsGhostAvailable())
        {
            // Re-enable: clones Hero → GhostHero and parks at frame 0
            ghost->ToggleGhostVisible();
            std::cout << "[WinLoseLogic] Ghost re-parked at start after restart.\n";
        }
    }
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
        m_recordingActive = false;
        m_elapsedTime = 0.0f;
        m_timerRunning = false;

        // Configure the engine text-input system with this game's font and UI style
        TextInputSystem* textInput = CORE ? CORE->GetTextInputSystem() : nullptr;
        if (textInput)
        {
            textInput->SetFont("../Assets/FrederickatheGreat-Regular.ttf", 50);

            // Match the win/lose screen visual style
            DialogStyle style;
            style.titleTexPath    = "../Assets/Sprites/Icons/win_lose_bracket.png";
            style.fieldTexPath    = "../Assets/Sprites/Icons/win_lose_button_dark.png";
            style.hintTexPath     = "../Assets/Sprites/Icons/win_lose_button_dark.png";
            style.overlayColor    = { 0.0f,  0.0f,  0.0f,  0.7f  };
            style.titlePanelColor = { 1.0f,  1.0f,  1.0f,  1.0f  };
            style.fieldPanelColor = { 1.0f,  1.0f,  1.0f,  1.0f  };
            style.hintPanelColor  = { 1.0f,  1.0f,  1.0f,  1.0f  };
            style.titleTextColor  = { 0.1f,  0.05f, 0.0f,  1.0f  };
            style.fieldTextColor  = { 0.1f,  0.05f, 0.0f,  1.0f  };
            style.hintTextColor   = { 0.0f,  0.0f,  0.0f,  1.0f  };
            textInput->SetStyle(style);

            textInput->Hide(); // ensure no stale dialog from a previous session
        }

        // Pre-configure ghost system and load best run so key-0 toggle works from frame 1
        GhostSystem* ghost = CORE ? CORE->GetGhostSystem() : nullptr;
        if (ghost)
        {
            // Derive level name from scene path and set it so load/save use the correct subfolder
            std::string lvl = currentScenePath;
            size_t slash = lvl.find_last_of("/\\");
            if (slash != std::string::npos) lvl = lvl.substr(slash + 1);
            size_t dot = lvl.rfind('.');
            if (dot != std::string::npos) lvl = lvl.substr(0, dot);
            ghost->SetLevelName(lvl);

            ghost->Configure("Hero", "GhostHero", "Hero_vfx", "GhostHero_vfx");
            ghost->LoadRecording("Level1_testing");
            ghost->ResetGhostToggle();  // ghost is hidden by default each session
        }
    }

    totalFrames++;

    // Tick elapsed level timer (starts when player leaves SafeZone, stops on win/lose)
    if (m_timerRunning)
    {
        m_elapsedTime += dt;
    }

    // Update audio fade
    AudioManager* audioManager = CORE->GetAudioManager();
    if (audioManager)
    {
        audioManager->UpdateFade(dt);
    }

    // Delayed victory BGM — starts counting down once TriggerWin fires,
    // plays when the timer expires (after the doors have finished opening).
    if (m_winBGMDelayTimer > 0.0f)
    {
        m_winBGMDelayTimer -= dt;
        if (m_winBGMDelayTimer <= 0.0f)
        {
            m_winBGMDelayTimer = -1.0f;
            if (audioManager)
            {
                audioManager->PlaySound("victory");
                audioManager->FadeInCategory("music", 2.0f);
            }
        }
    }

    // Check if credits are playing and have finished
    if (isPlayingCredits)
    {
        CutsceneSystem* cutscene = CORE->GetCutsceneSystem();
        if (cutscene && !cutscene->IsPlaying())
        {
            //std::cout << "[WinZone] Credits finished - Loading main menu" << std::endl;
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
            //AudioManager* audioManager = CORE->GetAudioManager();
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

    // Cheat codes
    if (!hasPlayerWon && !hasCreatedWinScreen && !hasCreatedLoseScreen)
    {
        InputManager* cheatInput = CORE->GetInputManager();
        if (cheatInput)
        {
            bool ctrl = (cheatInput->IsKeyHeld(GLFW_KEY_LEFT_CONTROL) || cheatInput->IsKeyPressed(GLFW_KEY_LEFT_CONTROL)) ||
                        (cheatInput->IsKeyHeld(GLFW_KEY_RIGHT_CONTROL) || cheatInput->IsKeyPressed(GLFW_KEY_RIGHT_CONTROL));
            if (ctrl && cheatInput->IsKeyPressed(GLFW_KEY_1))
            {
                // CTRL+1: Win current level instantly
                TriggerWin();
            }
            else if (ctrl && cheatInput->IsKeyPressed(GLFW_KEY_2))
            {
                // CTRL+2: Trigger lose/respawn instantly
                TriggerLose();
            }
            else if (ctrl && cheatInput->IsKeyPressed(GLFW_KEY_3))
            {
                // CTRL+3: Skip to next level
                std::string nextScene;
                if (currentScenePath.find("Level_1") != std::string::npos)
                    nextScene = "../Assets/Scenes/Level_2.json";
                else
                    std::cout << "[Cheat] No next level to skip to.\n";

                if (!nextScene.empty())
                    CORE->RequestSceneLoad(nextScene, true);
            }
            else if (ctrl && cheatInput->IsKeyPressed(GLFW_KEY_4))
            {
                // CTRL+4: Reload current level instantly
                if (!currentScenePath.empty())
                    CORE->RequestSceneLoad(currentScenePath, true);
            }
        }
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
            {"WinScreen",       vec2(0.0f, 0.0f)},
            {"WinTitleBracket", vec2(550.0f * uiScale,  350.0f * uiScale)},
            {"WinLevelNumber",  vec2(550.0f * uiScale,  380.0f * uiScale)},
            {"WinLevelName",    vec2(550.0f * uiScale,  310.0f * uiScale)},
            {"WinClockIcon",    vec2((550.0f - 180.0f) * uiScale,  180.0f * uiScale)},
            {"WinTimeLabel",    vec2((550.0f - 70.0f) * uiScale,   180.0f * uiScale)},
            {"WinTimeText",     vec2((550.0f + 130.0f) * uiScale,  180.0f * uiScale)},
            {"WinCoinIcon",     vec2((550.0f - 180.0f) * uiScale,  70.0f * uiScale)},
            {"WinCoinText",     vec2((550.0f - 100.0f) * uiScale,  70.0f * uiScale)},
            {"WinShellIcon",    vec2((550.0f + 60.0f) * uiScale,  70.0f * uiScale)},
            {"WinShellText",    vec2((550.0f + 155.0f) * uiScale,  70.0f * uiScale)},
            {"WinSeparator1",   vec2(550.0f * uiScale,  120.0f * uiScale)},
            {"WinSeparator2",   vec2(550.0f * uiScale,   10.0f * uiScale)},
            {"WinRatingLabel",  vec2((550.0f - 80.0f) * uiScale,   -50.0f * uiScale)},
            {"WinRatingBadge",  vec2((550.0f + 80.0f) * uiScale,  -50.0f * uiScale)},
            {"WinSignHere",     vec2(550.0f * uiScale, -220.0f * uiScale)},
            {"WinSignLabel",    vec2(550.0f * uiScale, -270.0f * uiScale)},
            {"QuitButton",      vec2(550.0f * uiScale, -370.0f * uiScale)}
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

        std::vector<std::pair<std::string, vec2>> uiElements = {
            {"LoseScreen",       vec2(0.0f, 0.0f)},
            {"LoseTitleBracket", vec2(550.0f * uiScale,  350.0f * uiScale)},
            {"LoseLevelNumber",  vec2(550.0f * uiScale,  380.0f * uiScale)},
            {"LoseLevelName",    vec2(550.0f * uiScale,  310.0f * uiScale)},
            {"LoseClockIcon",    vec2((550.0f - 180.0f) * uiScale,  180.0f * uiScale)},
            {"LoseTimeLabel",    vec2((550.0f - 70.0f) * uiScale,   180.0f * uiScale)},
            {"LoseTimeText",     vec2((550.0f + 130.0f) * uiScale,  180.0f * uiScale)},
            {"LoseCoinIcon",     vec2((550.0f - 180.0f) * uiScale,  70.0f * uiScale)},
            {"LoseCoinText",     vec2((550.0f - 100.0f) * uiScale,  70.0f * uiScale)},
            {"LoseShellIcon",    vec2((550.0f + 60.0f) * uiScale,  70.0f * uiScale)},
            {"LoseShellText",    vec2((550.0f + 155.0f) * uiScale,  70.0f * uiScale)},
            {"LoseSeparator1",   vec2(550.0f * uiScale,  120.0f * uiScale)},
            {"LoseSeparator2",   vec2(550.0f * uiScale,   10.0f * uiScale)},
            {"LoseRatingLabel",  vec2((550.0f - 80.0f) * uiScale,   -50.0f * uiScale)},
            {"LoseRatingBadge",  vec2((550.0f + 170.0f) * uiScale,  -50.0f * uiScale)},
            {"LoseSignHereCross",vec2(550.0f * uiScale, -160.0f * uiScale)},
            {"LoseSignHere",     vec2(550.0f * uiScale, -220.0f * uiScale)},
            {"LoseSignLabel",    vec2(550.0f * uiScale, -270.0f * uiScale)},
            {"LoseQuitButton",   vec2(550.0f * uiScale, -370.0f * uiScale)}
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
    GameObject* player = factory.FindGameObject("Hero");

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
        m_recordingActive = true;
        m_timerRunning = true;

        GhostSystem* ghost = CORE ? CORE->GetGhostSystem() : nullptr;
        if (ghost)
        {
            ghost->Configure("Hero", "GhostHero", "Hero_vfx", "GhostHero_vfx");
            ghost->StartRecording("Hero");

            // If the ghost is already toggled on, start playback now that the run has begun
            if (ghost->IsGhostAvailable() && ghost->IsGhostVisible())
                ghost->StartPlayback();

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

        // Stop ghost playback and park it at frame 0 so it's ready for the next run
        if (ghost->IsGhostPlaying())
            ghost->StopPlayback();
    }

    m_recordingActive = false;
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

            // Stop recording; player will name the run in the dialog
            if (m_recordingActive)
            {
                GhostSystem* ghost = CORE ? CORE->GetGhostSystem() : nullptr;
                if (ghost)
                {
                    ghost->StopRecording();
                    std::cout << "[WinLoseLogic] Recording stopped — awaiting run name.\n";
                }
                m_recordingActive = false;
            }

            hasPlayerWon = true;

            float score = CalculateRating();

            // Hide timer UI while the dialog is open
            //auto& factory = GameObjectFactory::Instance();
            if (auto* o = factory.FindGameObject("LevelTimerBarBackground")) o->SetActive(false);
            if (auto* o = factory.FindGameObject("LevelTimerBar"))           o->SetActive(false);
            if (auto* o = factory.FindGameObject("LevelTimerBarFrame"))      o->SetActive(false);
            if (auto* o = factory.FindGameObject("InGamePauseButton"))       o->SetActive(false);

            CORE->SetGamePaused(true);

            // Show the engine-level name-input dialog
            TextInputSystem* textInput = CORE ? CORE->GetTextInputSystem() : nullptr;
            if (textInput)
            {
                // Play a random typing sound on each keystroke
                TextInputSystem::SetOnCharTyped([]()
                {
                    static std::mt19937 rng{ std::random_device{}() };
                    static std::uniform_int_distribution<int> dist(1, 6);
                    if (auto* audio = CORE ? CORE->GetAudioManager() : nullptr)
                        audio->PlaySound("typing_sfx_" + std::to_string(dist(rng)));
                });

                textInput->Show(
                    "Name Your Run!", 20,
                    // --- onConfirm ---
                    [this, score](const std::string& rawName)
                    {
                        // Sanitise: trim whitespace, replace spaces with underscores
                        std::string runName = rawName;
                        while (!runName.empty() && runName.front() == ' ')
                            runName.erase(runName.begin());
                        while (!runName.empty() && runName.back() == ' ')
                            runName.pop_back();
                        for (char& c : runName) if (c == ' ') c = '_';
                        if (runName.empty()) runName = "Level1_testing";

                        GhostSystem* ghost = CORE ? CORE->GetGhostSystem() : nullptr;
                        if (ghost)
                        {
                            // Derive a friendly level name from the scene path (strip dir + ext)
                            std::string lvl = currentScenePath;
                            size_t slash = lvl.find_last_of("/\\");
                            if (slash != std::string::npos) lvl = lvl.substr(slash + 1);
                            size_t dot = lvl.rfind('.');
                            if (dot != std::string::npos)   lvl = lvl.substr(0, dot);
                            ghost->SetLevelName(lvl);
                            if (CORE->GetGameCallbacks())
                                ghost->SetShellsCollected(CORE->GetGameCallbacks()->GetCurrentShells());
                            ghost->SetRating(score);
                            ghost->SaveCurrentRun(runName);
                            std::cout << "[WinLoseLogic] Run saved as: " << runName << ".json (level: " << lvl << ")\n";
                        }

                        // Hand off to the door animation → win sequence
                        CORE->SetGamePaused(false);
                        isWaitingForWin = true;
                        if (m_doorAnim)
                        {
                            winDelayTimer = 1000000.0f;
                            m_doorAnim->StartAnimation(
                                [this]() { TriggerWin(); CreateWinScreen(); },
                                [this]() { isWaitingForWin = false; },
                                nullptr);
                        }
                        else
                        {
                            winDelayTimer = 0.5f;
                        }
                    },
                    // --- onCancel (Escape) — proceed to win screen without saving ---
                    [this, score]()
                    {
                        std::cout << "[WinLoseLogic] Name input cancelled — skipping save.\n";
                        CORE->SetGamePaused(false);
                        isWaitingForWin = true;
                        if (m_doorAnim)
                        {
                            winDelayTimer = 1000000.0f;
                            m_doorAnim->StartAnimation(
                                [this]() { TriggerWin(); CreateWinScreen(); },
                                [this]() { isWaitingForWin = false; },
                                nullptr);
                        }
                        else
                        {
                            winDelayTimer = 0.5f;
                        }
                    });
            }
            else
            {
                // TextInputSystem unavailable — fall back to default name
                GhostSystem* ghost = CORE ? CORE->GetGhostSystem() : nullptr;
                if (ghost)
                {
                    std::string lvl = currentScenePath;
                    size_t slash = lvl.find_last_of("/\\");
                    if (slash != std::string::npos) lvl = lvl.substr(slash + 1);
                    size_t dot = lvl.rfind('.');
                    if (dot != std::string::npos)   lvl = lvl.substr(0, dot);
                    ghost->SetLevelName(lvl);
                    if (CORE->GetGameCallbacks())
                        ghost->SetShellsCollected(CORE->GetGameCallbacks()->GetCurrentShells());
                    ghost->SetRating(score);
                    ghost->SaveCurrentRun("Level1_testing");
                }
                CORE->SetGamePaused(false);
                isWaitingForWin = true;
                if (m_doorAnim)
                {
                    winDelayTimer = 1000000.0f;
                    m_doorAnim->StartAnimation(
                        [this]() { TriggerWin(); CreateWinScreen(); },
                        [this]() { isWaitingForWin = false; },
                        nullptr);
                }
                else
                {
                    winDelayTimer = 0.5f;
                }
            }
        }
    }

    lastFrameColliding = colliding;
}

// ============ WIN ============
/**
 * @brief Formats elapsed seconds into MM:SS.cc string.
 */
static std::string FormatElapsedTime(float seconds)
{
    int mins = static_cast<int>(seconds) / 60;
    int secs = static_cast<int>(seconds) % 60;
    int centis = static_cast<int>((seconds - std::floor(seconds)) * 100.0f);
    char buf[16];
    std::snprintf(buf, sizeof(buf), "%02d:%02d.%02d", mins, secs, centis);
    return std::string(buf);
}

/**
 * @brief Pauses the game, hides HUD, stops BGM, and kicks off the door animation.
 *        Win screen UI is spawned later in CreateWinScreen() via the onOpening callback.
 */
void WinLoseLogicSystem::TriggerWin()
{
    auto& factory = GameObjectFactory::Instance();

    if (m_doorAnim && !m_doorAnim->IsAnimating())
    {
        winDelayTimer = 1000000.0f;
        m_doorAnim->StartAnimation(
            [this]() { CreateWinScreen(); },  // doors closed — create screen behind them
            [this]() { isWaitingForWin = false; },
            nullptr);
    }
    else if (!m_doorAnim)
    {
        CreateWinScreen();  // no door animation — show immediately
    }

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

    // Hide shell/coin UI
    GameObject* shellIcon1 = factory.FindGameObject("ShellIcon1");
    GameObject* shellIcon2 = factory.FindGameObject("ShellIcon2");
    GameObject* shellIcon3 = factory.FindGameObject("ShellIcon3");
    GameObject* coinIcon = factory.FindGameObject("CoinIcon");
    GameObject* coinText = factory.FindGameObject("CoinCountText");

    if (shellIcon1) shellIcon1->SetActive(false);
    if (shellIcon2) shellIcon2->SetActive(false);
    if (shellIcon3) shellIcon3->SetActive(false);
    if (coinIcon) coinIcon->SetActive(false);
    if (coinText) coinText->SetActive(false);

    // Stop BGM; victory music starts after doors open
    AudioManager* audioManager = CORE->GetAudioManager();
    if (audioManager)
    {
        audioManager->StopSound("bgm_channel");
        audioManager->SetCategoryVolume("music", 0.0f);
        audioManager->PlaySound("victory");
        audioManager->FadeInCategory("music", m_fadeWinIn);
    }
    m_winBGMDelayTimer = 1.5f;

    m_timerRunning = false; // stop elapsed-time counter
}

/**
 * @brief Spawns all win screen UI objects (background, stats panel, buttons).
 *        Called via the door animation's onClosed callback so the screen is
 *        created while the doors are fully shut, then revealed as they open.
 */
void WinLoseLogicSystem::CreateWinScreen()
{
    if (hasCreatedWinScreen) return;

    auto& factory = GameObjectFactory::Instance();
    vec2 camPos = CameraManager::Instance().GetCameraPosition();
    float uiScale = 2.0f;

    // stats for the right-side panel
    std::string statTime = FormatElapsedTime(m_elapsedTime);
    std::string statCoins = std::to_string(CORE->GetGameCallbacks()->GetCurrentCoins() * 10);
    std::string statShells = std::to_string(CORE->GetGameCallbacks()->GetCurrentShells()) + "/3";

    // Get dynamic level information from current scene
    std::string statLevelNum, statLevelName;
    GetLevelInfo(statLevelNum, statLevelName);

    // Right panel X anchor (right third of screen)
    float panelX = 550.0f * uiScale;

    // win screen background
    GameObject* board = factory.CreateGameObject("WinScreen");
    auto* boardTransform = board->AddComponent<TransformComponent>();
    boardTransform->posX = camPos.x;
    boardTransform->posY = camPos.y;
    boardTransform->scaleX = 1920.0f * uiScale;
    boardTransform->scaleY = 1080.0f * uiScale;
    auto* boardRender = board->AddComponent<RenderComponent>();
    boardRender->SetTextureID(GraphicsSystem::LoadTexture("../Assets/Sprites/Game_win_screen.png"));
    boardRender->SetColor(vec4(1.0f, 1.0f, 1.0f, 1.0f));
    boardRender->SetLayer(101);

    // ---- Bracket background behind level title ----
    GameObject* winTitleBracket = factory.CreateGameObject("WinTitleBracket");
    auto* winBracketT = winTitleBracket->AddComponent<TransformComponent>();
    winBracketT->posX = camPos.x + panelX;
    winBracketT->posY = camPos.y + 350.0f * uiScale;
    winBracketT->scaleX = 550.0f * uiScale;
    winBracketT->scaleY = 200.0f * uiScale;
    auto* winBracketR = winTitleBracket->AddComponent<RenderComponent>();
    winBracketR->SetTextureID(GraphicsSystem::LoadTexture("../Assets/Sprites/Icons/win_lose_bracket.png"));
    winBracketR->SetColor(vec4(1.0f, 1.0f, 1.0f, 1.0f));
    winBracketR->SetLayer(102);

    // ---- Level title: "Level 1" ----
    GameObject* levelNumObj = factory.CreateGameObject("WinLevelNumber");
    auto* levelNumT = levelNumObj->AddComponent<TransformComponent>();
    levelNumT->posX = camPos.x + panelX;
    levelNumT->posY = camPos.y + 380.0f * uiScale;
    levelNumT->scaleX = 500.0f * uiScale;
    levelNumT->scaleY = 100.0f * uiScale;
    levelNumObj->AddComponent<TextComponent>(
        "../Assets/FrederickatheGreat-Regular.ttf", static_cast<int>(60 * uiScale),
        statLevelNum, vec4(0.1f, 0.05f, 0.0f, 1.0f), false);
    levelNumObj->GetComponent<TextComponent>()->SetLayer(102);

    // ---- Level subtitle: "Barbershop" ----
    GameObject* levelNameObj = factory.CreateGameObject("WinLevelName");
    auto* levelNameT = levelNameObj->AddComponent<TransformComponent>();
    levelNameT->posX = camPos.x + panelX;
    levelNameT->posY = camPos.y + 310.0f * uiScale;
    levelNameT->scaleX = 500.0f * uiScale;
    levelNameT->scaleY = 70.0f * uiScale;
    levelNameObj->AddComponent<TextComponent>(
        "../Assets/FrederickatheGreat-Regular.ttf", static_cast<int>(36 * uiScale),
        statLevelName, vec4(0.2f, 0.15f, 0.1f, 1.0f), false);
    levelNameObj->GetComponent<TextComponent>()->SetLayer(102);

    // ---- Clock icon ----
    GameObject* clockIcon = factory.CreateGameObject("WinClockIcon");
    auto* clockT = clockIcon->AddComponent<TransformComponent>();
    clockT->posX = camPos.x + (panelX - 180.0f * uiScale);
    clockT->posY = camPos.y + 180.0f * uiScale;
    clockT->scaleX = 110.0f * uiScale;
    clockT->scaleY = 110.0f * uiScale;
    auto* clockR = clockIcon->AddComponent<RenderComponent>();
    clockR->SetTextureID(GraphicsSystem::LoadTexture("../Assets/Sprites/Icons/Clock_icon.png"));
    clockR->SetColor(vec4(1.0f, 1.0f, 1.0f, 1.0f));
    clockR->SetLayer(102);

    // ---- Time label ----
    GameObject* timeLabel = factory.CreateGameObject("WinTimeLabel");
    auto* timeLabelT = timeLabel->AddComponent<TransformComponent>();
    timeLabelT->posX = camPos.x + (panelX - 70.0f * uiScale);
    timeLabelT->posY = camPos.y + 180.0f * uiScale;
    timeLabelT->scaleX = 150.0f * uiScale;
    timeLabelT->scaleY = 70.0f * uiScale;
    timeLabel->AddComponent<TextComponent>(
        "../Assets/FrederickatheGreat-Regular.ttf", static_cast<int>(38 * uiScale),
        "Time", vec4(0.1f, 0.05f, 0.0f, 1.0f), false);
    timeLabel->GetComponent<TextComponent>()->SetLayer(102);

    // ---- Time value ----
    GameObject* timeText = factory.CreateGameObject("WinTimeText");
    auto* timeT = timeText->AddComponent<TransformComponent>();
    timeT->posX = camPos.x + (panelX + 130.0f * uiScale);
    timeT->posY = camPos.y + 180.0f * uiScale;
    timeT->scaleX = 300.0f * uiScale;
    timeT->scaleY = 70.0f * uiScale;
    timeText->AddComponent<TextComponent>(
        "../Assets/FrederickatheGreat-Regular.ttf", static_cast<int>(38 * uiScale),
        statTime, vec4(0.1f, 0.05f, 0.0f, 1.0f), false);
    timeText->GetComponent<TextComponent>()->SetLayer(102);

    // ---- Separator under time ----
    GameObject* winSep1 = factory.CreateGameObject("WinSeparator1");
    auto* winSep1T = winSep1->AddComponent<TransformComponent>();
    winSep1T->posX = camPos.x + panelX;
    winSep1T->posY = camPos.y + 120.0f * uiScale;
    winSep1T->scaleX = 500.0f * uiScale;
    winSep1T->scaleY = 50.0f * uiScale;
    winSep1->AddComponent<TextComponent>(
        "../Assets/FrederickatheGreat-Regular.ttf", static_cast<int>(48 * uiScale),
        "----------------------------", vec4(0.1f, 0.05f, 0.0f, 1.0f), false);
    winSep1->GetComponent<TextComponent>()->SetLayer(102);

    // ---- Coin icon ----
    GameObject* coinIconWin = factory.CreateGameObject("WinCoinIcon");
    auto* coinIconT = coinIconWin->AddComponent<TransformComponent>();
    coinIconT->posX = camPos.x + (panelX - 180.0f * uiScale);
    coinIconT->posY = camPos.y + 70.0f * uiScale;
    coinIconT->scaleX = 110.0f * uiScale;
    coinIconT->scaleY = 110.0f * uiScale;
    auto* coinIconR = coinIconWin->AddComponent<RenderComponent>();
    coinIconR->SetTextureID(GraphicsSystem::LoadTexture("../Assets/Sprites/Icons/Coin_icon.png"));
    coinIconR->SetColor(vec4(1.0f, 1.0f, 1.0f, 1.0f));
    coinIconR->SetLayer(102);

    // ---- Coin count text ----
    GameObject* coinCountWin = factory.CreateGameObject("WinCoinText");
    auto* coinCountT = coinCountWin->AddComponent<TransformComponent>();
    coinCountT->posX = camPos.x + (panelX - 90.0f * uiScale);
    coinCountT->posY = camPos.y + 70.0f * uiScale;
    coinCountT->scaleX = 150.0f * uiScale;
    coinCountT->scaleY = 70.0f * uiScale;
    coinCountWin->AddComponent<TextComponent>(
        "../Assets/FrederickatheGreat-Regular.ttf", static_cast<int>(38 * uiScale),
        statCoins, vec4(0.1f, 0.05f, 0.0f, 1.0f), false);
    coinCountWin->GetComponent<TextComponent>()->SetLayer(102);

    // ---- Shell icon ----
    GameObject* shellIconWin = factory.CreateGameObject("WinShellIcon");
    auto* shellIconT = shellIconWin->AddComponent<TransformComponent>();
    shellIconT->posX = camPos.x + (panelX + 60.0f * uiScale);
    shellIconT->posY = camPos.y + 70.0f * uiScale;
    shellIconT->scaleX = 110.0f * uiScale;
    shellIconT->scaleY = 110.0f * uiScale;
    auto* shellIconR = shellIconWin->AddComponent<RenderComponent>();
    shellIconR->SetTextureID(GraphicsSystem::LoadTexture("../Assets/Sprites/Icons/Shell_icon.png"));
    shellIconR->SetColor(vec4(1.0f, 1.0f, 1.0f, 1.0f));
    shellIconR->SetLayer(102);

    // ---- Shell count text ----
    GameObject* shellCountWin = factory.CreateGameObject("WinShellText");
    auto* shellCountT = shellCountWin->AddComponent<TransformComponent>();
    shellCountT->posX = camPos.x + (panelX + 155.0f * uiScale);
    shellCountT->posY = camPos.y + 70.0f * uiScale;
    shellCountT->scaleX = 150.0f * uiScale;
    shellCountT->scaleY = 70.0f * uiScale;
    shellCountWin->AddComponent<TextComponent>(
        "../Assets/FrederickatheGreat-Regular.ttf", static_cast<int>(38 * uiScale),
        statShells, vec4(0.1f, 0.05f, 0.0f, 1.0f), false);
    shellCountWin->GetComponent<TextComponent>()->SetLayer(102);

    // ---- Separator under collectibles ----
    GameObject* winSep2 = factory.CreateGameObject("WinSeparator2");
    auto* winSep2T = winSep2->AddComponent<TransformComponent>();
    winSep2T->posX = camPos.x + panelX;
    winSep2T->posY = camPos.y + 40.0f * uiScale;
    winSep2T->scaleX = 500.0f * uiScale;
    winSep2T->scaleY = 50.0f * uiScale;
    winSep2->AddComponent<TextComponent>(
        "../Assets/FrederickatheGreat-Regular.ttf", static_cast<int>(48 * uiScale),
        "----------------------------", vec4(0.1f, 0.05f, 0.0f, 1.0f), false);
    winSep2->GetComponent<TextComponent>()->SetLayer(102);

    // ---- Shop Rating: label ----
    GameObject* ratingLabel = factory.CreateGameObject("WinRatingLabel");
    auto* ratingLabelT = ratingLabel->AddComponent<TransformComponent>();
    ratingLabelT->posX = camPos.x + (panelX - 80.0f * uiScale);
    ratingLabelT->posY = camPos.y + -50.0f * uiScale;
    ratingLabelT->scaleX = 320.0f * uiScale;
    ratingLabelT->scaleY = 70.0f * uiScale;
    ratingLabel->AddComponent<TextComponent>(
        "../Assets/FrederickatheGreat-Regular.ttf", static_cast<int>(38 * uiScale),
        "Shop Rating:", vec4(0.1f, 0.05f, 0.0f, 1.0f), false);
    ratingLabel->GetComponent<TextComponent>()->SetLayer(102);

    // ---- Rating badge (Letterscore_dash sprite — score icon, no text overlay) ----
    GameObject* ratingBadge = factory.CreateGameObject("WinRatingBadge");
    auto* ratingBadgeT = ratingBadge->AddComponent<TransformComponent>();
    ratingBadgeT->posX = camPos.x + (panelX + 80.0f * uiScale);
    ratingBadgeT->posY = camPos.y + -20.0f * uiScale;
    ratingBadgeT->scaleX = 120.0f * uiScale;
    ratingBadgeT->scaleY = 100.0f * uiScale;
    auto* ratingBadgeR = ratingBadge->AddComponent<RenderComponent>();

    float score = CalculateRating();
    if (score >= 85.0f)
    {
        ratingBadgeR->SetTextureID(GraphicsSystem::LoadTexture("../Assets/Sprites/Icons/Letterscore_S.png"));
    }
    else if (score >= 75.0f)
    {
        ratingBadgeR->SetTextureID(GraphicsSystem::LoadTexture("../Assets/Sprites/Icons/Letterscore_A.png"));
    }
    else if (score >= 65.0f)
    {
        ratingBadgeR->SetTextureID(GraphicsSystem::LoadTexture("../Assets/Sprites/Icons/Letterscore_B.png"));
    }
    else if (score >= 50.0f)
    {
        ratingBadgeR->SetTextureID(GraphicsSystem::LoadTexture("../Assets/Sprites/Icons/Letterscore_C.png"));
    }
    else
    {
        ratingBadgeR->SetTextureID(GraphicsSystem::LoadTexture("../Assets/Sprites/Icons/Letterscore_dash.png"));
    }

    ratingBadgeR->SetColor(vec4(1.0f, 1.0f, 1.0f, 1.0f));
    ratingBadgeR->SetLayer(102);

    // ---- Sign here image ----
    GameObject* winSignHere = factory.CreateGameObject("WinSignHere");
    auto* winSignT = winSignHere->AddComponent<TransformComponent>();
    winSignT->posX = camPos.x + panelX;
    winSignT->posY = camPos.y + -160.0f * uiScale;
    winSignT->scaleX = 420.0f * uiScale;
    winSignT->scaleY = 120.0f * uiScale;
    auto* winSignR = winSignHere->AddComponent<RenderComponent>();
    winSignR->SetTextureID(GraphicsSystem::LoadTexture("../Assets/Sprites/Icons/Sign_here.png"));
    winSignR->SetColor(vec4(1.0f, 1.0f, 1.0f, 1.0f));
    winSignR->SetLayer(102);

    // ---- Sign here label ----
    GameObject* winSignLabel = factory.CreateGameObject("WinSignLabel");
    auto* winSignLabelT = winSignLabel->AddComponent<TransformComponent>();
    winSignLabelT->posX = camPos.x + panelX;
    winSignLabelT->posY = camPos.y + -210.0f * uiScale;
    winSignLabelT->scaleX = 420.0f * uiScale;
    winSignLabelT->scaleY = 50.0f * uiScale;
    winSignLabel->AddComponent<TextComponent>(
        "../Assets/FrederickatheGreat-Regular.ttf", static_cast<int>(18 * uiScale),
        "(sign here)", vec4(0.1f, 0.05f, 0.0f, 1.0f), false);
    winSignLabel->GetComponent<TextComponent>()->SetLayer(102);

    // ---- Save Run button ----
    GameObject* quitBtn = factory.CreateGameObject("QuitButton");
    auto* quitTransform = quitBtn->AddComponent<TransformComponent>();
    quitTransform->posX = camPos.x + panelX;
    quitTransform->posY = camPos.y + -280.0f * uiScale;
    quitTransform->scaleX = 430.0f * uiScale;
    quitTransform->scaleY = 130.0f * uiScale;
    auto* quitRender = quitBtn->AddComponent<RenderComponent>();
    quitRender->SetTextureID(GraphicsSystem::LoadTexture("../Assets/Sprites/Icons/win_lose_button_dark.png"));
    quitRender->SetColor(vec4(1.0f, 1.0f, 1.0f, 1.0f));
    quitRender->SetLayer(102);
    quitBtn->AddComponent<TextComponent>(
        "../Assets/FrederickatheGreat-Regular.ttf", static_cast<int>(38 * uiScale),
        "Save Run", vec4(0.0f, 0.0f, 0.0f, 1.0f), false);
    quitBtn->GetComponent<TextComponent>()->SetLayer(102);

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

    if (IsMouseOver(factory.FindGameObject("QuitButton"), mouseX, mouseY))
    {
        currentHover = "QuitButton";
    }

    // Keyboard selection: SPACE to confirm
    static const char* kWinButtons[] = { "QuitButton" };
    static int  kWinIdx    = 0;
    static bool kWinActive = false;
    bool spacePressed = false;
    {
        InputManager* inp = CORE->GetInputManager();
        if (inp && !pendingSceneChange)
        {
            if (isClicked && !currentHover.empty()) kWinActive = false; // mouse click takes over
            if (kWinActive) currentHover = kWinButtons[kWinIdx];
            spacePressed = kWinActive && (inp->IsKeyPressed(GLFW_KEY_SPACE) ||
                (inp->IsGamepadConnected() && inp->IsGamepadButtonPressed(GLFW_GAMEPAD_BUTTON_A)));
        }
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

    if ((isClicked || spacePressed) && !hoveredButtonName.empty() && !pendingSceneChange)
    {
        AudioManager* audioManager = CORE->GetAudioManager();

        if (hoveredButtonName == "QuitButton")
        {
            std::cout << "[WinZone] Quit clicked - Going to main menu" << std::endl;

            if (audioManager)
            {
                audioManager->TriggerEvent("UI_Quit");
                // Fade out music before going to main menu
                audioManager->FadeOutCategory("music", m_fadeQuitOut);
                std::cout << "[WinZone] Started music fade out (" << m_fadeQuitOut << " seconds)" << std::endl;
            }

            DestroyWinScreen();
            pendingSceneTimer = m_fadeQuitOut + m_fadeTimerBuffer;
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
        "WinTitleBracket",
        "WinLevelNumber",
        "WinLevelName",
        "WinClockIcon",
        "WinTimeLabel",
        "WinTimeText",
        "WinCoinIcon",
        "WinCoinText",
        "WinShellIcon",
        "WinShellText",
        "WinSeparator1",
        "WinSeparator2",
        "WinRatingLabel",
        "WinRatingBadge",
        "WinSignHere",
        "WinSignLabel",
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

    // Re-show shell/coin UI
    GameObject* shellIcon1 = factory.FindGameObject("ShellIcon1");
    GameObject* shellIcon2 = factory.FindGameObject("ShellIcon2");
    GameObject* shellIcon3 = factory.FindGameObject("ShellIcon3");
    GameObject* coinIcon = factory.FindGameObject("CoinIcon");
    GameObject* coinText = factory.FindGameObject("CoinCountText");

    if (shellIcon1) shellIcon1->SetActive(true);
    if (shellIcon2) shellIcon2->SetActive(true);
    if (shellIcon3) shellIcon3->SetActive(true);
    if (coinIcon) coinIcon->SetActive(true);
    if (coinText) coinText->SetActive(true);

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

    // Hide shell/coin UI
    GameObject* shellIcon1 = factory.FindGameObject("ShellIcon1");
    GameObject* shellIcon2 = factory.FindGameObject("ShellIcon2");
    GameObject* shellIcon3 = factory.FindGameObject("ShellIcon3");
    GameObject* coinIcon = factory.FindGameObject("CoinIcon");
    GameObject* coinText = factory.FindGameObject("CoinCountText");

    if (shellIcon1) shellIcon1->SetActive(false);
    if (shellIcon2) shellIcon2->SetActive(false);
    if (shellIcon3) shellIcon3->SetActive(false);
    if (coinIcon) coinIcon->SetActive(false);
    if (coinText) coinText->SetActive(false);

    // stop all music and play game over sound with fade in
    AudioManager* audioManager = CORE->GetAudioManager();
    if (audioManager)
    {
        audioManager->StopSound("bgm_channel");
        audioManager->SetCategoryVolume("music", 0.0f); // Start at 0
        audioManager->PlaySound("gameover");
        audioManager->FadeInCategory("music", m_fadeLoseIn);
    }

    // ---- Gather stats for the right-side panel ----
    m_timerRunning = false; // stop timer on lose
    std::string statTime = FormatElapsedTime(m_elapsedTime);
    std::string statCoins = std::to_string(CORE->GetGameCallbacks()->GetCurrentCoins() * 10);
    std::string statShells = std::to_string(CORE->GetGameCallbacks()->GetCurrentShells()) + "/3";

    // Get dynamic level information from current scene
    std::string statLevelNum, statLevelName;
    GetLevelInfo(statLevelNum, statLevelName);

    float panelX = 550.0f * uiScale;

    // lose screen background
    GameObject* board = factory.CreateGameObject("LoseScreen");
    auto* boardTransform = board->AddComponent<TransformComponent>();
    boardTransform->posX = camPos.x;
    boardTransform->posY = camPos.y;
    boardTransform->scaleX = 1920.0f * uiScale;
    boardTransform->scaleY = 1080.0f * uiScale;
    auto* boardRender = board->AddComponent<RenderComponent>();
    boardRender->SetTextureID(GraphicsSystem::LoadTexture("../Assets/Sprites/Game_over_screen.png"));
    boardRender->SetColor(vec4(1.0f, 1.0f, 1.0f, 1.0f));
    boardRender->SetLayer(101);

    // ---- Bracket background behind level title ----
    GameObject* loseTitleBracket = factory.CreateGameObject("LoseTitleBracket");
    auto* loseBracketT = loseTitleBracket->AddComponent<TransformComponent>();
    loseBracketT->posX = camPos.x + panelX;
    loseBracketT->posY = camPos.y + 350.0f * uiScale;
    loseBracketT->scaleX = 550.0f * uiScale;
    loseBracketT->scaleY = 200.0f * uiScale;
    auto* loseBracketR = loseTitleBracket->AddComponent<RenderComponent>();
    loseBracketR->SetTextureID(GraphicsSystem::LoadTexture("../Assets/Sprites/Icons/win_lose_bracket.png"));
    loseBracketR->SetColor(vec4(1.0f, 1.0f, 1.0f, 1.0f));
    loseBracketR->SetLayer(102);

    // ---- Level title: "Level 1" ----
    GameObject* levelNumObj = factory.CreateGameObject("LoseLevelNumber");
    auto* levelNumT = levelNumObj->AddComponent<TransformComponent>();
    levelNumT->posX = camPos.x + panelX;
    levelNumT->posY = camPos.y + 380.0f * uiScale;
    levelNumT->scaleX = 500.0f * uiScale;
    levelNumT->scaleY = 100.0f * uiScale;
    levelNumObj->AddComponent<TextComponent>(
        "../Assets/FrederickatheGreat-Regular.ttf", static_cast<int>(60 * uiScale),
        statLevelNum, vec4(0.1f, 0.05f, 0.0f, 1.0f), false);

    // ---- Level subtitle: "Barbershop" ----
    GameObject* levelNameObj = factory.CreateGameObject("LoseLevelName");
    auto* levelNameT = levelNameObj->AddComponent<TransformComponent>();
    levelNameT->posX = camPos.x + panelX;
    levelNameT->posY = camPos.y + 310.0f * uiScale;
    levelNameT->scaleX = 500.0f * uiScale;
    levelNameT->scaleY = 70.0f * uiScale;
    levelNameObj->AddComponent<TextComponent>(
        "../Assets/FrederickatheGreat-Regular.ttf", static_cast<int>(36 * uiScale),
        statLevelName, vec4(0.2f, 0.15f, 0.1f, 1.0f), false);

    // ---- Clock icon ----
    GameObject* clockIcon = factory.CreateGameObject("LoseClockIcon");
    auto* clockT = clockIcon->AddComponent<TransformComponent>();
    clockT->posX = camPos.x + (panelX - 180.0f * uiScale);
    clockT->posY = camPos.y + 210.0f * uiScale;
    clockT->scaleX = 110.0f * uiScale;
    clockT->scaleY = 110.0f * uiScale;
    auto* clockR = clockIcon->AddComponent<RenderComponent>();
    clockR->SetTextureID(GraphicsSystem::LoadTexture("../Assets/Sprites/Icons/Clock_icon.png"));
    clockR->SetColor(vec4(1.0f, 1.0f, 1.0f, 1.0f));
    clockR->SetLayer(102);

    // ---- Time label ----
    GameObject* timeLabel = factory.CreateGameObject("LoseTimeLabel");
    auto* timeLabelT = timeLabel->AddComponent<TransformComponent>();
    timeLabelT->posX = camPos.x + (panelX - 70.0f * uiScale);
    timeLabelT->posY = camPos.y + 210.0f * uiScale;
    timeLabelT->scaleX = 150.0f * uiScale;
    timeLabelT->scaleY = 70.0f * uiScale;
    timeLabel->AddComponent<TextComponent>(
        "../Assets/FrederickatheGreat-Regular.ttf", static_cast<int>(38 * uiScale),
        "Time", vec4(0.1f, 0.05f, 0.0f, 1.0f), false);

    // ---- Time value ----
    GameObject* timeText = factory.CreateGameObject("LoseTimeText");
    auto* timeT = timeText->AddComponent<TransformComponent>();
    timeT->posX = camPos.x + (panelX + 130.0f * uiScale);
    timeT->posY = camPos.y + 210.0f * uiScale;
    timeT->scaleX = 300.0f * uiScale;
    timeT->scaleY = 70.0f * uiScale;
    timeText->AddComponent<TextComponent>(
        "../Assets/FrederickatheGreat-Regular.ttf", static_cast<int>(38 * uiScale),
        statTime, vec4(0.1f, 0.05f, 0.0f, 1.0f), false);

    // ---- Separator under time ----
    GameObject* loseSep1 = factory.CreateGameObject("LoseSeparator1");
    auto* loseSep1T = loseSep1->AddComponent<TransformComponent>();
    loseSep1T->posX = camPos.x + panelX;
    loseSep1T->posY = camPos.y + 150.0f * uiScale;
    loseSep1T->scaleX = 500.0f * uiScale;
    loseSep1T->scaleY = 50.0f * uiScale;
    loseSep1->AddComponent<TextComponent>(
        "../Assets/FrederickatheGreat-Regular.ttf", static_cast<int>(48 * uiScale),
        "----------------------------", vec4(0.1f, 0.05f, 0.0f, 1.0f), false);

    // ---- Coin icon ----
    GameObject* coinIconLose = factory.CreateGameObject("LoseCoinIcon");
    auto* coinIconT = coinIconLose->AddComponent<TransformComponent>();
    coinIconT->posX = camPos.x + (panelX - 180.0f * uiScale);
    coinIconT->posY = camPos.y + 100.0f * uiScale;
    coinIconT->scaleX = 110.0f * uiScale;
    coinIconT->scaleY = 110.0f * uiScale;
    auto* coinIconR = coinIconLose->AddComponent<RenderComponent>();
    coinIconR->SetTextureID(GraphicsSystem::LoadTexture("../Assets/Sprites/Icons/Coin_icon.png"));
    coinIconR->SetColor(vec4(1.0f, 1.0f, 1.0f, 1.0f));
    coinIconR->SetLayer(102);

    // ---- Coin count text ----
    GameObject* coinCountLose = factory.CreateGameObject("LoseCoinText");
    auto* coinCountT = coinCountLose->AddComponent<TransformComponent>();
    coinCountT->posX = camPos.x + (panelX - 90.0f * uiScale);
    coinCountT->posY = camPos.y + 100.0f * uiScale;
    coinCountT->scaleX = 150.0f * uiScale;
    coinCountT->scaleY = 70.0f * uiScale;
    coinCountLose->AddComponent<TextComponent>(
        "../Assets/FrederickatheGreat-Regular.ttf", static_cast<int>(38 * uiScale),
        statCoins, vec4(0.1f, 0.05f, 0.0f, 1.0f), false);

    // ---- Shell icon ----
    GameObject* shellIconLose = factory.CreateGameObject("LoseShellIcon");
    auto* shellIconT = shellIconLose->AddComponent<TransformComponent>();
    shellIconT->posX = camPos.x + (panelX + 60.0f * uiScale);
    shellIconT->posY = camPos.y + 100.0f * uiScale;
    shellIconT->scaleX = 110.0f * uiScale;
    shellIconT->scaleY = 110.0f * uiScale;
    auto* shellIconR = shellIconLose->AddComponent<RenderComponent>();
    shellIconR->SetTextureID(GraphicsSystem::LoadTexture("../Assets/Sprites/Icons/Shell_icon.png"));
    shellIconR->SetColor(vec4(1.0f, 1.0f, 1.0f, 1.0f));
    shellIconR->SetLayer(102);

    // ---- Shell count text ----
    GameObject* shellCountLose = factory.CreateGameObject("LoseShellText");
    auto* shellCountT = shellCountLose->AddComponent<TransformComponent>();
    shellCountT->posX = camPos.x + (panelX + 155.0f * uiScale);
    shellCountT->posY = camPos.y + 100.0f * uiScale;
    shellCountT->scaleX = 150.0f * uiScale;
    shellCountT->scaleY = 70.0f * uiScale;
    shellCountLose->AddComponent<TextComponent>(
        "../Assets/FrederickatheGreat-Regular.ttf", static_cast<int>(38 * uiScale),
        statShells, vec4(0.1f, 0.05f, 0.0f, 1.0f), false);

    // ---- Separator under collectibles ----
    GameObject* loseSep2 = factory.CreateGameObject("LoseSeparator2");
    auto* loseSep2T = loseSep2->AddComponent<TransformComponent>();
    loseSep2T->posX = camPos.x + panelX;
    loseSep2T->posY = camPos.y + 40.0f * uiScale;
    loseSep2T->scaleX = 500.0f * uiScale;
    loseSep2T->scaleY = 50.0f * uiScale;
    loseSep2->AddComponent<TextComponent>(
        "../Assets/FrederickatheGreat-Regular.ttf", static_cast<int>(48 * uiScale),
        "----------------------------", vec4(0.1f, 0.05f, 0.0f, 1.0f), false);

    // ---- Shop Rating: label ----
    GameObject* ratingLabel = factory.CreateGameObject("LoseRatingLabel");
    auto* ratingLabelT = ratingLabel->AddComponent<TransformComponent>();
    ratingLabelT->posX = camPos.x + (panelX - 80.0f * uiScale);
    ratingLabelT->posY = camPos.y + -20.0f * uiScale;
    ratingLabelT->scaleX = 320.0f * uiScale;
    ratingLabelT->scaleY = 70.0f * uiScale;
    ratingLabel->AddComponent<TextComponent>(
        "../Assets/FrederickatheGreat-Regular.ttf", static_cast<int>(38 * uiScale),
        "Shop Rating:", vec4(0.1f, 0.05f, 0.0f, 1.0f), false);

    // ---- Rating badge (Letterscore_dash sprite — score icon, no text overlay) ----
    GameObject* ratingBadge = factory.CreateGameObject("LoseRatingBadge");
    auto* ratingBadgeT = ratingBadge->AddComponent<TransformComponent>();
    ratingBadgeT->posX = camPos.x + (panelX +  50.0f * uiScale);
    ratingBadgeT->posY = camPos.y + -20.0f * uiScale;
    ratingBadgeT->scaleX = 120.0f * uiScale;
    ratingBadgeT->scaleY = 100.0f * uiScale;
    auto* ratingBadgeR = ratingBadge->AddComponent<RenderComponent>();
    ratingBadgeR->SetTextureID(GraphicsSystem::LoadTexture("../Assets/Sprites/Icons/Letterscore_dash.png"));
    ratingBadgeR->SetColor(vec4(1.0f, 1.0f, 1.0f, 1.0f));
    ratingBadgeR->SetLayer(102);

    // ---- Sign here cross image ----
    GameObject* loseSignHere = factory.CreateGameObject("LoseSignHereCross");
    auto* loseSignT = loseSignHere->AddComponent<TransformComponent>();
    loseSignT->posX = camPos.x + panelX;
    loseSignT->posY = camPos.y + -160.0f * uiScale;
    loseSignT->scaleX = 420.0f * uiScale;
    loseSignT->scaleY = 120.0f * uiScale;
    auto* loseSignR = loseSignHere->AddComponent<RenderComponent>();
    loseSignR->SetTextureID(GraphicsSystem::LoadTexture("../Assets/Sprites/Icons/Sign_here_cross.png"));
    loseSignR->SetColor(vec4(1.0f, 1.0f, 1.0f, 1.0f));
    loseSignR->SetLayer(102);

    // ---- Sign here image ----
    GameObject* loseSignHere2 = factory.CreateGameObject("LoseSignHere");
    auto* loseSignT2 = loseSignHere2->AddComponent<TransformComponent>();
    loseSignT2->posX = camPos.x + panelX;
    loseSignT2->posY = camPos.y + -220.0f * uiScale;
    loseSignT2->scaleX = 420.0f * uiScale;
    loseSignT2->scaleY = 120.0f * uiScale;
    auto* loseSignR2 = loseSignHere2->AddComponent<RenderComponent>();
    loseSignR2->SetTextureID(GraphicsSystem::LoadTexture("../Assets/Sprites/Icons/Sign_here.png"));
    loseSignR2->SetColor(vec4(1.0f, 1.0f, 1.0f, 1.0f));
    loseSignR2->SetLayer(102);

    // ---- Sign here label ----
    GameObject* loseSignLabel = factory.CreateGameObject("LoseSignLabel");
    auto* loseSignLabelT = loseSignLabel->AddComponent<TransformComponent>();
    loseSignLabelT->posX = camPos.x + panelX;
    loseSignLabelT->posY = camPos.y + -270.0f * uiScale;
    loseSignLabelT->scaleX = 420.0f * uiScale;
    loseSignLabelT->scaleY = 50.0f * uiScale;
    loseSignLabel->AddComponent<TextComponent>(
        "../Assets/FrederickatheGreat-Regular.ttf", static_cast<int>(18 * uiScale),
        "(sign here)", vec4(0.1f, 0.05f, 0.0f, 1.0f), false);
    loseSignLabel->GetComponent<TextComponent>()->SetLayer(102);

    // ---- Retry button ----
    GameObject* loseQuitBtn = factory.CreateGameObject("LoseQuitButton");
    auto* loseQuitTransform = loseQuitBtn->AddComponent<TransformComponent>();
    loseQuitTransform->posX = camPos.x + panelX;
    loseQuitTransform->posY = camPos.y + -350.0f * uiScale;
    loseQuitTransform->scaleX = 430.0f * uiScale;
    loseQuitTransform->scaleY = 130.0f * uiScale;
    auto* loseQuitRender = loseQuitBtn->AddComponent<RenderComponent>();
    loseQuitRender->SetTextureID(GraphicsSystem::LoadTexture("../Assets/Sprites/Icons/win_lose_button_dark.png"));
    loseQuitRender->SetColor(vec4(1.0f, 1.0f, 1.0f, 1.0f));
    loseQuitRender->SetLayer(102);
    loseQuitBtn->AddComponent<TextComponent>(
        "../Assets/FrederickatheGreat-Regular.ttf", static_cast<int>(38 * uiScale),
        "Retry", vec4(0.0f, 0.0f, 0.0f, 1.0f), false);

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

    if (IsMouseOver(factory.FindGameObject("LoseQuitButton"), mouseX, mouseY))
    {
        currentHover = "LoseQuitButton";
    }

    // Keyboard selection: SPACE to confirm
    static const char* kLoseButtons[] = { "LoseQuitButton" };
    static int  kLoseIdx    = 0;
    static bool kLoseActive = false;
    bool spacePressed = false;
    {
        InputManager* inp = CORE->GetInputManager();
        if (inp && !pendingSceneChange)
        {
            if (isClicked && !currentHover.empty()) kLoseActive = false;
            if (kLoseActive) currentHover = kLoseButtons[kLoseIdx];
            spacePressed = kLoseActive && (inp->IsKeyPressed(GLFW_KEY_SPACE) ||
                (inp->IsGamepadConnected() && inp->IsGamepadButtonPressed(GLFW_GAMEPAD_BUTTON_A)));
        }
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

    if ((isClicked || spacePressed) && !hoveredButtonName.empty() && !pendingSceneChange)
    {
        AudioManager* audioManager = CORE->GetAudioManager();

        if (hoveredButtonName == "LoseQuitButton")
        {
            std::cout << "[WinLoseLogic] Retry clicked - Restarting level" << std::endl;

            AbandonRecording();

            if (audioManager)
            {
                audioManager->TriggerEvent("UI_Click");
                // Fade out music before restarting
                audioManager->FadeOutCategory("music", m_fadeReplayOut);
                //std::cout << "[WinLoseLogic] Started music fade out (" << m_fadeReplayOut << " seconds)" << std::endl;
            }

            DestroyLoseScreen();
            CORE->SetGamePaused(false);

            pendingSceneTimer = m_fadeReplayOut + m_fadeTimerBuffer;
            pendingScenePath = currentScenePath;
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
        "LoseTitleBracket",
        "LoseLevelNumber",
        "LoseLevelName",
        "LoseClockIcon",
        "LoseTimeLabel",
        "LoseTimeText",
        "LoseCoinIcon",
        "LoseCoinText",
        "LoseShellIcon",
        "LoseShellText",
        "LoseSeparator1",
        "LoseSeparator2",
        "LoseRatingLabel",
        "LoseRatingBadge",
        "LoseSignHereCross",
        "LoseSignHere",
        "LoseSignLabel",
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

    // Re-show shell/coin UI
    GameObject* shellIcon1 = factory.FindGameObject("ShellIcon1");
    GameObject* shellIcon2 = factory.FindGameObject("ShellIcon2");
    GameObject* shellIcon3 = factory.FindGameObject("ShellIcon3");
    GameObject* coinIcon = factory.FindGameObject("CoinIcon");
    GameObject* coinText = factory.FindGameObject("CoinCountText");

    if (shellIcon1) shellIcon1->SetActive(true);
    if (shellIcon2) shellIcon2->SetActive(true);
    if (shellIcon3) shellIcon3->SetActive(true);
    if (coinIcon) coinIcon->SetActive(true);
    if (coinText) coinText->SetActive(true);

    hasCreatedLoseScreen = false;
    hoveredButtonName.clear();

    std::cout << "[WinLoseLogic] Lose screen destroyed and state reset" << std::endl;
}

/**
 * @brief Extracts level number and friendly name from the current scene path.
 */
void WinLoseLogicSystem::GetLevelInfo(std::string& outLevelNum, std::string& outLevelName)
{
    // Extract filename from scene path (e.g., "../Assets/Scenes/Level_1.json" -> "Level_1")
    std::string levelFileName = currentScenePath;
    size_t slash = levelFileName.find_last_of("/\\");
    if (slash != std::string::npos)
        levelFileName = levelFileName.substr(slash + 1);
    size_t dot = levelFileName.rfind('.');
    if (dot != std::string::npos)
        levelFileName = levelFileName.substr(0, dot);

    // Map of level filenames to friendly display names
    static const std::map<std::string, std::string> levelNames = {
        {"Level_1", "Barbershop"},
        {"Level_2", "Hair Salon"},
        {"Level_2_V2", "Hair Salon"}
    };

    // override map for level number display
    static const std::map<std::string, std::string> levelNumbers = {
        {"Level_2_V2", "Level 2"}
    };

    auto numOverride = levelNumbers.find(levelFileName);
    if (numOverride != levelNumbers.end())
    {
        outLevelNum = numOverride->second;
    }
    else
    {
        // Try to extract level number from filename (e.g., "Level_1" -> "1")
        std::string levelNum = "";
        size_t underscorePos = levelFileName.find('_');
        if (underscorePos != std::string::npos && underscorePos + 1 < levelFileName.length())
        {
            std::string numPart = levelFileName.substr(underscorePos + 1);
            if (!numPart.empty() && std::all_of(numPart.begin(), numPart.end(), ::isdigit))
            {
                levelNum = numPart;
            }
        }

        if (!levelNum.empty())
        {
            outLevelNum = "Level " + levelNum;
        }
        else
        {
            // Fallback: use the filename as-is with spaces instead of underscores
            outLevelNum = levelFileName;
            for (char& c : outLevelNum)
            {
                if (c == '_') c = ' ';
            }
        }
    }

    // Get friendly name from map, or use filename if not found
    auto it = levelNames.find(levelFileName);
    if (it != levelNames.end())
    {
        outLevelName = it->second;
    }
    else
    {
        // Fallback: use filename with spaces instead of underscores
        outLevelName = levelFileName;
        for (char& c : outLevelName)
        {
            if (c == '_') c = ' ';
        }
    }
}

// helper function
float WinLoseLogicSystem::CalculateRating() const
{
    //(10000 / elapsedTime) * 1000 + (pickups * 500)
    // HARD CODED FOR NOW
    int scorepercentage = 60 - static_cast<int>(m_elapsedTime / 60);
    float currScore = (scorepercentage * 100.f) + (CORE->GetGameCallbacks()->GetCurrentCoins() * 10.f) + (CORE->GetGameCallbacks()->GetCurrentShells() * 50.f);
    float totalScore = 100.f + (CORE->GetGameCallbacks()->GetCurrentCoins() * 10.f) + (CORE->GetGameCallbacks()->GetCurrentShells() * 50.f);
    std::cout << "CalculateRating(): " << (currScore / totalScore) * 100 << '\n';
    return (currScore / totalScore) * 100;
}