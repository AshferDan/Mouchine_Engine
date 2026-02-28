/* Start Header ************************************************************************/
/*!
\file       CameraComponent.h
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
#include "Component.h"
#include "MathLib.h"
#include "gameObject.h"

/**
 * @class CameraComponent
 * @brief Defines the logic and parameters for the 2D game camera, handling
 * position, zoom, following targets, bounds, and view matrix calculations.
 *
 * It is designed to be attached to a GameObject, often a dedicated camera object.
 */
class CameraComponent : public IComponent {
public:
	/**
	 * @brief Defines the operational mode of the camera.
	 */
	enum class CameraMode {
		FOLLOW_TARGET, ///< The camera tracks a specified target GameObject.
		FREE_MOVE,     ///< The camera is free to be moved manually by systems (e.g., editor or player controls).
		FIXED          ///< The camera remains static at its set position.
	};

	/**
	 * @brief Constructor for the CameraComponent.
	 * @param owner The GameObject this component is attached to.
	 */
	CameraComponent(GameObject* owner);

	/**
	 * @brief Virtual destructor with default implementation.
	 */
	virtual ~CameraComponent() = default;

	/**
	 * @brief Updates the camera's position based on its current mode (e.g., following target).
	 * @param deltaTime Time elapsed since the last frame.
	 */
	virtual void Update(float deltaTime) override;

	/**
	 * @brief Returns the name of the component type.
	 * @return A string: "Camera Component".
	 */
	virtual std::string GetName() const override { return "Camera Component"; }

	/**
	 * @brief Creates a deep copy of the component for a new GameObject.
	 * @param newOwner The GameObject that will own the cloned component.
	 * @return A unique pointer to the cloned component.
	 */
	virtual std::unique_ptr<IComponent> Clone(GameObject* newOwner) const override;

	/**
	 * @brief Sets the camera's center position using separate X and Y coordinates.
	 * @param x The new X world coordinate.
	 * @param y The new Y world coordinate.
	 */
	void SetPosition(float x, float y);

	/**
	 * @brief Sets the camera's center position using a vec2.
	 * @param pos The new world position vector.
	 */
	void SetPosition(const vec2& pos);

	/**
	 * @brief Retrieves the current camera center position.
	 * @return The current position as a vec2.
	 */
	vec2 GetPosition() const { return position; }

	/**
	 * @brief Moves the camera relative to its current position.
	 * @param dx The change in X position.
	 * @param dy The change in Y position.
	 */
	void Move(float dx, float dy);

	/**
	 * @brief Sets the camera zoom level.
	 * @param z The zoom factor (1.0f is normal; > 1.0f is zoomed in).
	 */
	void SetZoom(float z);

	/**
	 * @brief Retrieves the current zoom level.
	 * @return The zoom factor.
	 */
	float getZoom() const { return zoom; }

	/**
	 * @brief Sets the target GameObject for the camera to follow (activates FOLLOW_TARGET mode).
	 * @param target Pointer to the GameObject to track.
	 */
	void SetFollowTarget(GameObject* target);

	/**
	 * @brief Sets the offset from the target's position.
	 * @param offset The world space offset from the target.
	 */
	void SetFollowOffset(const vec2& offset);

	/**
	 * @brief Retrieves the current follow offset.
	 * @return The follow offset vector.
	 */
	vec2 GetFollowOffset() { return followOffset; }

	/**
	 * @brief Retrieves the current follow target.
	 * @return Pointer to the target GameObject.
	 */
	GameObject* GetFollowTarget() const { return followTarget; }

	/**
	 * @brief Retrieves the name of the current follow target, or an empty string if none.
	 * @return The name of the follow target.
	 */
	std::string GetFollowTargetName() const { return followTarget ? followTarget->GetName() : ""; }

	/**
	 * @brief Stores the desired follow target name for deferred resolution after full scene load.
	 */
	void SetPendingFollowTargetName(const std::string& name) { pendingFollowTargetName = name; }
	const std::string& GetPendingFollowTargetName() const { return pendingFollowTargetName; }

	/**
	 * @brief Sets the operational mode of the camera.
	 * @param m The new CameraMode.
	 */
	void SetMode(CameraMode m);

	/**
	 * @brief Retrieves the current operational mode of the camera.
	 * @return The current CameraMode.
	 */
	CameraMode GetMode() const { return mode; }

	/**
	 * @brief Enables/disables smoothing (linear interpolation) when following a target.
	 * @param enable True to enable smoothing.
	 * @param speed The speed factor for smoothing (e.g., 5.0f).
	 */
	void SetSmoothing(bool enable, float speed);

	/**
	 * @brief Retrieves the current smoothing speed.
	 * @return The smoothing speed factor.
	 */
	float GetSmoothingSpeed() { return smoothSpeed; }

	/**
	 * @brief Checks if smoothing is currently active.
	 * @return True if smoothing is enabled.
	 */
	bool IsSmoothingEnabled() const { return enableSmoothing; }

	/**
	 * @brief Sets the rectangular boundaries the camera position will be clamped to.
	 * @param minX Minimum allowed X position.
	 * @param minY Minimum allowed Y position.
	 * @param maxX Maximum allowed X position.
	 * @param maxY Maximum allowed Y position.
	 */
	void SetBounds(float minX, float minY, float maxX, float maxY);

	/**
	 * @brief Enables or disables the use of positional boundary clamping.
	 * @param enable True to activate bounds checking.
	 */
	void EnableBounds(bool enable);

	/**
	 * @brief Checks if boundary clamping is enabled.
	 * @return True if bounds are active.
	 */
	bool AreBoundsEnabled() const { return useBounds; }

	/**
	 * @brief Calculates the OpenGL View Matrix (transforms world to camera space).
	 * @return The 4x4 view matrix.
	 */
	mat4 GetViewMatrix() const;

	/**
	 * @brief Calculates the OpenGL Orthographic Projection Matrix (transforms camera to clip space).
	 * @param screenWidth The width of the viewport/window.
	 * @param screenHeight The height of the viewport/window.
	 * @return The 4x4 projection matrix.
	 */
	mat4 GetProjectionMatrix(float screenWidth, float screenHeight) const;

	/**
	 * @brief Calculates the combined View-Projection Matrix (Projection * View).
	 * @param screenWidth The width of the viewport/window.
	 * @param screenHeight The height of the viewport/window.
	 * @return The 4x4 view-projection matrix.
	 */
	mat4 GetViewProjectionMatrix(float screenWidth, float screenHeight) const;

	/**
	 * @brief Adds trauma to the camera shake system
	 *
	 * Increases the current trauma level by the specified amount.
	 * Trauma values are typically clamped between 0.0 and 1.0.
	 *
	 * @param amount The amount of trauma to add (usually between 0.0 and 1.0)
	 */
	void AddTrauma(float amount);

	/**
	 * @brief Updates the trauma and camera shake over time
	 *
	 * Should be called every frame to update trauma decay and calculate
	 * the current shake offset based on the trauma level.
	 *
	 * @param deltaTime Time elapsed since the last update in seconds
	 */
	void UpdateTrauma(float deltaTime);

	/**
	 * @brief Gets the current camera shake offset
	 *
	 * @return vec2 The current screen shake offset in 2D space
	 */
	vec2 GetTraumaOffset() const { return traumaShakeOffset; };

	/**
	 * @brief Gets the current trauma level
	 *
	 * @return float The current trauma level (typically between 0.0 and 1.0)
	 */
	float GetTrauma() const { return trauma; };

	/**
	 * @brief Sets the rate at which trauma decays over time
	 *
	 * @param rate The decay rate (higher values mean faster decay)
	 */
	void SetTraumaDecayRate(float rate) { traumaDecayRate = rate; };

	/**
	 * @brief Sets the maximum screen shake offset
	 *
	 * @param offset The maximum offset in screen space units
	 */
	void SetMaxShakeOffset(float offset) { maxShakeOffset = offset; };

	/**
	 * @brief Starts a screen flash effect
	 *
	 * @param duration The duration of the flash in seconds
	 * @param color The color of the flash (RGBA)
	 * @param fadeOut If true, the flash will fade out smoothly; if false, it will end abruptly
	 */
	void StartFlash(float duration, const vec4& color, bool fadeOut = true);

	/**
	 * @brief Stops any active flash effect immediately
	 */
	void StopFlash();

	/**
	 * @brief Updates the flash effect over time
	 *
	 * Should be called every frame to update flash animation and fading.
	 *
	 * @param deltaTime Time elapsed since the last update in seconds
	 */
	void UpdateFlash(float deltaTime);

	/**
	 * @brief Checks if a flash effect is currently active
	 *
	 * @return bool True if a flash effect is currently active, false otherwise
	 */
	bool IsFlashing() const { return isFlashing; };

	/**
	 * @brief Gets the current flash color with alpha applied
	 *
	 * The returned color includes the current alpha value based on
	 * the flash progression (fade in/out).
	 *
	 * @return vec4 The current flash color with alpha modulation
	 */
	vec4 GetFlashColor() const;

private:
	vec2 position; 				///< The center position of the camera in world coordinates.
	float zoom;					///< The zoom factor applied to the view (1.0 = no zoom).
	CameraMode mode;			///< The current operational mode (FIXED, FOLLOW, FREE).
	std::string pendingFollowTargetName; ///< Deferred follow target name, resolved after full scene load.

	// Follow target settings
	GameObject* followTarget;	///< Pointer to the GameObject the camera is tracking.
	vec2 followOffset;			///< Static offset applied relative to the target's position.

	// Smoothing parameters
	bool enableSmoothing;		///< Flag to enable linear interpolation (Lerp) when following.
	float smoothSpeed;			///< The interpolation speed factor.

	// Bounds settings
	bool useBounds;				///< Flag to enable clamping the camera position to bounds.
	vec2 boundsMin;				///< Minimum allowed X and Y position.
	vec2 boundsMax;				///< Maximum allowed X and Y position.

	/**
	 * @brief Helper function to clamp the camera position to the defined bounds.
	 * This is called after any operation that changes the camera's position.
	 */
	void ApplyBounds();

	// ========== TRAUMA/SHAKE SYSTEM ==========
	float trauma = 0.f;					// Current trauma level (0=no shake, 1=max shake)
	float traumaDecayRate = 2.0f;		// Trauma reduction rate (units per second)
	float maxShakeOffset = 20.0f;		// Max camera displacement at trauma=1.0 (pixels)
	vec2 traumaShakeOffset = vec2(0.f, 0.f);	// Calculated shake offset for current frame

	// ========== FLASH EFFECT SYSTEM ==========
	bool isFlashing = false;			// Flash effect active flag
	float flashTimer = 0.f;				// Current time progress within flash duration
	float flashDuration = 0.f;			// Total flash effect time (seconds)
	vec4 flashColor = vec4(1.f, 0.f, 0.f, 0.5f);	// RGBA color for flash (r,g,b,a)
	bool flashFadeOut = true;			// Smooth fade-out toggle (true=fade, false=cutoff)

};