/* Start Header ************************************************************************/
/*!
\file       GameTimerSystem.h
\author     Leck Xin Yee [100% - 34 lines]
\par        xinyee.leck@digipen.edu
\date       November, 26, 2025

Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*/
/* End Header **************************************************************************/

#pragma once

#include "core.h"
#include "factory.h"
#include "LevelEditor.h"
#include "System.h"
#include "TextComponent.h"
#include "RenderComponent.h"

#include <string>

/**
 * @class GameTimerSystem
 * @brief Handles the level countdown timer, updating the UI text and triggering time-up conditions.
 */
class GameTimerSystem : public ISystem
{
public:
    /**
     * @brief Default constructor for GameTimerSystem.
     */
    GameTimerSystem() = default;

    /**
     * @brief Default destructor.
     */
    ~GameTimerSystem() = default;

    /**
     * @brief Initializes timer state and updates initial displayed text.
     */
    void Initialize() override;

    /**
     * @brief Updates the countdown timer depending on editor mode, pause state, and win/lose UI visibility.
     * @param dt Delta time in seconds.
     */
    void Update(float dt) override;

    /**
     * @brief Returns the system name.
     * @return "GameTimerSystem"
     */
    std::string GetName() override { return "GameTimerSystem"; }

    /**
     * @brief Resets timer to its initial value and updates display text.
     */
    void ResetTimer();

    /**
     * @brief Gets the remaining time (in seconds).
     * @return Current remaining time.
     */
    float GetRemainingTime() const { return remainingTime; }

    /**
     * @brief Sets the name of the GameObject that displays the timer text.
     * @param name Name of the timer GameObject.
     */
    void SetTimerObjectName(const std::string& name) { timerObjectName = name; }

    /**
     * @brief Decreases the timer by a specific amount.
     * @param amount Time (seconds) to subtract.
     */
    void DecreaseTime(float amount); // decrease timer by ...amount

    /**
     * @brief Checks whether the timer has reached zero.
     * @return True if remaining time is 0 or below.
     */
    bool IsTimeUp() const { return remainingTime <= 0.0f; }

    /**
     * @brief Compute Bar UV Bounds for timer
     *  @param renderComp of the TimerGO
     */
    void ComputeBarUVBounds(RenderComponent* renderComp);

    float GetLevelTimerLength() const { return LevelTimerLength; }

    /**
     * @brief Sets the level timer length (in seconds).
     * @param length Time in seconds for the level timer.
     */
    void SetLevelTimerLength(float length) { LevelTimerLength = length; }

    /**
     * @brief Adds time to remaining time. Handles clamping. Will be used for Timer pickup
     */
    void AddTime(float length) { 
        remainingTime += length; 
        if (remainingTime > LevelTimerLength)           // clamp
            remainingTime = LevelTimerLength; 
    }

private:
    float       LevelTimerLength = 60.0f;
    float       remainingTime = 30.0f;              // remaining time in 30 seconds for now
    int         lastDisplayedSeconds = -1;          // last whole second shown on HUD
    std::string timerObjectName = "LevelTimerBar";
    bool        hasTriggeredTimeUp = false;         // prevent multiple scene loads
    bool        hasTriggeredPanicSound = false;
    bool        hasTriggeredLowPass = false;
    float       lowPassThreshold = 20.0f;          // seconds before time-up to engage low-pass filter

    // Bar scaling variables
    float       originalScaleX = 0.0f;              // original bar width
    float       originalScaleY = 0.0f;              // original bar height
    float       originalCenterX = 0.0f;             // original bar center X position
    float       originalCenterY = 0.0f;             // original bar center Y position
    float       rightEdge = 0.0f;                   // right edge anchor point
    float       leftEdge = 0.0f;                   // right edge anchor point

    float       uvBarMin = 0.0f;
    float       uvBarMax = 1.0f;
    float       uvBarWidth = 1.0f;
    bool        uvBoundsReady = false;

    /**
     * @brief Updates the timer's displayed UI bar fill amount.
     * @param totalSeconds Whole seconds to display.
     */
    void UpdateBar(int totalSeconds);

    /**
     * @brief Checks if the player is currently inside the SafeZone.
     * @return True if player is in SafeZone, false otherwise.
     */
    bool IsPlayerInSafeZone();
};
