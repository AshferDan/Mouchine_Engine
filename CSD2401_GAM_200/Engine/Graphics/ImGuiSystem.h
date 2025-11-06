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
#include "GraphicsSystem.h"
#include <filesystem>
#include "Serialization.h"

// FILE SYSTEM
namespace fs = std::filesystem;

const std::string AssetPayloadID{ "Asset_Filepath" };
const std::string PrefabPayloadID{ "Prefab_Filepath" };
const std::string HierarchyPayloadID{ "GameObject_Name" };
const std::string ScenePayloadID{ "Scene_Name" };

// ASSET MANAGER/ DIRECTORY NAVIGATOR VARIABLES
// Vars for ImGui::ImageButton
inline const char* icon_folder = "../Assets/Editor_Icons/Folder.png";
inline const char* icon_file = "../Assets/Editor_Icons/File_Generic.png";
inline const char* icon_plus = "../Assets/Editor_Icons/Plus.png";
inline const char* icon_prefab = "../Assets/Editor_Icons/Cube.png";
extern GLuint icon_folder_ui;
extern GLuint icon_file_ui;
extern GLuint icon_plus_ui;
extern GLuint icon_prefab_ui;


// CONST VARIABLES
const ImVec2 icon_size(64, 64); // AssetManager icon sizes
const ImVec2 icon_size_small(20, 20); // AssetManager icon sizes
// Payload IDs (for different types of drag and drops)
static const fs::path assetPath = "../Assets";

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

    void Hierarchy_Render();

    void Hierarchy_GameObjectTree_Renderer(GameObject* obj, int depth, EntityID& selectedGameObjectID, GameObjectFactory& factory);

    ///**
    // * @brief Add a new game object through the editor
    // * @param data GameObject data to instantiate
    // * @return Pointer to created GameObject, or nullptr if failed
    // */
    //GameObject* AddGameObject(const GameObjectData& data);

    /**
     * @brief Remove a game object from the level
     * @param id EntityID of the game object to remove
     */
    void RemoveGameObject(EntityID id);

    ///**
    // * @brief Get the currently selected game object
    // * @return Pointer to selected GameObject, or nullptr if none selected
    // */
    //GameObject* GetSelectedGameObject() const;

    /**
     * @brief Set the currently selected game object
     * @param id EntityID of the game object to select
     */
    void SetSelectedGameObject(EntityID id);



private:
    /**
     * @brief Shutdown and cleanup ImGui resources.
     */
    void Shutdown();
    GameObjectFactory& factory;
    EntityID selectedGameObjectID;        // currently selected game object

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

void EditorLayout_SaveScene(std::string const& fileName);

void EditorLayout_LoadScene(std::string const& fileName);

void EditorLayout_Refresh();

void EditorLayout_SaveNamePopUp();

void EditorLayout_LoadPopUp();


void CreateHintText(std::string const& displayText);
