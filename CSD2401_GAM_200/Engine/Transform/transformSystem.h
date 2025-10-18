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

	std::string GetName() override { return "TransformSystems"; } ;

	~TransformSystem() override;

    /**
     * @brief Integrates velocity into position
     * @param physics Physics component with velocity
     * @param transform Transform to update
     * @param deltaTime Frame time
     */
    void IntegrateVelocity(PhysicsComponent* physics, TransformComponent* transform, float deltaTime);

};