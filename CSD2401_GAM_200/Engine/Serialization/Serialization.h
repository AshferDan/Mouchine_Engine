/* Start Header ************************************************************************/
/*!
\file       Serialization.h
\author     Muhammad Ashfer [100% - 168 lines]
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
#include "ComponentSerialization.h"

using json = nlohmann::json;

namespace Serialization
{
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
     * @brief Saves (creates or updates) a prefab to disk as a JSON file.
     * @param go GameObject reference to serialize.
     * @param existingPath Optional path to an existing prefab to overwrite.
     */
    void SavePrefab(GameObject& go, const std::string& existingPath, bool append);

    /**
     * @brief Loads a prefab JSON file and recreates its GameObjects.
     * @param path Path to the prefab file to load.
     */
    void LoadPrefab(const std::string& path);
}