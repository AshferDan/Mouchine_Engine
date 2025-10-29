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
#include <nlohmann/json.hpp>

class GameObject;
using json = nlohmann::json;

/**
 * @brief Save all active GameObjects in the scene into a single JSON file.
 */
void SaveSceneJSON(const std::string& filePath = "../Logs/Scene.json");

/**
 * @brief Load all GameObjects from a JSON file into the current game scene.
 */
void LoadSceneJSON(const std::string& filePath = "../Logs/Scene.json");

/**
 * @brief Save one GameObject as a prefab JSON file.
 */
void CreatePrefabJSON(GameObject& GO);

/**
 * @brief Load one prefab JSON file into the current scene.
 */
void LoadPrefabJSON(const std::string& filePath);
