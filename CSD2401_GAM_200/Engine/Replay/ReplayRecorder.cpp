/* Start Header ************************************************************************/
/*!
\file       ReplayRecorder.cpp
\author     Jonathan Fam Qian Wei
\par        Fam.q@digipen.edu
\date       2025
Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*/
/* End Header **************************************************************************/

#include "ReplayRecorder.h"
#include "Factory.h"
#include "GameObject.h"
#include "transform.h"
#include "Animator.hpp"

void ReplayRecorder::StartRecording(const std::string& targetName)
{
    m_targetName  = targetName;
    m_isRecording = true;
    m_clock       = 0.0f;
    m_recording.clear();
}

void ReplayRecorder::StopRecording()
{
    m_isRecording = false;
}

void ReplayRecorder::Reset()
{
    m_isRecording = false;
    m_clock       = 0.0f;
    m_recording.clear();
}

void ReplayRecorder::Update(float dt)
{
    if (!m_isRecording)
        return;

    m_clock += dt;

    GameObject* target = GameObjectFactory::Instance().FindGameObject(m_targetName);
    if (!target)
        return;

    GhostKeyframe frame;
    frame.timestamp = m_clock;

    // --- Transform ---
    auto* transform = target->GetComponent<TransformComponent>();
    if (transform)
    {
        frame.posX   = transform->posX;
        frame.posY   = transform->posY;
        frame.scaleX = transform->scaleX;
        frame.scaleY = transform->scaleY;
    }

    // --- Animation ---
    auto* sprite = target->GetComponent<Spritesheet>();
    if (sprite)
    {
        frame.animName = sprite->GetCurrentAnimationName();
        frame.animRow  = sprite->GetCurrentRow();
        frame.animCol  = sprite->GetCurrentCol();
    }
    else
    {
        frame.animRow = 0;
        frame.animCol = 0;
    }

    m_recording.push_back(frame);
}
