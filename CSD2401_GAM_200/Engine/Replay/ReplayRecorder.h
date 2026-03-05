/* Start Header ************************************************************************/
/*!
\file       ReplayRecorder.h
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
 * @brief Records a target GameObject's state each fixed-timestep frame.
 *        Samples are taken AFTER the game update so each frame reflects
 *        the result of that tick (position post-physics, animation post-update).
 */
class ReplayRecorder
{
public:
    /**
     * @brief Begin recording the named GameObject.
     *        Clears any previous recording and resets the clock.
     * @param targetName Name of the GameObject to track.
     */
    void StartRecording(const std::string& targetName);

    /**
     * @brief Stop recording. The current recording remains accessible via GetRecording().
     */
    void StopRecording();

    /**
     * @brief Sample the target this frame and store a keyframe.
     *        Call once per fixed-timestep tick.
     * @param dt Fixed delta time for this tick.
     */
    void Update(float dt);

    /** @brief Clear the recording buffer and reset the clock. */
    void Reset();

    /** @return True while actively recording. */
    bool IsRecording() const { return m_isRecording; }

    /** @return The recording accumulated so far. */
    const GhostRecording& GetRecording() const { return m_recording; }

private:
    std::string    m_targetName;
    GhostRecording m_recording;
    float          m_clock      = 0.0f;
    bool           m_isRecording = false;
};
