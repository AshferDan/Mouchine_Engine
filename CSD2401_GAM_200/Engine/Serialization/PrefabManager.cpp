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
static const std::string PREFAB_DIR = "Assets/Prefabs/";

/**
 * @brief Returns full path for a prefab filename.
 */
std::string PrefabManager::GetFullPath(const std::string& filename)
{
    return PREFAB_DIR + filename;
}

/**
 * @brief Returns all prefab JSON filenames found in the prefab directory.
 */
std::vector<std::string> PrefabManager::GetAllPrefabs()
{
    std::vector<std::string> files;
    if (!fs::exists(PREFAB_DIR)) return files;

    for (const auto& entry : fs::directory_iterator(PREFAB_DIR))
    {
        if (entry.is_regular_file())
        {
            std::string name = entry.path().filename().string();
            if (name.find("Prefab_") != std::string::npos && name.ends_with(".json"))
                files.push_back(name);
        }
    }
    return files;
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
    physics->gravityX = 0.0f;
    physics->gravityY = -9.8f;

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
void PrefabManager::SavePrefab(GameObject* go, const std::string& existingPath, bool append)
{
    // --- Resolve target path ---
    std::string path;
    if (!existingPath.empty())
        path = existingPath;
    else if (go)
        path = GetFullPath("Prefab_" + go->GetName() + ".json");
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

        Serialization::SavePrefab(*go, path, true);
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

        // Even though we pass one GameObject, Serialization::SavePrefab(false)
        // automatically iterates all active objects to rewrite the scene.
        Serialization::SavePrefab(*objs.front().get(), path, false);

        std::cout << "[PrefabManager] Overwrote prefab with full scene: " << path << "\n";
    }
}

/**
 * @brief Loads prefab file and recreates GameObjects.
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
 * @brief Renames an existing prefab JSON file.
 */

bool PrefabManager::RenamePrefab(const std::string& oldName, const std::string& newName)
{
    std::string oldPath = GetFullPath(oldName);
    std::string newPath = GetFullPath(newName);

    if (!fs::exists(oldPath))
    {
        std::cerr << "[PrefabManager] Cannot rename, file not found: " << oldPath << "\n";
        return false;
    }

    try
    {
        fs::rename(oldPath, newPath);
        std::cout << "[PrefabManager] Renamed " << oldName << " -> " << newName << "\n";
        return true;
    }
    catch (const std::exception& e)
    {
        std::cerr << "[PrefabManager] Rename failed: " << e.what() << "\n";
        return false;
    }
}

/**
 * @brief Deletes a prefab JSON file from disk.
 */
bool PrefabManager::DeletePrefab(const std::string& filename)
{
    std::string path = GetFullPath(filename);
    if (!fs::exists(path))
        return false;

    try
    {
        fs::remove(path);
        std::cout << "[PrefabManager] Deleted prefab: " << filename << "\n";
        return true;
    }
    catch (const std::exception& e)
    {
        std::cerr << "[PrefabManager] Delete failed: " << e.what() << "\n";
        return false;
    }
}
