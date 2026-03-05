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
 * @brief One sampled frame of player state for ghost playback.
 *        Captured after the game update so it reflects the frame's result.
 */
struct GhostKeyframe
{
    float timestamp; // seconds since recording started

    float posX, posY;     // world position
    float scaleX, scaleY; // transform scale (negative scaleX = facing left)

    std::string animName; // registered animation name (empty if no Spritesheet)
    int animRow, animCol; // exact spritesheet frame within that animation
};

/** @brief A full run recording — ordered by timestamp ascending. */
using GhostRecording = std::vector<GhostKeyframe>;
