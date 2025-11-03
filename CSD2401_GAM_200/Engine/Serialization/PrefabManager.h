/* Start Header ************************************************************************/
/*!
\file       PrefabManager.h
\author     Lee Hwee Min [100% - 72 lines]
\par        l.hweemin@digipen.edu
\date       Novemeber, 3, 2025
Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*/
/* End Header **************************************************************************/

#pragma once
#include <string>
#include <vector>
#include "RenderComponent.h"
#include "GameObject.h"

namespace PrefabManager
{
    /**
     * @brief Retrieves the list of all prefab filenames available.
     *
     * This function scans the prefab directory and returns all JSON
     * filenames found (excluding the directory path).
     *
     * @return Vector of prefab filenames as strings.
     */
    std::vector<std::string> GetAllPrefabs();

    /**
     * @brief Creates a new prefab GameObject with basic rendering properties.
     *
     * This is typically used for generating a new prefab template at runtime
     * or through the ImGui editor. The created prefab is initialized with
     * a transform, shape, and color.
     *
     * @param name       Name of the prefab.
     * @param shapeType  Shape type for its RenderComponent.
     * @param pos        Initial position of the prefab.
     * @param scale      Initial scale of the prefab.
     * @param color      Color applied to the RenderComponent.
     * @param rotation   Initial rotation in degrees or radians.
     *
     * @return Pointer to the newly created GameObject representing the prefab.
     */
    GameObject* CreatePrefabObject(
        const std::string& name,
        RenderComponent::ShapeType shapeType,
        const vec2& pos,
        const vec2& scale,
        const vec4& color,
        float rotation);

    void SaveScene(GameObject* go, const std::string& existingPath = "", bool append = false);
    void LoadScene(const std::string& filename);

    /**
     * @brief Saves or updates a prefab file on disk.
     *
     * Wraps the Serialization::SavePrefab call. Can either overwrite
     * an existing prefab or append data, depending on parameters.
     *
     * @param go            Pointer to the GameObject to save as prefab.
     * @param existingPath  Optional path to an existing prefab file to overwrite.
     * @param append        Whether to append to the existing prefab file.
     */
    void SavePrefab(GameObject& go);

    /**
     * @brief Loads and instantiates a prefab from disk.
     *
     * Wraps the Serialization::LoadPrefab function to read the prefab JSON file
     * and reconstruct its corresponding GameObject(s) in the scene.
     *
     * @param filename Filename (with or without extension) of the prefab to load.
     */
    void LoadPrefab(const std::string& filename);

    /**
     * @brief Renames an existing prefab JSON file.
     *
     * Useful for updating prefab identifiers within the editor. The
     * function safely renames the file and returns success status.
     *
     * @param oldName Current prefab file name.
     * @param newName New prefab file name.
     * @return True if renamed successfully, false otherwise.
     */
    void RenamePrefab(std::string const& oldName, std::string const& newName);

    /**
     * @brief Deletes a prefab JSON file from disk.
     *
     * Permanently removes the specified prefab file. Use with caution
     * as this action cannot be undone.
     *
     * @param filename Name of the prefab file to delete.
     * @return True if deleted successfully, false otherwise.
     */
    void RemovePrefab(std::string const& prefabName);

    /**
     * @brief Constructs the full filesystem path for a prefab file.
     *
     * Combines the prefab directory path with the given filename,
     * ensuring consistent path resolution across modules.
     *
     * @param filename Prefab filename (with or without extension).
     * @return Full file path as a string.
     */
    std::string GetFullPath(const std::string& filename);
}
