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
    m_clock       = 0.0;
    m_recording.clear();
}

void ReplayRecorder::StopRecording()
{
    m_isRecording = false;
}

void ReplayRecorder::Reset()
{
    m_isRecording = false;
    m_clock       = 0.0;
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
    frame.timestamp = static_cast<float>(m_clock);

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

    // --- VFX companion (e.g. "Hero_vfx" alongside "Hero") ---
    GameObject* vfxObj = GameObjectFactory::Instance().FindGameObject(m_targetName + "_vfx");
    if (vfxObj)
    {
        auto* vfxSprite = vfxObj->GetComponent<Spritesheet>();
        if (vfxSprite)
        {
            frame.vfxAnimName = vfxSprite->GetCurrentAnimationName();
            frame.vfxAnimRow  = vfxSprite->GetCurrentRow();
            frame.vfxAnimCol  = vfxSprite->GetCurrentCol();
        }
    }

    // --- Background layers ---
    for (int i = 0; i < 4; ++i)
    {
        frame.bgLayerX[i] = 0.f;
        frame.bgLayerY[i] = 0.f;
        if (m_bgLayers[i].empty()) continue;
        GameObject* bgObj = GameObjectFactory::Instance().FindGameObject(m_bgLayers[i]);
        if (bgObj)
        {
            auto* tf = bgObj->GetComponent<TransformComponent>();
            if (tf)
            {
                frame.bgLayerX[i] = tf->posX;
                frame.bgLayerY[i] = tf->posY;
            }
        }
        else if (m_recording.size() < 3)
        {
            std::cout << "[ReplayRecorder] WARNING: bg layer '" << m_bgLayers[i] << "' not found in scene!\n";
        }
    }
    if (m_recording.size() < 3)
    {
        std::cout << "[ReplayRecorder] frame bg XY: "
                  << frame.bgLayerX[0] << "," << frame.bgLayerY[0] << "  "
                  << frame.bgLayerX[1] << "," << frame.bgLayerY[1] << "  "
                  << frame.bgLayerX[2] << "," << frame.bgLayerY[2] << "\n";
    }

    // Cap to ~15 minutes at 60 fps to prevent unbounded memory growth
    static constexpr std::size_t k_maxFrames = 54000;
    if (m_recording.size() < k_maxFrames)
        m_recording.push_back(frame);
}
