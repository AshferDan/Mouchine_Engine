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
#include "RenderComponent.h"

class GameObject;
using json = nlohmann::json;

// --- Object Serialization ---
json SerializeGameObject(GameObject* GO);
void DeserializeGameObject(const json& j);

// --- Game Object Creation ---
GameObject* CreateObj(
    const std::string& name,
    RenderComponent::ShapeType shapeType,
    const vec2& pos,
    const vec2& scale,
    const vec4& color,
    float rotation = 0.f);

/**
 * @brief Save all active GameObjects in the scene into a single JSON file.
 */
void SaveScene(const std::string& filePath = "../Logs/Scene.json");

/**
 * @brief Load all GameObjects from a JSON file into the current game scene.
 */
void LoadScene(const std::string& filePath = "../Logs/Scene.json");

/**
 * @brief Save one GameObject as a prefab JSON file.
 */
void SavePrefab(GameObject& GO, const std::string& existingFile = "");

/**
 * @brief Load one prefab JSON file into the current scene.
 */
void LoadPrefab(const std::string& filePath);

std::vector<std::string> GetAllPrefabJSONFiles();
