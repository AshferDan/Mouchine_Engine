/* Start Header ************************************************************************/
/*!
\file       CameraManager.cpp
\author     Guan Shao Jun [100%]
\par        shaojun.g@digipen.edu
\date       November 07 2025

Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*/
/* End Header **************************************************************************/

#include "CameraManager.h"
#include <iostream>
// Include for TransformComponent, assumed to be needed by ResetState's logic
#include "transform.h" 

/**
 * @brief Retrieves the single instance of the CameraManager (Singleton access).
 * @details This ensures only one instance of the manager exists throughout the engine lifecycle.
 * @return A reference to the CameraManager instance.
 */
CameraManager& CameraManager::Instance() {
	static CameraManager instance; // Static local variable initializes on first call (thread-safe in C++11 and later)
	return instance;
}

/**
 * @brief Sets a GameObject with a CameraComponent as the active camera.
 * @details Validates the object and component presence before assignment.
 * @param cameraObject The GameObject intended to be the main camera.
 */
void CameraManager::SetActiveCamera(GameObject* cameraObject) {
	// 1. Validation check: Null object provided
	if (!cameraObject) {
		std::cerr << "Warning: Trying to set Null camera object as active" << std::endl;
		activeCameraObject = nullptr;
		return;
	}

	// 2. Validation check: Check for the required CameraComponent
	auto* camComponent = cameraObject->GetComponent<CameraComponent>();
	if (!camComponent) {
		std::cerr << "Warning: GameObject does not have a CameraComponent" << std::endl;
		activeCameraObject = nullptr;
		return;
	}

	// Success: Set the active camera object
	activeCameraObject = cameraObject;
	std::cout << "Active Camera set to: " << cameraObject->GetName() << std::endl;
}

/**
 * @brief Retrieves the CameraComponent of the active camera GameObject.
 * @return Pointer to the active CameraComponent, or nullptr if none is set or the object is inactive.
 */
CameraComponent* CameraManager::GetActiveCamera() {
	// Return nullptr if no camera is set or the camera GameObject is currently inactive
	if (!activeCameraObject || !activeCameraObject->IsActive()) {
		return nullptr;
	}

	// Safely retrieve and return the component
	return activeCameraObject->GetComponent<CameraComponent>();
};

/**
 * @brief Retrieves the combined View-Projection matrix for rendering.
 * @param screenWidth The current width of the viewport.
 * @param screenHeight The current height of the viewport.
 * @return The 4x4 View-Projection matrix. Returns a fallback Orthographic matrix if no camera is active.
 */
mat4 CameraManager::GetViewProjectMatrix(float screenWidth, float screenHeight) {
	auto* camera = GetActiveCamera();

	if (camera && camera->IsEnabled()) {
		// Use the active camera's matrix calculation
		return camera->GetViewProjectionMatrix(screenWidth, screenHeight);
	}

	// Fallback: Return a simple, un-transformed orthographic matrix
	// This ensures rendering can still happen even without an explicitly set camera.
	return mat4::ortho(0.f, screenWidth, 0.f, screenHeight);
}

/**
 * @brief Retrieves the position of the active camera in world space.
 * @return The camera's position as a vec2, or a zero vector if no camera is active.
 */
vec2 CameraManager::GetCameraPosition() {
	auto* camera = GetActiveCamera();
	if (camera) {
		return camera->GetPosition();
	}

	// Fallback: Return origin if no active camera component exists
	return vec2(0.f, 0.f);
}

/**
 * @brief Checks if there is currently an active camera set and if the object is active.
 * @return True if `activeCameraObject` is not null and is active.
 */
bool CameraManager::HasActiveCamera() const {
	// Check if the pointer exists AND if the GameObject itself is currently active
	return activeCameraObject != nullptr && activeCameraObject->IsActive();
}

/**
 * @brief Clears the current active camera reference.
 */
void CameraManager::ClearActiveCamera() {
	activeCameraObject = nullptr;
	std::cout << "[CameraManager] Active camera cleared\n";
}

/**
 * @brief Resets the camera state, attempting to find or create a default camera and setting up tracking.
 * @details This function is typically called when loading a new level or resetting the game.
 */
void CameraManager::ResetState() {
	auto& factory = GameObjectFactory::Instance();
	ClearActiveCamera(); // Always start by clearing the current camera

	// 1. Try to find an existing camera from the loaded scene
	GameObject* activeCamObj = nullptr;
	for (auto& obj : factory.GetAllGameObjects()) {
		// Check if the object has a CameraComponent
		if (obj->GetComponent<CameraComponent>()) {
			activeCamObj = obj.get();
			// Prioritize objects named "MainCamera" and stop searching if found
			if (obj->GetName() == "MainCamera") break;
		}
	}

	// 2. If NO camera found, CREATE one (Fallback)
	if (!activeCamObj) {
		std::cout << "[CameraManager] No camera found. Creating default 'MainCamera'.\n";
		// Create the new object
		activeCamObj = factory.CreateGameObject("MainCamera");
		// Add necessary components (CameraComponent is mandatory)
		activeCamObj->AddComponent<CameraComponent>();
		// Add Transform if your system requires it for all objects
		// This ensures the camera has a world position to reference.
		activeCamObj->AddComponent<TransformComponent>();
	}

	// 3. Set the identified/created camera as active
	SetActiveCamera(activeCamObj);
	CameraComponent* camComp = GetActiveCamera();

	// 4. Find Hero and set to follow
	GameObject* hero = factory.FindGameObject("Hero");
	if (hero && camComp) {
		std::cout << "[CameraManager] Setting camera to follow 'Hero'.\n";
		// Configure the camera to follow the hero
		camComp->SetMode(CameraComponent::CameraMode::FOLLOW_TARGET);
		camComp->SetFollowTarget(hero);
		camComp->SetFollowOffset({ 0.f, 0.f });
		camComp->SetSmoothing(true, 5.f);

		// Initial position snap: set camera position to hero's current position
		if (auto* heroTx = hero->GetComponent<TransformComponent>()) {
			// Assumes TransformComponent has public members or getters for position
			camComp->SetPosition({ heroTx->posX, heroTx->posY });
		}
	}
	// 5. If no Hero is found, ensure the camera is in a stable, fixed state
	else if (camComp) {
		camComp->SetMode(CameraComponent::CameraMode::FIXED);
		camComp->SetPosition(0.f, 0.f);
	}
}

void CameraManager::AddCameraTrauma(float amount) {
	if (activeCameraObject) {
		auto* cam = activeCameraObject->GetComponent<CameraComponent>();
		if (cam) {
			cam->AddTrauma(amount);
		}
	}
}

void CameraManager::FlashActiveCamera(float duration, const vec4& color, bool fadeOut) {
	if (activeCameraObject) {
		auto* cam = activeCameraObject->GetComponent<CameraComponent>();
		if (cam) {
			cam->StartFlash(duration, color, fadeOut);
		}
	}
}