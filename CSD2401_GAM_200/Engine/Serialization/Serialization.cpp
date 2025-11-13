/* Start Header ************************************************************************/
/*!
\file       Serialization.cpp
\author     Lee Hwee Min [100% - ? lines]
\par        l.hweemin@digipen.edu
\date       November, 3, 2025
Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*/
/* End Header **************************************************************************/

#include "serialization.h"

#include <fstream>
#include <sstream>
#include <iostream>
#include <filesystem>

namespace fs = std::filesystem;
static const std::string PREFAB_DIR = PrefabFilePath;

std::string Serialization::GetFullPath(const std::string &filename)
{
	return PREFAB_DIR + filename + ".json";
}

/**
 * @brief Returns all prefab JSON filenames found in the prefab directory.
 */
std::vector<std::string> Serialization::GetAllPrefabs()
{
	std::vector<std::string> temp;
	for (auto const &prefab : std::filesystem::directory_iterator(PrefabFilePath))
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
 * @brief Converts a GameObject and its components into a JSON object.
 *
 * Collects GameObject properties such as name and active state,
 * then delegates the serialization of all attached components to
 * ComponentSerialization.
 *
 * @param go Pointer to the GameObject to serialize.
 * @return JSON object representing the serialized GameObject.
 */
json Serialization::SerializeGameObject(GameObject *go)
{
	json j;
	if (!go)
		return j;

	j["name"] = go->GetName();
	j["active"] = go->IsActive();

	ComponentSerialization::SerializeAllComponents(go, j);
	return j;
}

/**
 * @brief Creates a GameObject from JSON data and registers it with the factory.
 *
 * Reads GameObject attributes such as name and active state, and reconstructs
 * all serialized components through ComponentSerialization.
 *
 * @param j JSON object describing the GameObject.
 */
void Serialization::DeserializeGameObject(const json &j)
{
	GameObjectFactory &factory = GameObjectFactory::Instance();
	std::string name = j.value("name", "Unnamed");

	std::cout << "[Deserialize] Creating: " << name << std::endl;
	GameObject *go = factory.CreateGameObject(name);
	std::cout << "[Deserialize] Created GO at " << go << " (ID " << go->GetID() << ")\n";

	go->SetActive(j.value("active", true));

	ComponentSerialization::DeserializeAllComponents(go, j);
}

/**
 * @brief Saves a single GameObject as a prefab JSON file.
 * @param go Reference to the GameObject to save.
 */
void Serialization::SavePrefab(GameObject &go)
{
	// Make sure the prefab directory exists
	if (!fs::exists(PrefabFilePath))
		fs::create_directories(PrefabFilePath);

	// Construct full prefab path
	std::string path = PrefabFilePath + go.GetName() + ".json";

	// JSON object
	json j;
	j["GameObjects"] = json::array();

	// Serialize only this single GameObject
	j["GameObjects"].push_back(Serialization::SerializeGameObject(&go));

	// Write to disk
	std::ofstream out(path, std::ios::trunc);
	if (out.is_open())
	{
		out << j.dump(4);
		out.close();
		std::cout << "[SavePrefab] Prefab saved successfully: " << path << "\n";
	}
	else
	{
		std::cerr << "[SavePrefab] Failed to write prefab: " << path << "\n";
	}
}

/**
 * @brief Loads and instantiates a prefab from disk.
 * @param prefabName Name of the prefab file (without extension).
 */
void Serialization::LoadPrefab(const std::string &prefabName)
{
	// Construct full prefab file path
	std::string path = PrefabFilePath + prefabName + ".json";
	std::cout << "[LoadPrefab] Loading prefab from: " << path << "\n";

	// Verify the prefab exists
	if (!fs::exists(path))
	{
		std::cerr << "[LoadPrefab] Prefab not found: " << path << "\n";
		return;
	}

	// Read file
	std::ifstream in(path);
	if (!in.is_open())
	{
		std::cerr << "[LoadPrefab] Cannot open prefab: " << path << "\n";
		return;
	}

	json j;
	try
	{
		in >> j;
	}
	catch (const std::exception &e)
	{
		std::cerr << "[LoadPrefab] JSON Parse error: " << e.what() << "\n";
		return;
	}

	if (j.contains("GameObjects"))
	{
		for (const auto &obj : j["GameObjects"])
			Serialization::DeserializeGameObject(obj);

		std::cout << "[LoadPrefab] Prefab instantiated successfully.\n";
	}
}

/**
 * @brief Saves all active GameObjects in the scene to a JSON file.
 * @param scenePath Name of the scene file (without extension).
 */
void Serialization::SaveScene(const std::string &scenePath)
{
	std::string path = SceneFilePath + scenePath + ".json";
	json j;
	j["GameObjects"] = json::array();

	// Get all GameObjects currently managed by factory
	const auto &objects = GameObjectFactory::Instance().GetAllGameObjects();

	for (const auto &obj : objects)
	{
		if (obj && obj->IsActive())
		{
			// Serialize and add to JSON array
			j["GameObjects"].push_back(Serialization::SerializeGameObject(obj.get()));
		}
	}

	// Write scene JSON to disk
	std::ofstream out(path, std::ios::trunc);
	if (out.is_open())
	{
		out << j.dump(4);
		out.close();
		std::cout << "[SaveScene] Scene saved successfully :" << path << "\n";
	}

	else
	{
		std::cerr << "[SaveScene] Failed to open file for writing (saving): " << path << "\n";
	}
}

/**
 * @brief Loads a full scene from a JSON file, clearing current objects and audio.
 * @param scenePath Name of the scene file (without extension).
 */
void Serialization::LoadScene(const std::string& scenePath)
{
	std::string path = SceneFilePath + scenePath + ".json";
	if (!fs::exists(path))
	{
		std::cerr << "[LoadScene] Scene file not found: " << path << "\n";
		return;
	}

	std::ifstream in(path);
	if (!in.is_open())
	{
		std::cerr << "[LoadScene] Cannot open scene file: " << path << "\n";
		return;
	}

	json j;
	try
	{
		in >> j;
	}
	catch (const std::exception& e)
	{
		std::cerr << "[LoadScene] Parse error: " << e.what() << "\n";
		return;
	}

	// Clear all existing GameObjects before loading new scene
	std::cout << "[LoadScene] Clearing previous scene GameObjects...\n";
	GameObjectFactory::Instance().Clear();

	// Stop all currently playing audio and unload audio resources
	std::cout << "[LoadScene] Stopping all audio and clearing audio resources...\n";
	AudioEngine* audioEngine = ResourceManager::Instance().GetAudioEngine();
	if (audioEngine)
	{
		audioEngine->stopAllSound();
		audioEngine->unloadAllSound();
	}

	// Clear all audio resources from ResourceManager
	ResourceManager::Instance().UnloadAllAudio();

	if (j.contains("GameObjects"))
	{
		for (const auto& obj : j["GameObjects"])
			Serialization::DeserializeGameObject(obj);

		std::cout << "[LoadScene] Scene loaded successfully: " << path << "\n";
	}
}

/**
 * @brief Deletes a prefab file by name.
 * @param prefabName Name of the prefab file (without extension).
 */
void Serialization::RemovePrefab(std::string const &prefabName)
{
	for (auto const &prefab : std::filesystem::directory_iterator(PrefabFilePath))
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

/**
 * @brief Renames an existing prefab file on disk.
 * @param oldName Current prefab file name (without extension).
 * @param newName New prefab file name (without extension).
 */
void Serialization::RenamePrefab(std::string const &oldName, std::string const &newName)
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

/**
* @brief Use full filepath name! Simply checks if file stream is valid & readable.
*		 Nvm just found out that ifstream automatically closes fstreams but if u ever want to use this its here i guess
*/
const bool Serialization::IsStreamGood(std::string const& fullFilePath)
{
	// literally just checks if file is readable and closes filestreams properly.
	std::ifstream ifs(fullFilePath);
	if (ifs.good())
	{
		std::cout << "filepath: " << fullFilePath << " is good.\n";
		ifs.close();
		return true;
	}
	else
	{
		ifs.close();
		if (std::filesystem::exists(fullFilePath))
		{
			std::cout << "WARNING - unable to open: " << fullFilePath << " . Check file type.\n";
		}
		else
		{
			std::cout << "WARNING - filepath: " << fullFilePath << " DOES NOT EXIST! Check filepath name.\n";
		}
		return false;
	}
}
