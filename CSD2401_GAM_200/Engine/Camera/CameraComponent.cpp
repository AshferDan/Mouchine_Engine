/* Start Header ************************************************************************/
/*!
\file       CameraComponent.cpp
\author     Guan Shao Jun [100%]
\par        shaojun.g@digipen.edu
\date       November 07 2025

Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*/
/* End Header **************************************************************************/

#include "CameraComponent.h"
#include "transform.h" // Assumed to contain TransformComponent definitions
#include <algorithm>   // For std::clamp
#include <cmath>       // For pow() function

/**
 * @brief Constructor for CameraComponent.
 * Initializes the camera with default values: position at origin, zoom 1.0,
 * fixed mode, no smoothing, and large default bounds.
 * @param owner The GameObject this component is attached to.
 */
CameraComponent::CameraComponent(GameObject* owner)
	: IComponent(owner),
	position(0.f, 0.f),
	zoom(1.f),
	mode(CameraMode::FIXED),
	followTarget(nullptr),
	followOffset(0.f, 0.f),
	enableSmoothing(false),
	smoothSpeed(5.f),
	useBounds(false),
	boundsMin(-10000.f, -10000.f),
	boundsMax(10000.f, 10000.f) {
}

/**
 * @brief Main update loop for the camera component.
 * Handles following logic and boundary clamping.
 * @param dt Time elapsed since the last frame.
 */
void CameraComponent::Update(float dt) {
	// Early exit if the component is disabled
	if (!IsEnabled()) {
		return;
	}

	// Follow Target Logic: Execute only if in FOLLOW mode and a valid, active target exists.
	if (mode == CameraMode::FOLLOW_TARGET && followTarget && followTarget->IsActive()) {
		// Attempt to get the target's position component.
		auto* transform = followTarget->GetComponent<TransformComponent>();
		if (transform) {
			float targetX, targetY;
			// Retrieve the world position of the target GameObject
			transform->GetPosition(targetX, targetY);

			// Calculate the final desired position including the offset.
			vec2 targetPos(targetX + followOffset.x, targetY + followOffset.y);

			if (enableSmoothing) {
				// Exponential smoothing (Lerp) for frame rate independent dampening:
				// t = 1 - 0.5^(speed * dt). This ensures smooth, distance-based following.
				float t = 1.f - pow(0.5f, smoothSpeed * dt);

				// Linearly interpolate the position towards the target position
				position.x += (targetPos.x - position.x) * t;
				position.y += (targetPos.y - position.y) * t;
			}
			else {
				// Snap instantly to the target position
				position = targetPos;
			}
		}
	}

	// Apply boundary clamping if enabled, ensuring the camera stays within the world bounds.
	if (useBounds) {
		ApplyBounds();
	}
}

/**
 * @brief Sets the camera's position using separate X and Y coordinates.
 * @param x The new X world coordinate for the camera center.
 * @param y The new Y world coordinate for the camera center.
 */
void CameraComponent::SetPosition(float x, float y) {
	position.x = x;
	position.y = y;

	if (useBounds) {
		ApplyBounds(); // Clamp immediately if bounds are active
	}
}

/**
 * @brief Sets the camera's position using a vec2.
 * @param pos The new world position (vec2) for the camera center.
 */
void CameraComponent::SetPosition(const vec2& pos) {
	position = pos;

	if (useBounds) {
		ApplyBounds(); // Clamp immediately if bounds are active
	}
}

/**
 * @brief Moves the camera by a delta amount relative to its current position.
 * @param dx The change in X position.
 * @param dy The change in Y position.
 */
void CameraComponent::Move(float dx, float dy) {
	position.x += dx;
	position.y += dy;

	if (useBounds) {
		ApplyBounds(); // Clamp immediately if bounds are active
	}
}

/**
 * @brief Sets the zoom level.
 * @param z The zoom factor (clamped to a minimum of 0.1f to prevent inversions or extreme zoom).
 */
void CameraComponent::SetZoom(float z) {
	// Ensure zoom is never too small or negative
	zoom = std::max(0.1f, z);
}

/**
 * @brief Sets the target GameObject for the camera to follow.
 * Note: Does not automatically change the mode to FOLLOW_TARGET.
 * @param target Pointer to the GameObject to track.
 */
void CameraComponent::SetFollowTarget(GameObject* target) {
	followTarget = target;
}

/**
 * @brief Sets the offset from the target's position when in FOLLOW_TARGET mode.
 * @param offset The world space offset from the target.
 */
void CameraComponent::SetFollowOffset(const vec2& offset) {
	followOffset = offset;
}

/**
 * @brief Sets the operational mode of the camera.
 * @param m The new CameraMode (e.g., FIXED, FOLLOW_TARGET, FREE_MOVE).
 */
void CameraComponent::SetMode(CameraMode m) {
	mode = m;
}

/**
 * @brief Enables or disables smoothing and sets the speed factor.
 * @param enable True to enable exponential smoothing.
 * @param speed The smoothing speed factor (higher is faster, less smooth).
 */
void CameraComponent::SetSmoothing(bool enable, float speed) {
	enableSmoothing = enable;
	smoothSpeed = speed;
}

/**
 * @brief Sets the min and max bounds for camera position clamping.
 * @param minX Minimum allowed X position.
 * @param minY Minimum allowed Y position.
 * @param maxX Maximum allowed X position.
 * @param maxY Maximum allowed Y position.
 */
void CameraComponent::SetBounds(float minX, float minY, float maxX, float maxY) {
	boundsMin.x = minX;
	boundsMin.y = minY;
	boundsMax.x = maxX;
	boundsMax.y = maxY;
}

/**
 * @brief Enables or disables the use of positional boundary clamping.
 * @param enable True to activate bounds checking.
 */
void CameraComponent::EnableBounds(bool enable) {
	useBounds = enable;
}

/**
 * @brief Clamps the camera position to the defined bounds (boundsMin and boundsMax).
 */
void CameraComponent::ApplyBounds() {
	// Clamp position.x between boundsMin.x and boundsMax.x
	position.x = std::clamp(position.x, boundsMin.x, boundsMax.x);
	// Clamp position.y between boundsMin.y and boundsMax.y
	position.y = std::clamp(position.y, boundsMin.y, boundsMax.y);
}

/**
 * @brief Calculates the OpenGL View Matrix (World-to-View transformation).
 * For 2D, this translates the world opposite to the camera's position.
 * @return The 4x4 view matrix (mat4).
 */
mat4 CameraComponent::GetViewMatrix() const {
	// Translate the world opposite to the camera's position
	return mat4::translate(vec3(-position.x, -position.y, 0.f));
}

/**
 * @brief Calculates the OpenGL Orthographic Projection Matrix (View-to-Clip transformation).
 * Defines the visible world area based on screen dimensions and zoom level.
 * @param screenWidth The width of the viewport/window.
 * @param screenHeight The height of the viewport/window.
 * @return The 4x4 orthographic projection matrix (mat4).
 */
mat4 CameraComponent::GetProjectionMatrix(float screenWidth, float screenHeight) const {
	// Calculate the half width and height of the visible world space, scaled by zoom
	float halfWidth = (screenWidth / 2.0f) / zoom;
	float halfHeight = (screenHeight / 2.0f) / zoom;

	// Create the orthographic projection matrix: Left, Right, Bottom, Top, Near, Far
	// The near/far planes are typically -1.f and 1.f for a standard 2D setup.
	return mat4::ortho(-halfWidth, halfWidth, -halfHeight, halfHeight, -1.f, 1.f);
}

/**
 * @brief Calculates the combined View-Projection Matrix (Projection * View).
 * This is the final matrix passed to the shader to transform world coordinates to screen coordinates.
 * @param screenWidth The width of the viewport/window.
 * @param screenHeight The height of the viewport/window.
 * @return The 4x4 combined View-Projection matrix (mat4).
 */
mat4 CameraComponent::GetViewProjectionMatrix(float screenWidth, float screenHeight) const {
	mat4 projection = GetProjectionMatrix(screenWidth, screenHeight);
	mat4 view = GetViewMatrix();
	return projection * view;
}

/**
 * @brief Creates a deep copy of the component for a new GameObject.
 * All state variables, including position, zoom, and follow settings, are cloned.
 * @param newOwner The GameObject that will own the cloned component.
 * @return A unique pointer to the newly cloned CameraComponent.
 */
std::unique_ptr<IComponent> CameraComponent::Clone(GameObject* newOwner) const {
	auto cloned = std::make_unique<CameraComponent>(newOwner);

	// Core camera state
	cloned->position = this->position;
	cloned->zoom = this->zoom;
	cloned->mode = this->mode;

	// Follow target settings
	// WARNING: Clones the raw pointer. If the target is destroyed, this pointer becomes invalid.
	cloned->followTarget = this->followTarget;
	cloned->followOffset = this->followOffset;

	// Smoothing settings
	cloned->enableSmoothing = this->enableSmoothing;
	cloned->smoothSpeed = this->smoothSpeed;

	// Bounds settings
	cloned->useBounds = this->useBounds;
	cloned->boundsMin = this->boundsMin;
	cloned->boundsMax = this->boundsMax; // Ensure boundsMax is also cloned

	// Component state
	cloned->SetEnabled(this->IsEnabled());

	return cloned;
}