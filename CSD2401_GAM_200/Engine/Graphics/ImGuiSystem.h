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
#include "InputManager.h"
#include <filesystem>

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

void ReloadPrefabsList();

class DirectoryNavigator
{
public:
    DirectoryNavigator();
    void RenderImGui(); 
private:
    std::filesystem::path currentDir;
};

void Profiler_Render();

void GOCreationMenu_Render();

void GOCreationMenu_Spawn();

void PrefabMenu_Render();

void PrefabMenu_Spawn();

/**
  * @brief 1/2 functions for drag and drop operations within asset browser. This one is for sending. Sends a payload with data + leaves a trailing text for users to see what file is being moved.
  * @param PayloadID        ID that the Receiving end will check against, can be any string!
  * @param filePath         string containing filepath/ data to send over
  * @param displayText      string that shows near cursor while holding left click + dragging.
  */
template <typename DataType>
void SendDragAndDrop(std::string const& PayloadID, DataType const& dataToSend, std::string const& displayText);
void SendDragAndDrop(std::string const& PayloadID, std::string const& dataToSend, std::string const& displayText);

void ReceiveDragAndDrop(std::string const& PayloadID, std::string& dataToReceive);

void CompletedDragAndDrop();

void TopMenuBar_Render();
