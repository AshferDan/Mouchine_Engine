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

    /** @brief Stop playback and release the recording reference. */
    void StopPlayback();

    /**
     * @brief Advance the playback clock and update the ghost object.
     *        Call once per fixed-timestep tick.
     * @param dt Fixed delta time for this tick.
     */
    void Update(float dt);

    /** @return True while actively playing back. */
    bool IsPlaying() const { return m_isPlaying; }

private:
    std::string           m_ghostName;
    const GhostRecording* m_recording  = nullptr;
    float                 m_clock      = 0.0f;
    bool                  m_isPlaying  = false;
};
