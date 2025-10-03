/* Start Header ************************************************************************/
/*!
\file       ImGuiSystem.h
\author     Guan Shao Jun [95%]
\par        shaojun.g@digipen.edu
\date       October 02 2025

Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*/
/* End Header **************************************************************************/

#pragma once
#include "System.h"
#include "factory.h"
#include "transform.h"
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <GLFW/glfw3.h>
#include <map>

using SystemTimingsMap = std::map<std::string, double>;

class GameObject;
class GameObjectFactory;
class Serialize_Writer;

/**
 * @class ImGuiSystem
 * @brief Handles ImGui initialization, updates, and rendering for the engine.
 */
class ImGuiSystem : public ISystem {
public:
    /**
     * @brief Construct a new ImGuiSystem with a GLFW window.
     * @param window Pointer to the GLFW window.
     */
    ImGuiSystem(GLFWwindow* window);

    /**
     * @brief Destroy the ImGuiSystem and cleanup ImGui resources.
     */
    ~ImGuiSystem();

    /**
     * @brief Initialize ImGui (create context, setup bindings).
     */
    void Initialize() override;

    /**
     * @brief Update and render ImGui elements each frame.
     * @param dt Delta time since last frame.
     */
    void Update(float) override;

    /**
     * @brief Return the system's name for identification.
     * @return std::string Name of the system.
     */
    std::string GetName() override;

private:
    /**
     * @brief Shutdown and cleanup ImGui resources.
     */
    void Shutdown();

    GLFWwindow* m_window; ///< Pointer to the GLFW window
};
