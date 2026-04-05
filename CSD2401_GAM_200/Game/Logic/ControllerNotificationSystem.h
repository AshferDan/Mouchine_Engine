/* Start Header ************************************************************************/
/*!
\file       ControllerNotificationSystem.h
\date       2026
Copyright (C) 2025 DigiPen Institute of Technology.
*/
/* End Header **************************************************************************/

#pragma once
#include "System.h"
#include "gameObject.h"

/**
 * @class ControllerNotificationSystem
 * @brief Shows a brief on-screen popup when a controller is connected or disconnected.
 *        Dismissed automatically after 2.5s, or immediately on any key / button press.
 */
class ControllerNotificationSystem : public ISystem
{
public:
    ControllerNotificationSystem() = default;
    ~ControllerNotificationSystem() override { ClearPointers(); }

    void Initialize() override;
    void Update(float dt) override;
    std::string GetName() override { return "ControllerNotificationSystem"; }

private:
    void ShowPopup(const std::string& message);
    void DestroyPopup();
    void ClearPointers(); // null out stored pointers without destroying (after scene reload)

    bool        m_active       = false;
    float       m_timer        = 0.0f;

    static constexpr float kDuration     = 2.5f;  // total display time
    static constexpr float kFadeDuration = 0.5f;  // fade-out phase at the end
    static constexpr float kDismissGrace = 0.3f;  // input is ignored for this long after popup opens

    GameObject* m_background = nullptr;
    GameObject* m_text       = nullptr;
};
