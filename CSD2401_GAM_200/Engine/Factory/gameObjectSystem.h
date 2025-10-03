
/* Start Header ************************************************************************/
/*!
\file       gameObjectSystem.h
\author     Jonathan Fam Qian Wei [100% - 12 lines]
\par        Fam.q@digipen.edu
\date       October, 2, 2025

Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*/
/* End Header **************************************************************************/
#pragma once
#include "System.h"

class GameObjectFactory;

class GameObjectSystem : public ISystem {
public:
    GameObjectSystem();

    void Update(float timeslice) override;
    std::string GetName() override { return "GameObjectSystem"; }
    void Initialize() override;

private:
    GameObjectFactory& factory;
};