/* Start Header ************************************************************************/
/*!
\file       Serialization.h
\author     Muhammad Ashfer [100% - 27 lines]
\par        m.binafzalkhan@digipen.edu
\date       October, 3, 2025
Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*/
/* End Header **************************************************************************/

#pragma once
#include <string>
#include <vector>
#include <nlohmann/json.hpp>
#include "GameObject.h"
#include "RenderComponent.h"
#include "factory.h"
#include "CameraComponent.h"
#include "ComponentSerialization.h"

using json = nlohmann::json;
const std::string PrefabFilePath { "../Assets/Prefabs/" };
const std::string SceneFilePath{ "../Assets/Scenes/" };
const std::string LayoutsFilePath{ "../Logs/EditorLayout/" };

namespace Serialization
{
    std::vector<std::string> GetAllPrefabs();
    std::string GetFullPath(const std::string& filename);

    /**
     * @brief Serializes a GameObject and all its components into JSON format.
     * @param go Pointer to GameObject.
     * @return JSON object representing the serialized GameObject.
     */
    json SerializeGameObject(GameObject* go);

    /**
     * @brief Creates a GameObject from JSON data and adds it to the factory.
     * @param j JSON object describing the GameObject.
     */
    void DeserializeGameObject(const json& j);

    /**
    * @brief Saves a single GameObject as a prefab file.
    */
    void SavePrefab(GameObject& go);

    /**
    * @brief Loads a prefab from file and instantiates it into the scene.
    */
    void LoadPrefab(const std::string& path);

    /** 
    * @brief Saves all GameObjects in the current scene to a JSON file. 
    */
    void SaveScene(const std::string& existingPath);

    /** 
    * @brief Loads a complete scene from a JSON file. 
    */
    void LoadScene(const std::string& path);

    /** 
    * @brief Deletes a prefab file by name.
    */
    void RemovePrefab(std::string const& prefabName);

    /**
    * @brief Renames an existing prefab file.
    */
    void RenamePrefab(std::string const& oldName, std::string const& newName);

    extern bool Prefab_DragAndDropped;
}
