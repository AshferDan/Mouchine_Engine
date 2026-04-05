/* Start Header ************************************************************************/
/*!
\file       DoorAnimationLogic.cpp
\author     Jonathan
\date       April, 2, 2026

Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*/
/* End Header **************************************************************************/

#include "DoorAnimationLogic.h"
#include "Serialization.h"
#include "ScriptComponent.h"
#include "RenderComponent.h"
#include <algorithm>
#include <iostream>
#include <unordered_set>

// ── Math helpers ─────────────────────────────────────────────────────────────

float DoorAnimationLogicSystem::SmoothStep(float t)
{
    t = std::max(0.0f, std::min(t, 1.0f));
    return t * t * (3.0f - 2.0f * t);
}

float DoorAnimationLogicSystem::Lerp(float a, float b, float t)
{
    return a + (b - a) * t;
}

// ── ISystem interface ─────────────────────────────────────────────────────────

void DoorAnimationLogicSystem::Initialize()
{
    m_state          = DoorAnimState::IDLE;
    m_timer          = 0.0f;
    m_leftTransform  = nullptr;
    m_rightTransform = nullptr;
    m_onClosed       = nullptr;
    m_onOpened       = nullptr;
    m_onOpening      = nullptr;
    m_leftID         = UINT32_MAX;
    m_rightID        = UINT32_MAX;
    m_rootID         = UINT32_MAX;
    m_audioClosing   = true;
    m_audioOpening   = true;

    API::AddSound("doorClose", "../Assets/Audio/play_button.wav", false, 1.0f, 1, "sfx");
}

/**
 * @brief Advances the door animation state machine each frame.
 *
 * State transitions:
 *   CLOSING      → snap to closed positions → invoke onClosed → CLOSED_HOLD
 *   CLOSED_HOLD  → wait k_holdDuration → OPENING
 *   OPENING      → snap to open positions  → destroy doors → invoke onOpened → DONE
 *   DONE         → reset to IDLE
 */
void DoorAnimationLogicSystem::Update(float dt)
{
    if (m_state == DoorAnimState::IDLE || m_state == DoorAnimState::DONE)
        return;

    // Cap dt to 50 ms — prevents the animation skipping instantly on the first
    // frame after unpausing when the engine passes the full accumulated paused time.
    dt = std::min(dt, 0.05f);

    m_timer += dt;
    
    switch (m_state)
    {
    // ── Phase 1: slide doors in from off-screen ───────────────────────────
    case DoorAnimState::CLOSING:
    {

        if (m_audioClosing) {
            API::PlaySound("doorClose", "sfx");
            m_audioClosing = false;
        }
        
        
        float t = SmoothStep(m_timer / k_closeDuration);
        if (m_leftTransform)  m_leftTransform->posX  = Lerp(k_leftOpenX,  k_leftClosedX,  t);
        if (m_rightTransform) m_rightTransform->posX = Lerp(k_rightOpenX, k_rightClosedX, t);

        if (m_timer >= k_closeDuration)
        {
            // Snap to exact centre to avoid floating-point overshoot
            if (m_leftTransform)  m_leftTransform->posX  = k_leftClosedX;
            if (m_rightTransform) m_rightTransform->posX = k_rightClosedX;

            m_timer = 0.0f;
            m_state = DoorAnimState::CLOSED_HOLD;

            // Doors fully shut — create the win/lose screen now while view is blocked
            if (m_onClosed) m_onClosed();
            
        }
        break;
    }

    // ── Phase 2: hold with doors shut ────────────────────────────────────
    case DoorAnimState::CLOSED_HOLD:
    {
        if (m_timer >= k_holdDuration)
        {
            m_timer = 0.0f;
            m_state = DoorAnimState::OPENING;

            // Begin opening phase — win/lose screen already exists behind the doors
            if (m_onOpening) m_onOpening();
        }
        break;
    }

    // ── Phase 3: slide doors back out to reveal win/lose screen ──────────
    case DoorAnimState::OPENING:
    {
        if (m_audioOpening) {
            API::PlaySound("doorClose", "sfx");
            m_audioOpening = false;
        }
        float t = SmoothStep(m_timer / k_openDuration);
        if (m_leftTransform)  m_leftTransform->posX  = Lerp(k_leftClosedX,  k_leftOpenX,  t);
        if (m_rightTransform) m_rightTransform->posX = Lerp(k_rightClosedX, k_rightOpenX, t);

        if (m_timer >= k_openDuration)
        {
            m_timer = 0.0f;
            m_state = DoorAnimState::DONE;

            // Remove the door GameObjects; the win/lose screen is now visible
            DestroyDoors();
            if (m_onOpened) m_onOpened();
        }
        break;
    }

    default:
        break;
    }
}

// ── Public API ────────────────────────────────────────────────────────────────

void DoorAnimationLogicSystem::StartAnimation(
    std::function<void()> onClosed,
    std::function<void()> onOpened,
    std::function<void()> onOpening)
{
    m_onClosed     = std::move(onClosed);
    m_onOpened     = std::move(onOpened);
    m_onOpening    = std::move(onOpening);
    m_timer        = 0.0f;
    m_audioClosing = true;
    m_audioOpening = true;
    m_state        = DoorAnimState::CLOSING;
    SpawnDoors();
}

// ── Private helpers ───────────────────────────────────────────────────────────

void DoorAnimationLogicSystem::SpawnDoors()
{
    // Destroy any doors WE previously spawned (by ID, never by name alone)
    DestroyDoors();

    auto& factory = GameObjectFactory::Instance();

    // Snapshot existing IDs so we can identify the newly created objects
    std::unordered_set<EntityID> existingIDs;
    for (auto& obj : factory.GetAllGameObjects())
        if (obj) existingIDs.insert(obj->GetID());

    // Load the prefab; creates DOOR_ANIMATION, Door_LEFT, Door_RIGHT
    Serialization::LoadPrefab("DOOR_ANIMATION");

    // Find only the objects that did NOT exist before loading
    GameObject* leftObj  = nullptr;
    GameObject* rightObj = nullptr;
    GameObject* rootObj  = nullptr;

    for (auto& obj : factory.GetAllGameObjects())
    {
        if (!obj || existingIDs.count(obj->GetID())) continue;
        const std::string& n = obj->GetName();
        if      (n == "Door_LEFT")       leftObj  = obj.get();
        else if (n == "Door_RIGHT")      rightObj = obj.get();
        else if (n == "DOOR_ANIMATION")  rootObj  = obj.get();
    }

    // Disable the legacy C# script so it can't fight our positions
    if (rootObj)
    {
        m_rootID = rootObj->GetID();
        if (auto* sc = rootObj->GetComponent<ScriptComponent>())
            sc->SetEnabled(false);
    }

    m_leftTransform  = leftObj  ? leftObj->GetComponent<TransformComponent>()  : nullptr;
    m_rightTransform = rightObj ? rightObj->GetComponent<TransformComponent>() : nullptr;
    m_leftID  = leftObj  ? leftObj->GetID()  : UINT32_MAX;
    m_rightID = rightObj ? rightObj->GetID() : UINT32_MAX;

    if (!m_leftTransform || !m_rightTransform)
    {
        std::cerr << "[DoorAnim] WARNING: could not find newly spawned Door_LEFT or Door_RIGHT!\n";
        return;
    }

    // Mark both door panels as screen-space so they render at fixed screen
    // positions and are not affected by the game camera.
    if (auto* rc = leftObj->GetComponent<RenderComponent>())
        rc->SetScreenSpace(true);
    if (auto* rc = rightObj->GetComponent<RenderComponent>())
        rc->SetScreenSpace(true);

    // Park both panels off-screen so they slide IN during CLOSING
    m_leftTransform->posX  = k_leftOpenX;
    m_leftTransform->posY  = k_doorY;
    m_rightTransform->posX = k_rightOpenX;
    m_rightTransform->posY = k_doorY;

    std::cout << "[DoorAnim] Doors spawned (IDs " << m_leftID << ", " << m_rightID
              << ") — positioned off-screen.\n";
}

void DoorAnimationLogicSystem::DestroyDoors()
{
    auto& factory = GameObjectFactory::Instance();

    auto tryDestroyID = [&](EntityID id)
    {
        if (id != UINT32_MAX)
            factory.QueueDestroyGameObject(id);
    };

    tryDestroyID(m_leftID);
    tryDestroyID(m_rightID);
    tryDestroyID(m_rootID);

    m_leftTransform  = nullptr;
    m_rightTransform = nullptr;
    m_leftID  = UINT32_MAX;
    m_rightID = UINT32_MAX;
    m_rootID  = UINT32_MAX;
}
