/* Start Header ************************************************************************/
/*!
\file       ReplayData.h
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
#include <string>
#include <vector>

/**
 * @brief Represents a single sampled frame of player state for ghost playback.
 *        Captured after the game update to ensure it reflects the frame's final state.
 */
struct GhostKeyframe
{
    float timestamp;

    float posX, posY;
    float scaleX, scaleY;

    std::string animName;
    int animRow, animCol;

    // VFX companion object state
    std::string vfxAnimName;
    int vfxAnimRow = 0;
    int vfxAnimCol = 0;
};

/** 
 * @brief A collection of keyframes representing a full run, ordered by ascending timestamp. 
 */
using GhostRecording = std::vector<GhostKeyframe>;
