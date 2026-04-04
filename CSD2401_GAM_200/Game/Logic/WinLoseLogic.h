/* Start Header ************************************************************************/
/*!
\file       WinLoseLogic.h
\author     Leck Xin Yee [100% - 39 lines]
\par        xinyee.leck@digipen.edu
\date       November, 29, 2025

Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*/
/* End Header **************************************************************************/

#pragma once
#include "System.h"
#include "factory.h"
#include "Collidercomponent.h"
#include "transform.h"
#include "core.h"

class DoorAnimationLogicSystem;  // forward declaration

/**
 * @class WinLoseLogicSystem
 * @brief Handles win and lose state transitions, UI screens, and scene loading logic.
 */
class WinLoseLogicSystem : public ISystem {
public:
    /**
     * @brief Constructs the WinLoseLogicSystem.
     */
    WinLoseLogicSystem() = default;

    /**
     * @brief Destructs the WinLoseLogicSystem.
     */
    ~WinLoseLogicSystem() = default;

    /**
     * @brief Initializes all win/lose state variables.
     */
    void Initialize() override;

    /**
     * @brief Updates win/lose logic every frame.
     * @param dt Delta time in seconds.
     */
    void Update(float dt) override;

    /**
     * @brief Gets the name of this system.
     * @return The string "WinLoseLogicSystem".
     */
    std::string GetName() override { return "WinLoseLogicSystem"; }

    /**
     * @brief Sets the current scene filepath for restart operations.
     * @param scenePath Path to the active scene.
     */
    void SetCurrentScene(const std::string& scenePath) { currentScenePath = scenePath; }

    /**
     * @brief Connects the door animation system so it plays when the player wins.
     * @param doorAnim Pointer to the registered DoorAnimationLogicSystem.
     */
    void SetDoorAnimSystem(DoorAnimationLogicSystem* doorAnim) { m_doorAnim = doorAnim; }

    /**
     * @brief Checks if the win screen UI is currently active.
     * @return True if win screen is enabled.
     */
    bool IsWinScreenActive() const { return hasCreatedWinScreen || hasCreatedWinScreen2; }

    /**
     * @brief Checks if the lose screen UI is currently active.
     * @return True if lose screen is enabled.
     */
    bool IsLoseScreenActive() const { return hasCreatedLoseScreen; }

    /**
     * @brief Triggers the lose sequence and displays the lose UI.
     */
    void TriggerLose();

    float CalculateRating() const;

    /**
     * @brief Sets audio fade durations from scene metadata.
     * @param winIn Victory screen music fade in duration.
     * @param loseIn Game over screen music fade in duration.
     * @param replayOut Replay button fade out duration.
     * @param quitOut Quit button fade out duration.
     * @param nextOut Next/Continue button fade out duration.
     * @param buffer Extra time buffer after fade completes.
     */
    void SetAudioFadeDurations(float winIn, float loseIn, float replayOut, float quitOut, float nextOut, float buffer)
    {
        m_fadeWinIn = winIn;
        m_fadeLoseIn = loseIn;
        m_fadeReplayOut = replayOut;
        m_fadeQuitOut = quitOut;
        m_fadeNextOut = nextOut;
        m_fadeTimerBuffer = buffer;
    }

private:
    // ---------------- DOOR ANIMATION ----------------
    DoorAnimationLogicSystem* m_doorAnim = nullptr;

    // ---------------- WIN STATE ----------------
    bool hasPlayerWon = false;
    float winDelayTimer = 0.0f;
    bool isWaitingForWin = false;
    bool hasCreatedWinScreen = false;
    bool hasCreatedWinScreen2 = false;
    bool AudioCheck = false;
    float m_winBGMDelayTimer = -1.0f;  // counts down after doors open; -1 = inactive

    // ---------------- LOSE STATE ----------------
    bool hasCreatedLoseScreen = false;

    // ---------------- RECORDING STATE ----------------
    bool m_playerInSafeZone = true;   // true until player first leaves SafeZone
    bool m_recordingActive = false;  // true while the GhostSystem is recording

    // ---------------- SHARED STATE ----------------
    std::string hoveredButtonName = "";
    std::string currentScenePath = "";
    bool pendingSceneChange = false;
    float pendingSceneTimer = 0.0f;
    std::string pendingScenePath = "";
    bool hasCreatedTransitionOverlay = false;
    bool isPlayingCredits = false;

    // ---------------- ELAPSED TIME ----------------
    float m_elapsedTime = 0.0f;  // seconds since player left SafeZone
    bool  m_timerRunning = false;

    // ---------------- AUDIO FADE DURATIONS ----------------
    // Loaded from scene's GameSettings.AudioFadeDurations
    float m_fadeWinIn = 2.0f;       // Victory screen music fade in
    float m_fadeLoseIn = 2.0f;      // Game over screen music fade in
    float m_fadeReplayOut = 1.5f;   // Replay button fade out
    float m_fadeQuitOut = 2.0f;     // Quit button fade out
    float m_fadeNextOut = 1.5f;     // Next/Continue button fade out
    float m_fadeTimerBuffer = 0.2f; // Extra time buffer after fade completes

    /**
     * @brief Checks if the player has left the safe zone and starts recording.
     */
    void CheckSafeZoneExit();

    /** @brief Stop and discard any in-progress recording (player died / restarted). */
    void AbandonRecording();

    /**
     * @brief Checks if the player has collided with the win zone using physics.
     */
    void CheckWinZoneCollision();

    /**
     * @brief Pauses the game, hides HUD, stops BGM, and starts the door animation.
     */
    void TriggerWin();

    /**
     * @brief Spawns all win screen UI objects. Called via the door animation's
     *        onOpening callback so the screen is revealed as the doors slide open.
     */
    void CreateWinScreen();

    /**
     * @brief Handles hover/click logic for win screen buttons.
     */
    void HandleWinScreenButtons();

    /**
     * @brief Destroys win screen UI objects and resets state.
     */
    void DestroyWinScreen();

    /**
     * @brief Spawns the second victory screen with Continue/Replay/Quit buttons.
     */
    void CreateWinScreen2();

    /**
     * @brief Handles hover/click logic for the second win screen buttons.
     */
    void HandleWinScreen2Buttons();

    /**
     * @brief Destroys the second win screen UI objects and resets state.
     */
    void DestroyWinScreen2();

    /**
     * @brief Handles hover/click logic for lose screen buttons.
     */
    void HandleLoseScreenButtons();

    /**
     * @brief Destroys lose screen UI objects and resets state.
     */
    void DestroyLoseScreen();

    /**
     * @brief Checks if the mouse is over a UI button.
     * @param button Pointer to the GameObject representing the button.
     * @param mouseX Mouse world X position.
     * @param mouseY Mouse world Y position.
     * @return True if mouse overlaps button region.
     */
    bool IsMouseOver(GameObject* button, double mouseX, double mouseY);

    /**
     * @brief Updates the texture of a UI button based on hover state.
     * @param buttonName Name of the button object.
     * @param isHovered Whether the button is currently hovered.
     */
    void SetButtonTexture(std::string buttonName, bool isHovered);

    /**
     * @brief Extracts level number and friendly name from current scene path.
     * @param outLevelNum Output string for level number display (e.g., "Level 1").
     * @param outLevelName Output string for level friendly name (e.g., "Barbershop").
     */
    void GetLevelInfo(std::string& outLevelNum, std::string& outLevelName);
};