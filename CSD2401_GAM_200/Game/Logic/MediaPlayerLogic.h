/* Start Header ************************************************************************/
/*!
\file       MediaPlayerLogic.h
\author     Jonathan Fam Qian Wei
\par        Fam.q@digipen.edu
\date       2026

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
#include "CameraManager.h"
#include "GraphicsSystem.h"
#include "RenderComponent.h"

/**
 * @class MediaPlayerLogicSystem
 * @brief Game-side system that drives the Media_Player UI during replay watch mode.
 *
 * Spawns/destroys the Media_Player prefab automatically based on GhostSystem's
 * replay-mode state, and handles mouse interaction for the play/pause, seek bar,
 * forward (+10 s), backward (-10 s), and exit-replay (ReplayVideo) buttons.
 *
 * Usage:
 *   Register this system in the game the same way as PauseMenuLogicSystem.
 *   It reads GhostSystem state each frame via CORE->GetGhostSystem().
 */
class MediaPlayerLogicSystem : public ISystem
{
public:
    MediaPlayerLogicSystem() = default;
    ~MediaPlayerLogicSystem() = default;

    /**
     * @brief Resets all internal state.
     */
    void Initialize() override;

    /**
     * @brief Each frame: spawns/destroys the media player UI to match replay mode,
     *        then handles button clicks and slider dragging.
     * @param dt Delta time (seconds) — not used directly but kept for ISystem contract.
     */
    void Update(float dt) override;

    /**
     * @brief Returns the system identifier.
     * @return "MediaPlayerLogicSystem"
     */
    std::string GetName() override { return "MediaPlayerLogicSystem"; }

private:
    bool m_spawned        = false;  ///< True while the Media_Player UI is visible
    bool m_mouseWasDown   = false;  ///< Previous-frame LMB state for edge detection
    bool m_sliderDragging = false;  ///< True while the user is dragging the timeline scrubber
    bool m_wasPaused      = false;  ///< Tracks last-known pause state to detect icon changes
    bool m_playHovered    = false;  ///< True when the cursor is over the play/pause button
    bool m_playPressed    = false;  ///< True while LMB is held down on the play/pause button

    /**
     * @brief Activates all Media_Player child objects in the scene.
     */
    void SpawnMediaPlayer();

    /**
     * @brief Deactivates all Media_Player child objects in the scene.
     */
    void DestroyMediaPlayer();

    /**
     * @brief Updates screen-space layout, slider thumb, and all button interactions.
     */
    void UpdateMediaPlayer();

    /**
     * @brief Swaps the Replay_playing button texture between the play and pause icons.
     * @param paused True when currently paused (show play icon), false when playing (show pause icon).
     */
    void UpdatePlayPauseIcon(bool paused);

    /**
     * @brief Activates or deactivates a named GameObject and enables/disables its
     *        TransformComponent and RenderComponent to match the desired state.
     * @param name   Name of the GameObject.
     * @param active True to activate, false to deactivate.
     */
    static void SetObjectActive(const char* name, bool active);
};
