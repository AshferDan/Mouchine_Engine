/* Start Header ************************************************************************/
/*!
\file       CameraManager.h
\author     Guan Shao Jun [100%]
\par        shaojun.g@digipen.edu
\date       November 07 2025

Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*/
/* End Header **************************************************************************/

#pragma once
#include "CameraComponent.h"
#include "gameObject.h"
#include "factory.h"
#include "MathLib.h" // Assuming mat4 and vec2 are defined here

/**
 * @class CameraManager
 * @brief Singleton class responsible for tracking and managing the single active game camera.
 *
 * @details This manager provides global access to the active camera's properties,
 * such as its position and the combined View-Projection matrix needed for rendering.
 * It enforces that only one camera is considered "active" at any given time.
 */
class CameraManager {
public:
	/**
	 * @brief Retrieves the single instance of the CameraManager (Singleton access).
	 * @return A reference to the CameraManager instance.
	 */
	static CameraManager& Instance();

	/**
	 * @brief Sets a GameObject with a CameraComponent as the active camera.
	 * @param cameraObject The GameObject containing the CameraComponent.
	 */
	void SetActiveCamera(GameObject* cameraObject);

	/**
	 * @brief Retrieves the CameraComponent of the active camera GameObject.
	 * @return Pointer to the active CameraComponent, or nullptr if none is set.
	 */
	CameraComponent* GetActiveCamera();

	/**
	 * @brief Retrieves the GameObject associated with the active camera.
	 * @return Pointer to the active camera GameObject, or nullptr if none is set.
	 */
	GameObject* GetActiveCameraObject() { return activeCameraObject; }

	/**
	 * @brief Retrieves the combined View-Projection matrix for rendering.
	 * This matrix is calculated using the active camera's component.
	 * @param screenWidth The current width of the viewport.
	 * @param screenHeight The current height of the viewport.
	 * @return The 4x4 View-Projection matrix (mat4).
	 */
	mat4 GetViewProjectMatrix(float screenWidth, float screenHeight);

	/**
	 * @brief Retrieves the position of the active camera in world space.
	 * @return The camera's position as a vec2, or a zero vector if no camera is active.
	 */
	vec2 GetCameraPosition();

	/**
	 * @brief Resets the state of the manager (e.g., clears the active camera object).
	 */
	void ResetState();

	/**
	 * @brief Checks if there is currently an active camera set.
	 * @return True if `activeCameraObject` is not null.
	 */
	bool HasActiveCamera() const;

	/**
	 * @brief Clears the current active camera reference.
	 */
	void ClearActiveCamera();

private:
	/**
	 * @brief Private constructor (for Singleton pattern).
	 */
	CameraManager() : activeCameraObject(nullptr) {};

	/**
	 * @brief Private destructor.
	 */
	~CameraManager() = default;

	// Deleted copy constructor and assignment operator to prevent copying
	CameraManager(const CameraManager&) = delete;
	CameraManager& operator=(const CameraManager&) = delete;

	GameObject* activeCameraObject; ///< Pointer to the GameObject currently designated as the main camera.
};