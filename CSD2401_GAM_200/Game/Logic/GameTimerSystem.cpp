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
#include "GameCallbacks.h"
#include "ColliderComponent.h"
#include "Collision.h"
#include "transform.h"
#include "stb_image.h"

#include <cstdio>
#include <iostream>
#include <cmath>
#include <algorithm>

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

    uvBoundsReady = false;
    uvBarMin = 0.0f;
    uvBarMax = 1.0f;
    uvBarWidth = 1.0f;

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
    IGameCallbacksSystem* gameCb = CORE ? CORE->GetGameCallbacks() : nullptr;
    if (gameCb && (gameCb->IsWinScreenActive() || gameCb->IsLoseScreenActive()))
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
    bool isHowToPlayOpen = gameCb && gameCb->IsHowToPlayOpen();

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

            auto* renderComp = timerGO->GetComponent<RenderComponent>();
            auto* transformComp = timerGO->GetComponent<TransformComponent>();

            // Restore full bar transform
            if (transformComp && originalScaleX > 0.0f)
            {
                transformComp->SetScale(originalScaleX, originalScaleY);
                transformComp->SetPosition(originalCenterX, originalCenterY);
            }

            // Restore full texture uv
            if (renderComp)
            {
                renderComp->SetUVScale(vec2(1.0f, 1.0f));
                renderComp->SetUVOffset(vec2(0.0f, 0.0f));
            }

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
            if (gameCb)
            {
                gameCb->TriggerLose();
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

    uvBoundsReady = false;
    uvBarMin = 0.0f;
    uvBarMax = 1.0f;
    uvBarWidth = 1.0f;

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

void GameTimerSystem::ComputeBarUVBounds(RenderComponent* renderComp) {
    // Use the real blue bar texture path here if you do not have a getter
    std::string texturePath = renderComp->GetTexturePath();

    int width = 0;
    int height = 0;
    int channels = 0;
    unsigned char* pixels = stbi_load(texturePath.c_str(), &width, &height, &channels, 4);

    if (pixels && width > 0 && height > 0)
    {
        int minX = width;
        int maxX = -1;

        for (int y = 0; y < height; ++y)
        {
            for (int x = 0; x < width; ++x)
            {
                int index = (y * width + x) * 4;
                unsigned char alpha = pixels[index + 3];

                if (alpha > 0)
                {
                    minX = std::min(minX, x);
                    maxX = std::max(maxX, x);
                }
            }
        }

        if (maxX >= minX)
        {
            uvBarMin = static_cast<float>(minX) / static_cast<float>(width);
            uvBarMax = static_cast<float>(maxX + 1) / static_cast<float>(width); // exclusive bound
            uvBarWidth = uvBarMax - uvBarMin;
        }
        else
        {
            uvBarMin = 0.0f;
            uvBarMax = 1.0f;
            uvBarWidth = 1.0f;
        }

        stbi_image_free(pixels);
    }
    else
    {
        uvBarMin = 0.0f;
        uvBarMax = 1.0f;
        uvBarWidth = 1.0f;
    }

    uvBoundsReady = true;
}

/**
 * @brief Updates the timer bar fill amount based on remaining time.
 */
void GameTimerSystem::UpdateBar(int )
{
    // find timer bar by name
    auto& factory = GameObjectFactory::Instance();
    GameObject* timerGO = factory.FindGameObject(timerObjectName);
    GameObject* timerBackground = factory.FindGameObject("LevelTimerBarBackground");

    if (!timerGO)
    {
        //std::cout << "[GameTimerSystem] Timer GameObject \"" << timerObjectName << "\" not found.\n";
        return;
    }

    auto* renderComp = timerGO->GetComponent<RenderComponent>();
    auto* transformComp = timerGO->GetComponent<TransformComponent>();
    auto* bgTransform = timerBackground->GetComponent<TransformComponent>();

    if (!renderComp || !transformComp)
    {
        //std::cout << "[GameTimerSystem] No RenderComponent or TransformComponent on \"" << timerObjectName << "\".\n";
        return;
    }

    // check if win or lose screen is active, if so hide the timer
    IGameCallbacksSystem* cb = CORE ? CORE->GetGameCallbacks() : nullptr;
    if (cb && (cb->IsWinScreenActive() || cb->IsLoseScreenActive()))
    {
        timerGO->SetActive(false);
        return;
    }

    // make sure timer is active when not on win/lose screen
    if (!timerGO->IsActive())
    {
        timerGO->SetActive(true);
    }
    
    // Compute the visible blue UV range once from the actual timer texture
    if (!uvBoundsReady)
    {
        ComputeBarUVBounds(renderComp);
    }

    if (originalScaleX <= 0.0f)
    {
        transformComp->GetScale(originalScaleX, originalScaleY);
        transformComp->GetPosition(originalCenterX, originalCenterY);

        // world-space start of the ACTUAL visible blue bar
        leftEdge = (originalCenterX - originalScaleX * 0.5f) + (originalScaleX * uvBarMin);

        // usable visible blue width only
        rightEdge = originalScaleX * uvBarWidth;
    }

    float fillPercentage = std::clamp(remainingTime / 60.0f, 0.0f, 1.0f);
    float safeFill = std::max(fillPercentage, 0.001f);

    // world width of remaining visible blue
    float newScaleX = rightEdge * safeFill;
    float newCenterX = leftEdge + (newScaleX * 0.5f);

    transformComp->SetScale(newScaleX, originalScaleY);
    transformComp->SetPosition(newCenterX, originalCenterY);

    // crop inside the painted blue bar and preserve the right-end detail
    float croppedUWidth = uvBarWidth * safeFill;
    renderComp->SetUVScale(vec2(croppedUWidth, 1.0f));
    renderComp->SetUVOffset(vec2(uvBarMax - croppedUWidth, 0.0f));;
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
