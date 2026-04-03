/* Start Header ************************************************************************/
/*!
\file       MenuLogic.h
\author     Jonathan Fam Qian Wei [100% - 21 lines]
\par        Fam.q@digipen.edu
\date       Novenber, 7, 2025

Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*/
/* End Header **************************************************************************/

#pragma once
#include "System.h"
#include "factory.h"
#include "Serialization.h"
#include "InputManager.h"
#include "transform.h"
#include "MathLib.h"
#include "GraphicsSystem.h"
#include "LevelEditor.h"
#include <vector>
#include <string>

class LevelEditor;
class GraphicsSystem;
/**
 * @brief System responsible for handling menu logic and button interactions.
 */
class MenuLogicSystem : public ISystem {
public:
    /**
     * @brief Default constructor.
     */
    MenuLogicSystem() = default;
    
    /**
     * @brief Default destructor.
     */
    ~MenuLogicSystem() = default;

    /**
     * @brief The main update loop for menu logic, handles button hover states and interactions.
     * @param dt The time elapsed since the last frame.
     */
    void Update(float dt) override;

    /**
     * @brief Gets the name of this system.
     * @return The system name as a string.
     */
    std::string GetName() override { return "MenuLogicSystem"; }

    /**
     * @brief Initializes the menu logic system.
     */
    void Initialize() override { m_levelSelectDataLoaded = false; }
 
    /**
     * @brief Resets the menu state to the default (Home screen).
     */
    void Reset();

private:
    std::string hoveredButtonName = "";   ///< The name of the currently hovered button
    std::string pressedButtonName = "";   ///< The name of the currently pressed button
    
    // Persistent level selection state
    std::string selectedLevelButtonName = "";
    std::string selectedLevelScenePath = "";

    bool pendingSceneChange = false;
    float pendingTimer = 0.0f;
    bool creditsWerePlayed = false;       ///< Flag to restart BGM after credits finish

    // ===== Replay =====
    std::vector<std::string> m_replayNames;   ///< Base names of replay files (newest-first).
    std::vector<std::string> m_replayLevels;  ///< Level name stored inside each replay.
    std::string              m_selectedReplay; ///< Replay queued for load; consumed on scene change.

    /**
     * @brief Scans ../Assets/Replay/ and fills m_replayNames sorted newest-first.
     */
    void ScanReplays();

    // ===== Leaderboard =====
    struct LeaderboardEntry {
        std::string name; 
        float time; 
        float rating = 0.0f;  
        int shells = 0;       
    };
    std::vector<LeaderboardEntry> m_leaderboardData;
    int                           m_leaderboardLevel = 1;  ///< Which level tab (1/2/3) is shown.
    int                           m_selectedRank     = -1; ///< Which rank row is selected (0/1/2), -1 = none.

    /** @brief Scans ../Assets/Replay/Level_X/ and fills m_leaderboardData sorted by bestTime. */
    void ScanLeaderboard(int level);

    /** @brief Writes m_leaderboardData into the Name/Time TextComponent placeholders. */
    void PopulateLeaderboard();

    /** @brief Updates level tab and rank row textures to reflect current selections. */
    void UpdateLeaderboardTextures();

    //Best stats per level 
    LeaderboardEntry  m_level1Best;
    LeaderboardEntry  m_level2Best;
    LeaderboardEntry  m_level3Best;

    bool m_levelSelectDataLoaded = false;

    // Slider state tracking
    struct SliderState {
        float originalScaleX = 0.0f;
        float originalScaleY = 0.0f;
        float originalCenterX = 0.0f;
        float originalCenterY = 0.0f;
        float leftEdge = 0.0f;
        float rightEdge = 0.0f;
        bool uvBoundsReady = false;
        float uvBarMin = 0.0f;
        float uvBarMax = 1.0f;
        float uvBarWidth = 1.0f;
        float currentVolume = 1.0f;
    };

    SliderState masterSlider;
    SliderState musicSlider;
    SliderState sfxSlider;

    bool isDraggingSlider = false;
    std::string currentSliderCategory = "";  // "master", "music", or "sfx"
    bool justEnteredSettings = false;  // prevents slider input on first frame
    bool wasMouseDownOnSlider = false;  // tracks previous mouse state for click detection

    // save last known volumes to restore visuals
    float savedMasterVolume = 1.0f;
    float savedMusicVolume = 1.0f;
    float savedSfxVolume = 1.0f;

    /**
     * @brief Checks if the mouse cursor is over a button GameObject.
     * @param button The button GameObject to check.
     * @param mouseX The current mouse X coordinate.
     * @param mouseY The current mouse Y coordinate.
     * @return True if the mouse is over the button, false otherwise.
     */
    bool IsMouseOver(GameObject* button, double mouseX, double mouseY);

    /**
     * @brief Sets the texture of a button based on its hover and pressed state.
     * @param buttonName The name of the button to update.
     * @param isHovered Whether the button is currently being hovered.
     * @param isPressed Whether the button is currently being pressed.
     */
    void SetButtonTexture(std::string buttonName, bool isHovered, bool isPressed = false);

    /**
     * @brief Computes the UV bounds for a slider bar by analyzing the texture.
     * @param renderComp The render component containing the texture.
     * @param sliderState The slider state to update with UV bounds.
     */
    void ComputeSliderUVBounds(RenderComponent* renderComp, SliderState& sliderState);

    /**
     * @brief Updates a slider's visual appearance based on volume percentage.
     * @param barName Name of the slider bar GameObject.
     * @param sliderState The slider state to update.
     * @param volumePercent Volume percentage (0.0 to 1.0).
     */
    void UpdateSlider(const std::string& barName, SliderState& sliderState, float volumePercent);

    /**
     * @brief Handles mouse interaction with audio sliders.
     * @param mouseX Current mouse X position.
     * @param mouseY Current mouse Y position.
     * @param isMouseDown Whether mouse button is pressed.
     */
    void HandleSliderInput(double mouseX, double mouseY, bool isMouseDown);

     /**
     * @brief Returns the button currently under the mouse for the active menu screen.
     */
    std::string FindHoveredButton(double mouseX, double mouseY);

    /**
     * @brief Plays the correct UI sound for the clicked button.
     */
    void TriggerUISound(const std::string& buttonName);

    void ClearLevelSelection();

    /**
     * @brief Handles level select screen buttons only.
     * Returns true if the click was handled here.
     */
    bool HandleLevelSelectClick(const std::string& buttonName);

    /**
     * @brief Handles buttons that should act immediately instead of using delayed scene change.
     * Returns true if the click was handled here.
     */
    bool HandleImmediateButtonClick(const std::string& buttonName);
};