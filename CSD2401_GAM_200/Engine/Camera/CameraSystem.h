/* Start Header ************************************************************************/
/*!
\file       CameraSystem.h
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
#include "System.h"
#include "CameraComponent.h"

/**
 * @class CameraSystem
 * @brief Manages all active CameraComponents in the game world.
 *
 * @details The CameraSystem's primary responsibility is to iterate through
 * all GameObjects that possess a CameraComponent and call their Update()
 * method. This handles camera movement logic such as following targets,
 * applying smoothing, and checking world bounds every frame.
 */
class CameraSystem : public ISystem {
public:
	/**
	 * @brief Default constructor for the CameraSystem.
	 */
	CameraSystem() = default;

	/**
	 * @brief Virtual destructor with default implementation.
	 */
	virtual ~CameraSystem() = default;

	/**
	 * @brief Initializes the CameraSystem.
	 * Currently does not require any setup, but is provided for the ISystem interface.
	 */
	virtual void Initialize() override {}

	/**
	 * @brief The main update function for the system, called every frame.
	 * Iterates over all GameObjects with a CameraComponent and calls the component's Update logic.
	 * @param dt The time elapsed since the last frame (delta time).
	 */
	virtual void Update(float dt) override;

	/**
	 * @brief Returns the name of the system.
	 * @return A string: "Camera System".
	 */
	virtual std::string GetName() override { return "Camera System"; }
private:
	// No private members are required in the header as the system typically gets
	// access to components via a central component/object manager.
};