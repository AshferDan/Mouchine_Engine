/* Start Header ************************************************************************/
/*!
\file       tranformSystem.h
\author     Jonathan Fam [100% - 48 lines]

\par        fam.q@digipen.edu
\date       October, 15, 2025

Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*/
/* End Header **************************************************************************/

#pragma once

#include "transform.h"
#include "Component.h"
#include "System.h"
#include "core.h"
#include "factory.h"
#include "gameObject.h"

class TransformSystem : public ISystem {
public:
	TransformSystem();

	void Initialize() override {}

	void Update(float dt) override;

	std::string GetName() override { return "TransformSystem"; } ;

	~TransformSystem() override;

    /**
     * @brief Integrates velocity into position
     * @param physics Physics component with velocity
     * @param transform Transform to update
     * @param deltaTime Frame time
     */
    void IntegrateVelocity(PhysicsComponent* physics, TransformComponent* transform, float deltaTime);

};