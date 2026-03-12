/* Start Header ************************************************************************/
/*!
\file       PauseMenuLogic.h
\author     Leck Xin Yee [100% - 57 lines]
\par        xinyee.leck@digipen.edu
\date       November, 28, 2025

Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*/
/* End Header **************************************************************************/

#pragma once
#include "System.h"
#include "factory.h"
#include "InputManager.h"
#include "transform.h"
#include "MathLib.h"
#include "GraphicsSystem.h"

class GraphicsSystem;

/**
 * @enum ButtonState
 * @brief Represents the visual state of a button
 */
enum class ButtonState {
    Normal,   // Default appearance
    Hover,    // Mouse over (light)
    Pressed   // Being clicked (dark)
};

/**
 * @class PauseMenuLogicSystem
 * @brief Handles pause menu UI creation, visibility, interactions, and scene actions.
 */
class PauseMenuLogicSystem : public ISystem {
public:
    /**
     * @brief Default constructor for the pause menu system.
     */
    PauseMenuLogicSystem() = default;

    /**
     * @brief Default destructor.
     */
    ~PauseMenuLogicSystem() = default;

    /**
     * @brief Initializes the pause menu state.
     */
    void Initialize() override;

    /**
     * @brief Updates pause menu logic every frame, handling UI movement, hover/click logic, and pending scene transitions.
     * @param dt Delta time in seconds.
     */
    void Update(float dt) override;

    /**
     * @brief Returns the system name.
     * @return "PauseMenuLogicSystem"
     */
    std::string GetName() override { return "PauseMenuLogicSystem"; }


    /**
     * @brief Checks whether the pause menu is currently visible.
     * @return True if the pause menu is active.
     */
    bool IsPauseMenuActive() const { return isPauseMenuActive; }

    /**
     * @brief Sets the current scene path so Restart can reload the same scene.
     * @param scenePath File path of the scene.
     */
    void SetCurrentScene(const std::string& scenePath) { currentScenePath = scenePath; }

    /**
     * @brief Gets the current scene path.
     * @return Current scene file path.
     */
    std::string GetCurrentScene() const { return currentScenePath; }

    /**
     * @brief Toggles the pause menu on/off and pauses/unpauses the game.
     */
    void TogglePauseMenu();

    /**
     * @brief Opens the How To Play screen (creates pause menu if needed).
     */
    void OpenHowToPlay();

    /**
     * @brief Closes the How To Play screen.
     */
    void CloseHowToPlay();

    /**
     * @brief Checks if How To Play screen is currently open.
     * @return True if How To Play is active.
     */
    bool IsHowToPlayOpen() const { return isHowToPlayOpen; }

    /**
     * @brief Checks if the pause settings screen is currently open.
     * @return True if settings is active.
     */
    bool IsPauseSettingsOpen() const { return isPauseSettingsOpen; }

    /**
     * @brief Closes the pause settings screen.
     */
    void ClosePauseSettings();

private:
    bool isPauseMenuActive = false;        // whether the pause menu is currently displayed
    std::string hoveredButtonName = "";    // name of the currently hovered button
    bool hasCreatedUI = false;             // whether UI elements have been created
    bool pendingSceneChange = false;       // whether a scene change is pending
    float pendingTimer = 0.0f;             // timer for scene change delay
    std::string currentScenePath = "";
    bool isPauseSettingsOpen = false;      // whether pause settings screen is open
    bool hasCreatedSettingsUI = false;     // whether settings UI has been created
    bool wasSettingsOpenLastFrame = false; // tracks if settings was open last frame to avoid redundant updates
    bool pendingSettingsClose = false;     // whether settings back button was clicked and waiting to close
    float settingsCloseTimer = 0.0f;       // timer for settings close delay
    bool isHowToPlayOpen = false;          // whether how to play screen is open
    bool hasCreatedHowToPlayUI = false;    // whether how to play UI has been created
    bool wasHowToPlayOpenLastFrame = false; // tracks if how to play was open last frame
    bool pendingHowToPlayClose = false;    // whether how to play back button was clicked
    float howToPlayCloseTimer = 0.0f;      // timer for how to play close delay
    bool howToPlayOpenedViaTab = false;    // tracks if how to play was opened via TAB key

    /**
     * @brief Creates all pause menu UI elements (overlay, buttons, title).
     */
    void CreatePauseMenuUI();

    /**
     * @brief Creates settings screen UI elements for pause menu.
     */
    void CreatePauseSettingsUI();

    /**
     * @brief Destroys all pause menu UI objects.
     */
    void DestroyPauseMenuUI();

    /**
     * @brief Destroys pause settings UI objects.
     */
    void DestroyPauseSettingsUI();

    /**
     * @brief Sets all pause menu UI objects active or inactive.
     * @param show Whether to enable or disable UI visibility.
     */
    void SetPauseMenuVisible(bool show);

    /**
     * @brief Sets all pause settings UI objects active or inactive.
     * @param show Whether to enable or disable settings UI visibility.
     */
    void SetPauseSettingsVisible(bool show);

    /**
     * @brief Creates how to play screen UI.
     */
    void CreateHowToPlayUI();

    /**
     * @brief Destroys how to play UI objects.
     */
    void DestroyHowToPlayUI();

    /**
     * @brief Sets how to play UI objects active or inactive.
     * @param show Whether to enable or disable how to play UI visibility.
     */
    void SetHowToPlayVisible(bool show);

    /**
     * @brief Checks if the mouse cursor is within a button's area.
     * @param button The GameObject representing a UI button.
     * @param mouseX Mouse world X coordinate.
     * @param mouseY Mouse world Y coordinate.
     * @return True if the mouse is over the button.
     */
    bool IsMouseOver(GameObject* button, double mouseX, double mouseY);

    /**
     * @brief Changes a button's texture depending on button state.
     * @param buttonName Name of the button object.
     * @param state The visual state (Normal, Hover, or Pressed).
     */
    void SetButtonTexture(std::string buttonName, ButtonState state);
};
