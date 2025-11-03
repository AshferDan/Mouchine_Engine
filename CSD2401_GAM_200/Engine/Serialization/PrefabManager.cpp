/* Start Header ************************************************************************/
/*!
\file       PrefabManager.cpp
\author     Lee Hwee Min [100% - ?? lines]
\par        l.hweemin@digipen.edu
\date       Novemeber, 3, 2025
Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*/
/* End Header **************************************************************************/

#include "PrefabManager.h"
#include "Serialization.h"
#include <filesystem>
#include <iostream>

namespace fs = std::filesystem;
static const std::string PREFAB_DIR = PrefabFilePath;

/**
 * @brief Returns full path for a prefab filename.
 */
std::string PrefabManager::GetFullPath(const std::string& filename)
{
    return PREFAB_DIR + filename + ".json";
}

/**
 * @brief Returns all prefab JSON filenames found in the prefab directory.
 */
std::vector<std::string> PrefabManager::GetAllPrefabs()
{
    std::vector<std::string> temp;
    for (auto const& prefab : std::filesystem::directory_iterator(PrefabFilePath))
    {
        if (prefab.path().extension() == ".json")
        {
            std::string prefabName = prefab.path().stem().string();
            temp.push_back(prefabName);
        }
    }
    temp.shrink_to_fit();
    return temp;
}

/**
 * @brief Creates a new GameObject with basic components (Transform, Render, Spritesheet, Physics, Collider).
 */
GameObject* PrefabManager::CreatePrefabObject(
    const std::string& name,
    RenderComponent::ShapeType shapeType,
    const vec2& pos,
    const vec2& scale,
    const vec4& color,
    float rotation)
{
    auto& factory = GameObjectFactory::Instance();

    // --- Create GameObject ---
    GameObject* go = factory.CreateShape(
        name, shapeType,
        pos.x, pos.y,
        scale.x, scale.y,
        color,
        CollisionLayer::Default
    );

    // --- Transform ---
    auto* t = go->AddComponent<TransformComponent>(0, 0);
    t->SetPosition(pos.x, pos.y);
    t->SetScale(scale.x, scale.y);
    t->SetRotation(rotation);

    // --- Physics ---
    auto* physics = go->AddComponent<PhysicsComponent>();
    physics->SetMass(1.0f);
    physics->SetStatic(false);
    physics->useGravity = true;
    physics->SetGrounded(false);
    physics->isJumping = false;
    physics->wasGrounded = false;
    physics->velocityX = 0.0f;
    physics->velocityY = 0.0f;
   

    // --- Collider ---
    ColliderComponent::Type colliderType =
        (shapeType == RenderComponent::ShapeType::Circle)
        ? ColliderComponent::Type::Circle
        : ColliderComponent::Type::Square;

    float width = scale.x;
    float height = scale.y;

    CollisionLayer layer = CollisionLayer::Player; // you can adjust this depending on context

    go->AddComponent<ColliderComponent>(colliderType, width, height, layer);

    std::cout << "[CreateObj] Created full GameObject '" << name
        << "' with Transform, Render, Physics, and Collider.\n";

    return go;
}

/**
 * @brief Saves a prefab to disk (append or overwrite mode).
 */
void PrefabManager::SaveScene(GameObject* go, const std::string& existingPath, bool append)
{
    // --- Resolve target path ---
    std::string path;
    if (!existingPath.empty())
        path = existingPath;
    else if (go)
        path = GetFullPath(go->GetName() + ".json");
    else
        path = GetFullPath("Prefab_DefaultScene.json");

    // --- Append Mode: add a single GameObject to existing prefab ---
    if (append)
    {
        if (!go)
        {
            std::cerr << "[PrefabManager] Append mode requires a valid GameObject.\n";
            return;
        }

        Serialization::SaveScene(*go, path, true);
        std::cout << "[PrefabManager] Appended '" << go->GetName()
            << "' to prefab file: " << path << "\n";
    }

    // --- Overwrite Mode: reserialize entire scene ---
    else
    {
        const auto& objs = GameObjectFactory::Instance().GetAllGameObjects();
        if (objs.empty())
        {
            std::cerr << "[PrefabManager] No active GameObjects to save.\n";
            return;
        }

        // Even though we pass one GameObject, Serialization::SaveScene(false)
        // automatically iterates all active objects to rewrite the scene.
        Serialization::SaveScene(*objs.front().get(), path, false);

        std::cout << "[PrefabManager] Overwrote prefab with full scene: " << path << "\n";
    }
}

void PrefabManager::SavePrefab(GameObject& go)
{
    // Create prefab path based on GameObject name
    std::string path = "..Assets/Prefabs/" + go.GetName() + ".json";

    // Create a new JSON object
    json root;
    root["GameObjects"] = json::array();

    // Serialize only this one GameObject
    root["GameObjects"].push_back(Serialization::SerializeGameObject(&go));

    // Write the JSON file to disk
    std::ofstream out(path, std::ios::trunc);
    if (out.is_open())
    {
        out << root.dump(4);
        out.close();
        std::cout << "[SaveSinglePrefab] Saved single prefab: " << path << "\n";
    }
    else
    {
        std::cerr << "[SaveSinglePrefab] Failed to open file for writing: " << path << "\n";
    }
}

/**
 * @brief Loads a single prefab GO
 */
void PrefabManager::LoadPrefab(const std::string& filename)
{
    std::string fullPath = GetFullPath(filename);
    if (!fs::exists(fullPath))
    {
        std::cerr << "[PrefabManager] File not found: " << fullPath << "\n";
        return;
    }
    Serialization::LoadPrefab(fullPath);
    std::cout << "[PrefabManager] Loaded prefab: " << fullPath << "\n";
}

/**
 * @brief Loads prefab file and recreates GameObjects.
 */
void PrefabManager::LoadScene(const std::string& filename)
{
    std::string fullPath = GetFullPath(filename);
    if (!fs::exists(fullPath))
    {
        std::cerr << "[PrefabManager] Scene not found: " << fullPath << "\n";
        return;
    }
    Serialization::LoadScene(fullPath);
    std::cout << "[PrefabManager] Loaded scene: " << fullPath << "\n";
}

void PrefabManager::RemovePrefab(std::string const& prefabName)
{
 	for (auto const& prefab : std::filesystem::directory_iterator(PrefabFilePath))
 	{
 		if (prefab.path().stem().string() == prefabName)
 		{
 			std::filesystem::remove(prefab);
 		}
 		else
 		{
 			std::cout << "Error! No matching prefab found!\n";
 		}
 	}
}

void PrefabManager::RenamePrefab(std::string const& oldName, std::string const& newName)
{
 	if (!std::filesystem::exists(PrefabFilePath + oldName + ".json"))
 	{
 		std::cout << "File Path " + PrefabFilePath + oldName + ".json\" cannot be found!\n";
 		return;
 	}

 	if (!std::filesystem::exists(PrefabFilePath + newName + ".json"))
 	{
 		std::filesystem::rename(PrefabFilePath + oldName + ".json", PrefabFilePath + newName + ".json");
 	}
 	else
 	{
 		std::filesystem::rename(PrefabFilePath + oldName + ".json", PrefabFilePath + newName + "(1).json");
 	}

 	std::cout << "Prefab: " << oldName << " changed to " << newName << std::endl;
}
