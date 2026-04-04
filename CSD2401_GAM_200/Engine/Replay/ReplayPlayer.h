/* Start Header ************************************************************************/
/*!
\file       ReplayPlayer.h
\author     Jonathan Fam Qian Wei
\par        Fam.q@digipen.edu
\date       2025
Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*/
/* End Header **************************************************************************/

#pragma once
#include "ReplayData.h"
#include <string>

/**
 * @brief Drives a ghost GameObject from a GhostRecording.
 *
 *  - Position:        linearly interpolated between the two surrounding keyframes.
 *  - Scale / facing:  snapped to the nearest past keyframe (no lerp — avoids mid-flip weirdness).
 *  - Animation state: snapped to the nearest past keyframe (no lerp).
 *  - Animation frame: snapped (SetCurrentFrame) so the ghost shows the exact recorded frame.
 *
 * The ghost object must already exist in the scene. The player adds no collision
 * so the ghost passes through walls as expected.
 */
class ReplayPlayer
{
public:
    /**
     * @brief Begin driving the named ghost object from the supplied recording.
     * @param ghostObjectName Name of the ghost GameObject in the scene.
     * @param recording       The recording to play back (must outlive this call's Update loop).
     */
    void StartPlayback(const std::string& ghostObjectName, const GhostRecording& recording);

    /**
     * @brief Stops playback and releases the recording reference.
     */
    void StopPlayback();

    /**
     * @brief Advances the playback clock and updates the ghost object's state.
     *        Should be called once per fixed-timestep tick.
     * @param dt Fixed delta time for this tick.
     */
    void Update(float dt);

    /**
     * @brief Toggles the pause state of playback.
     *        When paused, the clock stops advancing but the ghost remains visible.
     */
    void TogglePause();

    /**
     * @brief Shifts the playback clock by the specified delta time.
     *        Clamps to [0, recording end]. Safe to call while paused.
     * @param deltaSeconds The amount of time to shift (negative to rewind).
     */
    void Seek(float deltaSeconds);

    /**
     * @brief Checks if playback is currently active.
     * @return True while actively playing back (even if paused).
     */
    bool IsPlaying() const { return m_isPlaying; }

    /**
     * @brief Checks if playback is currently paused.
     * @return True if playback is paused.
     */
    bool IsPaused()  const { return m_isPaused; }

    /**
     * @brief Returns the current playback clock time in seconds.
     * @return Current time in seconds.
     */
    float GetClock() const { return static_cast<float>(m_clock); }

    /**
     * @brief Returns the total duration of the recording in seconds.
     * @return Duration in seconds, or 0 if no recording.
     */
    float GetDuration() const
    {
        return (m_recording && !m_recording->empty()) ? m_recording->back().timestamp : 0.0f;
    }

    /**
     * @brief Sets the background layer object names to drive from the recording.
     *        Pass empty strings for unused slots.
     *        Call before StartPlayback so the player knows which objects to update.
     */
    void SetBgLayers(const std::string& b0, const std::string& b1,
                     const std::string& b2, const std::string& b3)
    {
        m_bgLayers[0] = b0; m_bgLayers[1] = b1;
        m_bgLayers[2] = b2; m_bgLayers[3] = b3;
    }

    /**
     * @brief Seeks to an absolute time position in the recording.
     *        Clamps to [0, duration]. Safe to call while paused.
     * @param t Target time in seconds.
     */
    void SeekAbsolute(float t);

private:
    std::string           m_ghostName;
    const GhostRecording* m_recording  = nullptr;
    double                m_clock      = 0.0;  // double avoids float precision loss over long sessions
    bool                  m_isPlaying  = false;
    bool                  m_isPaused   = false;
    std::string           m_bgLayers[4];        // background layer object names (up to 4)
    float                 m_bgTileWidths[4] = {0.f, 0.f, 0.f, 0.f}; // auto-detected from _L copy offset
};
