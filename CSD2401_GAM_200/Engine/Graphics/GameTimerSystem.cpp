/* Start Header ************************************************************************/
/*!
\file       GameTimerSystem.cpp
\author     Leck Xin Yee [100% - 232 lines]
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

    // Reset bar scaling variables
    originalScaleX = 0.0f;
    originalScaleY = 0.0f;
    originalCenterX = 0.0f;
    originalCenterY = 0.0f;
    leftEdge = 0.0f;

    UpdateBar(60);
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

    // get background and frame objects
    GameObject* timerBackground = factory.FindGameObject("LevelTimerBarBackground");
    GameObject* timerFrame = factory.FindGameObject("LevelTimerBarFrame");

    // hide timer if win or lose screen is active
    WinLoseLogicSystem* winLoseLogic = CORE->GetWinLoseLogic();
    if (winLoseLogic && (winLoseLogic->IsWinScreenActive() || winLoseLogic->IsLoseScreenActive()))
    {
        if (timerGO->IsActive())
        {
            timerGO->SetActive(false);
        }
        if (timerBackground && timerBackground->IsActive())
        {
            timerBackground->SetActive(false);
        }
        if (timerFrame && timerFrame->IsActive())
        {
            timerFrame->SetActive(false);
        }
        return;
    }

    // Check if How To Play is open - if so, stop timer and hide it
    PauseMenuLogicSystem* pauseMenu = CORE->GetPauseMenuLogic();
    bool isHowToPlayOpen = pauseMenu && pauseMenu->IsHowToPlayOpen();

    // Stop timer countdown and hide timer bar if How To Play is open
    if (isHowToPlayOpen)
    {
        // Hide timer bar
        if (timerGO->IsActive())
        {
            timerGO->SetActive(false);
        }
        if (timerBackground && timerBackground->IsActive())
        {
            timerBackground->SetActive(false);
        }
        if (timerFrame && timerFrame->IsActive())
        {
            timerFrame->SetActive(false);
        }
        // Don't count down
        return;
    }

    // hide timer if game is paused
    if (mode == EditorMode::Pause || CORE->IsGamePaused())
    {
        if (timerGO->IsActive())
        {
            timerGO->SetActive(false);
        }
        if (timerBackground && timerBackground->IsActive())
        {
            timerBackground->SetActive(false);
        }
        if (timerFrame && timerFrame->IsActive())
        {
            timerFrame->SetActive(false);
        }
        return;
    }

    // show timer when not paused and not in win/lose state
    if (!timerGO->IsActive())
    {
        timerGO->SetActive(true);
    }
    if (timerBackground && !timerBackground->IsActive())
    {
        timerBackground->SetActive(true);
    }
    if (timerFrame && !timerFrame->IsActive())
    {
        timerFrame->SetActive(true);
    }

    // reset timer and bar
    if (mode == EditorMode::Stop)
    {
        if (remainingTime != 60.0f || lastDisplayedSeconds != 60)
        {
            remainingTime = 60.0f;
            lastDisplayedSeconds = 60;
            hasTriggeredTimeUp = false;

            // Reset bar scaling variables
            originalScaleX = 0.0f;
            originalScaleY = 0.0f;
            originalCenterX = 0.0f;
            originalCenterY = 0.0f;
            leftEdge = 0.0f;

            UpdateBar(60);
        }
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
        }

        // Update bar every frame for smooth animation
        UpdateBar(totalSeconds);
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

    // Reset bar scaling variables
    originalScaleX = 0.0f;
    originalScaleY = 0.0f;
    originalCenterX = 0.0f;
    originalCenterY = 0.0f;
    leftEdge = 0.0f;

    UpdateBar(30);
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
 * @brief Updates the timer bar fill amount based on remaining time.
 */
void GameTimerSystem::UpdateBar(int totalSeconds)
{
    // find timer bar by name
    auto& factory = GameObjectFactory::Instance();
    GameObject* timerGO = factory.FindGameObject(timerObjectName);

    if (!timerGO)
    {
        //std::cout << "[GameTimerSystem] Timer GameObject \"" << timerObjectName << "\" not found.\n";
        return;
    }

    auto* renderComp = timerGO->GetComponent<RenderComponent>();
    auto* transformComp = timerGO->GetComponent<TransformComponent>();

    if (!renderComp || !transformComp)
    {
        //std::cout << "[GameTimerSystem] No RenderComponent or TransformComponent on \"" << timerObjectName << "\".\n";
        return;
    }

    // check if win or lose screen is active, if so hide the timer
    WinLoseLogicSystem* winLoseLogic = CORE->GetWinLoseLogic();
    if (winLoseLogic && (winLoseLogic->IsWinScreenActive() || winLoseLogic->IsLoseScreenActive()))
    {
        timerGO->SetActive(false);
        return;
    }

    // make sure timer is active when not on win/lose screen
    if (!timerGO->IsActive())
    {
        timerGO->SetActive(true);
    }

    // store original dimensions ONCE (when variables are zero)
    if (originalScaleX == 0.0f)
    {
        transformComp->GetScale(originalScaleX, originalScaleY);
        transformComp->GetPosition(originalCenterX, originalCenterY);

        // Store the left edge position (this will be our anchor point)
        leftEdge = originalCenterX - (originalScaleX / 2.0f);

        // check the grey background and frame positions too
        auto& factory = GameObjectFactory::Instance();
        GameObject* greyBG = factory.FindGameObject("LevelTimerBarBackground");
        GameObject* frame = factory.FindGameObject("LevelTimerBarFrame");

        if (greyBG && frame)
        {
            auto* greyTransform = greyBG->GetComponent<TransformComponent>();
            auto* frameTransform = frame->GetComponent<TransformComponent>();

            float greyX, greyY, greyScaleX, greyScaleY;
            float frameX, frameY, frameScaleX, frameScaleY;

            greyTransform->GetPosition(greyX, greyY);
            greyTransform->GetScale(greyScaleX, greyScaleY);
            frameTransform->GetPosition(frameX, frameY);
            frameTransform->GetScale(frameScaleX, frameScaleY);

            float greyLeft = greyX - (greyScaleX / 2.0f);
            float greyRight = greyX + (greyScaleX / 2.0f);

            //std::cout << "[Timer Init] BLUE: Center=" << originalCenterX << ", Scale=" << originalScaleX
            //          << ", Edges=[" << leftEdge << " to " << (leftEdge + originalScaleX) << "]" << std::endl;
            //std::cout << "[Timer Init] GREY: Center=" << greyX << ", Scale=" << greyScaleX
            //          << ", Edges=[" << greyLeft << " to " << greyRight << "]" << std::endl;
            //std::cout << "[Timer Init] FRAME: Center=" << frameX << ", Scale=" << frameScaleX << std::endl;
        }
    }

    // calculate fill percentage (0.0 to 1.0)
    float fillPercentage = remainingTime / 60.0f;
    if (fillPercentage < 0.0f) fillPercentage = 0.0f;
    if (fillPercentage > 1.0f) fillPercentage = 1.0f;

    // scale the bar width
    const float MIN_WIDTH = 20.0f;
    float newScaleX = originalScaleX * fillPercentage;
    if (newScaleX < MIN_WIDTH) newScaleX = MIN_WIDTH;

    // calculate new center position to keep left edge anchored at leftEdge
    float newCenterX = leftEdge + (newScaleX / 2.0f);

    transformComp->SetScale(newScaleX, originalScaleY);
    transformComp->SetPosition(newCenterX, originalCenterY);
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
