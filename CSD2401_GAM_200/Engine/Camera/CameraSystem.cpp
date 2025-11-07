/* Start Header ************************************************************************/
/*!
\file       CameraSystem.cpp
\author     Guan Shao Jun [100%]
\par        shaojun.g@digipen.edu
\date       November 07 2025

Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*/
/* End Header **************************************************************************/

#include "CameraSystem.h"
#include "factory.h" // Assumed to contain GameObjectFactory definition

/**
 * @brief The main update function for the CameraSystem, called every frame.
 *
 * @details This function iterates over all GameObjects in the scene. For each
 * GameObject that is active and possesses an enabled CameraComponent, it calls
 * the component's `Update` method to execute the camera's movement logic
 * (e.g., following a target, applying smoothing, and clamping bounds).
 *
 * @param dt The time elapsed since the last frame (delta time).
 */
void CameraSystem::Update(float dt) {
	// 1. Retrieve the list of all active GameObjects from the factory/manager.
	auto& gameObjects = GameObjectFactory::Instance().GetAllGameObjects();

	// 2. Iterate through every GameObject.
	for (auto& obj : gameObjects) {
		// Skip processing if the GameObject itself is inactive.
		if (!obj->IsActive()) {
			continue;
		}

		// 3. Attempt to retrieve the CameraComponent.
		auto* camera = obj->GetComponent<CameraComponent>();

		// 4. Check if a CameraComponent exists and is enabled.
		if (camera && camera->IsEnabled()) {
			// 5. Execute the component's update logic (handles follow, smooth, and bounds).
			camera->Update(dt);
		}
	}
}