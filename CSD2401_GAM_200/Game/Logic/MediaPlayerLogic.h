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
    bool m_spawned        = false;  ///< True while Media_Player prefab is live
    bool m_mouseWasDown   = false;  ///< Previous-frame LMB state for press/release edge detection
    bool m_sliderDragging = false;  ///< True while the user is dragging the timeline scrubber

    /**
     * @brief Loads the Media_Player prefab into the scene.
     */
    void SpawnMediaPlayer();

    /**
     * @brief Destroys all Media_Player prefab objects from the scene.
     */
    void DestroyMediaPlayer();

    /**
     * @brief Repositions UI objects to follow the camera, updates the slider thumb,
     *        and handles all mouse input for the media player controls.
     */
    void UpdateMediaPlayer();
};
