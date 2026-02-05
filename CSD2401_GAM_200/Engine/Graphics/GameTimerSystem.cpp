/* Start Header ************************************************************************/
/*!
\file       GameTimerSystem.cpp
\author     Leck Xin Yee [100% - 118 lines]
\par        xinyee.leck@digipen.edu
\date       November, 26, 2025

Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*/
/* End Header **************************************************************************/

#include "GameTimerSystem.h"
#include "WinLoseLogic.h"
#include "ColliderComponent.h"
#include "Collision.h"
#include "transform.h"

#include <cstdio>
#include <iostream>
#include <cmath>

/**
 * @brief Initializes the countdown timer and updates initial display.
 */
void GameTimerSystem::Initialize()
{
    remainingTime = 60.0f;
    lastDisplayedSeconds = 60;
    hasTriggeredTimeUp = false;
    UpdateText(60);
}

/**
 * @brief Updates timer logic, handles editor/pause conditions, hides timer during win/lose,
 *        and triggers lose state when time reaches zero.
 */
void GameTimerSystem::Update(float dt)
{
    LevelEditor* editor = CORE ? CORE->GetLevelEditor() : nullptr;
    if (!editor)
    {
        return;
    }

    EditorMode mode = editor->GetEditorMode();

    // check if timer object exists in current scene, if not don't run timer
    auto& factory = GameObjectFactory::Instance();
    GameObject* timerGO = factory.FindGameObject(timerObjectName);
    if (!timerGO)
    {
        // timer doesn't exist in this scene (e.g. main menu), don't run
        return;
    }

    // hide timer if win or lose screen is active
    WinLoseLogicSystem* winLoseLogic = CORE->GetWinLoseLogic();
    if (winLoseLogic && (winLoseLogic->IsWinScreenActive() || winLoseLogic->IsLoseScreenActive()))
    {
        if (timerGO->IsActive())
        {
            timerGO->SetActive(false);
        }
        return;
    }

    // reset timer and text
    if (mode == EditorMode::Stop)
    {
        if (remainingTime != 60.0f || lastDisplayedSeconds != 60)
        {
            remainingTime = 60.0f;
            lastDisplayedSeconds = 60;
            hasTriggeredTimeUp = false;
            UpdateText(60);
        }
        return;
    }

    // keep timer frozen
    if (mode == EditorMode::Pause || CORE->IsGamePaused())
    {
        return;
    }

    // only decrease time in play mode
    if (mode == EditorMode::Play)
    {
        // Only decrement timer if player is outside SafeZone
        if (!IsPlayerInSafeZone())
        {
            remainingTime -= dt;
        }

        // check for lose condition aka time ran out
        if (remainingTime <= 0.0f && !hasTriggeredTimeUp)
        {
            remainingTime = 0.0f;
            UpdateText(0);
            hasTriggeredTimeUp = true;

            // LOSE CONDITION - trigger lose screen
            std::cout << "TIME'S UP! Triggering lose screen..." << std::endl;
            WinLoseLogicSystem* P_winLoseLogic = CORE->GetWinLoseLogic();
            if (P_winLoseLogic)
            {
                P_winLoseLogic->TriggerLose();
            }
            return;
        }

        int totalSeconds = static_cast<int>(remainingTime);
        if (totalSeconds != lastDisplayedSeconds)
        {
            lastDisplayedSeconds = totalSeconds;
            UpdateText(totalSeconds);
        }
    }
}

/**
 * @brief Resets timer and updates displayed text.
 */
void GameTimerSystem::ResetTimer()
{
    remainingTime = 30.0f;
    lastDisplayedSeconds = 30;
    hasTriggeredTimeUp = false;
    UpdateText(30);
}

/**
 * @brief Decreases the remaining timer value by a given amount.
 */
void GameTimerSystem::DecreaseTime(float amount)
{
    remainingTime -= amount;
    if (remainingTime < 0.0f)
    {
        remainingTime = 0.0f;
    }
}

/**
 * @brief Updates the formatted timer text (mm:ss) on the assigned TextComponent.
 */
void GameTimerSystem::UpdateText(int totalSeconds)
{
    // find timer by name
    auto& factory = GameObjectFactory::Instance();
    GameObject* timerGO = factory.FindGameObject(timerObjectName);

    if (!timerGO)
    {
        //std::cout << "[GameTimerSystem] Timer GameObject \"" << timerObjectName << "\" not found.\n";
        return;
    }

    auto* textComp = timerGO->GetComponent<TextComponent>();
    if (!textComp)
    {
        //std::cout << "[GameTimerSystem] No TextComponent on \"" << timerObjectName << "\".\n";
        return;
    }

    // check if win or lose screen is active, if so hide the timer
    WinLoseLogicSystem* winLoseLogic = CORE->GetWinLoseLogic();
    if (winLoseLogic && (winLoseLogic->IsWinScreenActive() || winLoseLogic->IsLoseScreenActive()))
    {
        textComp->SetText("");
        timerGO->SetActive(false);
        return;
    }

    // make sure timer is active when not on win/lose screen
    if (!timerGO->IsActive())
    {
        timerGO->SetActive(true);
    }

    int minutes = totalSeconds / 60;
    int seconds = totalSeconds % 60;

    char buffer[16];
    std::snprintf(buffer, sizeof(buffer), "%02d:%02d", minutes, seconds);

    textComp->SetText(buffer);
}

/**
 * @brief Checks if the player is inside the safe zone using AABB collision detection.
 */
bool GameTimerSystem::IsPlayerInSafeZone()
{
    auto& factory = GameObjectFactory::Instance();
    GameObject* safeZone = factory.FindGameObject("SafeZone");
    GameObject* player = factory.FindGameObject("Hero");

    // if SafeZone doesn't exist or player doesn't exist, timer runs normally
    if (!safeZone || !player || !safeZone->IsActive() || !player->IsActive())
    {
        return false;
    }

    auto* safeZoneTransform = safeZone->GetComponent<TransformComponent>();
    auto* playerTransform = player->GetComponent<TransformComponent>();
    auto* safeZoneCollider = safeZone->GetComponent<ColliderComponent>();
    auto* playerCollider = player->GetComponent<ColliderComponent>();

    if (!safeZoneTransform || !playerTransform || !safeZoneCollider || !playerCollider)
    {
        return false;
    }

    AABB safeAABB = safeZoneCollider->GetAABB();
    AABB playerAABB = playerCollider->GetAABB();

    float safeX = (safeAABB.min.x + safeAABB.max.x) / 2.0f;
    float safeY = (safeAABB.min.y + safeAABB.max.y) / 2.0f;
    float playerX = (playerAABB.min.x + playerAABB.max.x) / 2.0f;
    float playerY = (playerAABB.min.y + playerAABB.max.y) / 2.0f;

    float safeHalfWidth = (safeAABB.max.x - safeAABB.min.x) / 2.0f;
    float safeHalfHeight = (safeAABB.max.y - safeAABB.min.y) / 2.0f;
    float playerHalfWidth = (playerAABB.max.x - playerAABB.min.x) / 2.0f;
    float playerHalfHeight = (playerAABB.max.y - playerAABB.min.y) / 2.0f;

    float distX = std::abs(playerX - safeX);
    float distY = std::abs(playerY - safeY);
    float thresholdX = playerHalfWidth + safeHalfWidth;
    float thresholdY = playerHalfHeight + safeHalfHeight;

    // true if player is colliding with or inside the safe zone
    return (distX < thresholdX) && (distY < thresholdY);
}
