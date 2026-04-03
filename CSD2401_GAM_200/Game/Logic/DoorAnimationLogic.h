/* Start Header ************************************************************************/
/*!
\file       DoorAnimationLogic.h
\author     Jonathan
\date       April, 2, 2026

Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*/
/* End Header **************************************************************************/

#pragma once
#include "System.h"
#include "factory.h"
#include "transform.h"
#include "ScriptAPI.h"
#include "core.h"
#include <functional>

/**
 * @brief Internal state of the door animation sequence.
 */
enum class DoorAnimState
{
    IDLE,         ///< No animation running
    CLOSING,      ///< Doors sliding in from off-screen to centre
    CLOSED_HOLD,  ///< Doors held shut (brief pause before opening)
    OPENING,      ///< Doors sliding back out to reveal the win/lose screen
    DONE          ///< Animation finished; pending cleanup
};

/**
 * @class DoorAnimationLogicSystem
 * @brief Plays the door-close/open cinematic when the player reaches the win zone.
 *
 * Sequence (matches the hand-drawn storyboard):
 *   1. CLOSING  – Door_LEFT slides in from the left, Door_RIGHT from the right.
 *   2. CLOSED   – Doors fully shut; onClosed() is invoked so callers can create
 *                 the win/lose screen while the view is blocked.
 *   3. OPENING  – Doors slide back out, revealing the win/lose screen.
 *   4. onOpened() is invoked and the door GameObjects are destroyed.
 */
class DoorAnimationLogicSystem : public ISystem
{
public:
    DoorAnimationLogicSystem()  = default;
    ~DoorAnimationLogicSystem() = default;

    void Initialize() override;
    void Update(float dt) override;
    std::string GetName() override { return "DoorAnimationLogicSystem"; }

    /**
     * @brief Spawns the door prefab and begins the animation.
     * @param onClosed   Callback fired when doors are fully closed (use for prep work).
     * @param onOpened   Callback fired when doors are fully open and destroyed.
     * @param onOpening  Optional callback fired when the OPENING phase begins —
     *                   spawn win/lose UI here so it is revealed as the doors slide out.
     */
    void StartAnimation(std::function<void()> onClosed,
                        std::function<void()> onOpened,
                        std::function<void()> onOpening = nullptr);

    /** @return True while a door animation is in progress. */
    bool IsAnimating() const
    {
        return m_state != DoorAnimState::IDLE && m_state != DoorAnimState::DONE;
    }

private:
    DoorAnimState m_state = DoorAnimState::IDLE;
    float         m_timer = 0.0f;

    // ── Timings (seconds) ───────────────────────────────────────────────────
    static constexpr float k_closeDuration = 0.7f;
    static constexpr float k_holdDuration  = 0.7f;
    static constexpr float k_openDuration  = 0.7f;

    // ── Screen-space positions for a 1920 x 1080 display ───────────────────
    static constexpr float k_leftClosedX  =  480.0f;
    static constexpr float k_leftOpenX    = -480.0f;   // off-screen left
    static constexpr float k_rightClosedX = 1440.0f;
    static constexpr float k_rightOpenX   = 2400.0f;   // off-screen right
    static constexpr float k_doorY        =  540.0f;   // vertical centre

    std::function<void()> m_onClosed;
    std::function<void()> m_onOpened;
    std::function<void()> m_onOpening;

    TransformComponent* m_leftTransform  = nullptr;
    TransformComponent* m_rightTransform = nullptr;

    // EntityIDs of the objects we spawned so DestroyDoors never hits a
    // pre-existing scene object with the same name.
    EntityID m_leftID  = UINT32_MAX;
    EntityID m_rightID = UINT32_MAX;
    EntityID m_rootID  = UINT32_MAX;

    // One-shot flags so each animation phase plays its sound exactly once
    bool m_audioClosing = true;
    bool m_audioOpening = true;

    void SpawnDoors();
    void DestroyDoors();

    /** Clamp t to [0,1] and apply a smoothstep ease-in/out curve. */
    static float SmoothStep(float t);
    static float Lerp(float a, float b, float t);
};
