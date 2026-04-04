/* Start Header ************************************************************************/
/*!
\file       ReplayPlayer.cpp
\author     Jonathan Fam Qian Wei
\par        Fam.q@digipen.edu
\date       2025
Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*/
/* End Header **************************************************************************/

#include "ReplayPlayer.h"
#include "Factory.h"
#include "GameObject.h"
#include "transform.h"
#include "Animator.hpp"
#include <algorithm>
#include <cmath>

void ReplayPlayer::StartPlayback(const std::string& ghostObjectName, const GhostRecording& recording)
{
    m_ghostName = ghostObjectName;
    m_recording = &recording;
    m_clock     = 0.0;
    m_isPlaying = !recording.empty();

    // Auto-detect tile widths for each bg layer by reading the current offset of its _L copy.
    // The relationship posX_center - posX_L == tileWidth is always maintained by ParallaxBackground.
    for (int i = 0; i < 4; ++i)
    {
        m_bgTileWidths[i] = 0.f;
        if (m_bgLayers[i].empty()) continue;
        GameObject* center = GameObjectFactory::Instance().FindGameObject(m_bgLayers[i]);
        GameObject* left   = GameObjectFactory::Instance().FindGameObject(m_bgLayers[i] + "_L");
        if (center && left)
        {
            auto* tfC = center->GetComponent<TransformComponent>();
            auto* tfL = left->GetComponent<TransformComponent>();
            if (tfC && tfL)
            {
                m_bgTileWidths[i] = tfC->posX - tfL->posX;
                std::cout << "[ReplayPlayer] bg layer '" << m_bgLayers[i]
                          << "' tileWidth=" << m_bgTileWidths[i] << "\n";
            }
        }
    }
}

void ReplayPlayer::StopPlayback()
{
    m_isPlaying = false;
    m_isPaused  = false;
    m_recording = nullptr;
}

void ReplayPlayer::TogglePause()
{
    if (m_isPlaying)
        m_isPaused = !m_isPaused;
}

void ReplayPlayer::SeekAbsolute(float t)
{
    if (!m_recording || m_recording->empty())
        return;

    m_clock = t;
    if (m_clock < 0.0f)
        m_clock = 0.0f;
    else if (m_clock > m_recording->back().timestamp)
        m_clock = m_recording->back().timestamp;
}

void ReplayPlayer::Seek(float deltaSeconds)
{
    if (!m_recording || m_recording->empty())
        return;

    m_clock += deltaSeconds;

    if (m_clock < 0.0f)
        m_clock = 0.0f;
    else if (m_clock > m_recording->back().timestamp)
        m_clock = m_recording->back().timestamp;
}

void ReplayPlayer::Update(float dt)
{
    if (!m_isPlaying || !m_recording || m_recording->empty())
        return;

    const GhostRecording& rec = *m_recording;

    if (!m_isPaused)
    {
        m_clock += dt;

        // Recording ended — clamp and stop advancing (ghost stays visible at end)
        if (m_clock >= rec.back().timestamp)
            m_clock = rec.back().timestamp;
    }

    // Binary search for the last keyframe with timestamp <= clock (keyframe A)
    // Lower bound gives first element > clock, so we step back one.
    float clockF = static_cast<float>(m_clock);
    auto it = std::upper_bound(rec.begin(), rec.end(), clockF,
        [](float t, const GhostKeyframe& kf) { return t < kf.timestamp; });

    // Clamp A to valid range
    if (it == rec.begin())
        it = rec.begin();
    else
        --it;

    const GhostKeyframe& A = *it;

    // Find the next keyframe B (may not exist at the very end)
    auto nextIt = it + 1;
    bool hasB = (nextIt != rec.end());

    // --- Position: lerp between A and B ---
    float posX = A.posX;
    float posY = A.posY;

    if (hasB)
    {
        const GhostKeyframe& B = *nextIt;
        float span = B.timestamp - A.timestamp;
        float t    = (span > 0.0f) ? (clockF - A.timestamp) / span : 0.0f;
        t = (t < 0.0f) ? 0.0f : (t > 1.0f) ? 1.0f : t;

        posX = A.posX + (B.posX - A.posX) * t;
        posY = A.posY + (B.posY - A.posY) * t;
    }

    // --- Apply to ghost GameObject ---
    GameObject* ghost = GameObjectFactory::Instance().FindGameObject(m_ghostName);
    if (!ghost)
        return;

    // Transform — position lerped, scale snapped
    auto* transform = ghost->GetComponent<TransformComponent>();
    if (transform)
    {
        transform->SetPosition(posX, posY);
        transform->SetScale(A.scaleX, A.scaleY); // snap (preserves facing direction flip)
    }

    // Animation — snap state and exact frame
    auto* sprite = ghost->GetComponent<Spritesheet>();
    if (sprite && !A.animName.empty())
    {
        // Only switch animation if it changed (avoids restarting the same clip)
        if (sprite->GetCurrentAnimationName() != A.animName)
            sprite->PlayAnimation(A.animName, true);

        sprite->SetCurrentFrame(A.animRow, A.animCol);
    }

    // VFX companion — position + animation driven from recorded VFX state
    GameObject* vfxObj = GameObjectFactory::Instance().FindGameObject(m_ghostName + "_vfx");
    if (vfxObj)
    {
        auto* vfxTransform = vfxObj->GetComponent<TransformComponent>();
        if (vfxTransform)
        {
            vfxTransform->SetPosition(posX, posY - 10.0f);
            // Mirror facing direction from ghost scale sign
            float vfxSx, vfxSy;
            vfxTransform->GetScale(vfxSx, vfxSy);
            float sign = (A.scaleX >= 0.0f) ? 1.0f : -1.0f;
            vfxTransform->SetScale(std::abs(vfxSx) * sign, vfxSy);
        }

        if (!A.vfxAnimName.empty())
        {
            auto* vfxSprite = vfxObj->GetComponent<Spritesheet>();
            if (vfxSprite)
            {
                if (vfxSprite->GetCurrentAnimationName() != A.vfxAnimName)
                    vfxSprite->PlayAnimation(A.vfxAnimName, true);
                vfxSprite->SetCurrentFrame(A.vfxAnimRow, A.vfxAnimCol);
            }
        }
    }

    // Background layers — override parallax position with the recorded value.
    // Drives center, _L and _R tiling copies. Runs after scripts so it takes priority
    // over ParallaxBackground's camera-based calculation.
    for (int i = 0; i < 4; ++i)
    {
        if (m_bgLayers[i].empty()) continue;
        float bx = A.bgLayerX[i];
        float by = A.bgLayerY[i];
        float tw = m_bgTileWidths[i];

        auto setPos = [](GameObject* obj, float x, float y)
        {
            if (!obj) return;
            auto* tf = obj->GetComponent<TransformComponent>();
            if (tf) tf->SetPosition(x, y);
        };

        setPos(GameObjectFactory::Instance().FindGameObject(m_bgLayers[i]),        bx,      by);
        setPos(GameObjectFactory::Instance().FindGameObject(m_bgLayers[i] + "_L"), bx - tw, by);
        setPos(GameObjectFactory::Instance().FindGameObject(m_bgLayers[i] + "_R"), bx + tw, by);
    }
}
