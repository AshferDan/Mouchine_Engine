/* Start Header ************************************************************************/
/*!
\file       GameTimerSystem.h
\author     Leck Xin Yee [100% - ??? lines]
\par        xinyee.leck@digipen.edu
\date       November, 26, 2025

Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*/
/* End Header **************************************************************************/

#pragma once

#include "core.h"
#include "factory.h"
#include "LevelEditor.h"
#include "System.h"
#include "TextComponent.h"

#include <string>

class GameTimerSystem : public ISystem
{
public:
    GameTimerSystem() = default;
    ~GameTimerSystem() = default;

    void Initialize() override;
    void Update(float dt) override;
    std::string GetName() override { return "GameTimerSystem"; }
    void ResetTimer();
    float GetRemainingTime() const { return remainingTime; }
    void SetTimerObjectName(const std::string& name) { timerObjectName = name; }
    void DecreaseTime(float amount); // decrease timer by ...amount
    bool IsTimeUp() const { return remainingTime <= 0.0f; }

private:
    float       remainingTime = 30.0f;              // remaining time in 30 seconds for now
    int         lastDisplayedSeconds = -1;          // last whole second shown on HUD
    std::string timerObjectName = "LevelTimerText";
    bool        hasTriggeredTimeUp = false;         // prevent multiple scene loads

    void UpdateText(int totalSeconds);
};
