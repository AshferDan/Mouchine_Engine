/* Start Header ************************************************************************/
/*!
\file       Serialization.cpp
\author     Lee Hwee Min [100% - ?? lines]
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

	GameObject *go = factory.CreateGameObject(name);
	go->SetActive(j.value("active", true));

	ComponentSerialization::DeserializeAllComponents(go, j);
}

/**
 * @brief Saves one or more GameObjects to a prefab JSON file.
 *
 * Supports two modes:
 * - **Append Mode:** Adds a single GameObject to an existing JSON prefab.
 * - **Overwrite Mode:** Serializes all active GameObjects and replaces the file.
 *
 * @param go            Reference to the GameObject to save or append.
 * @param existingPath  Optional file path to an existing prefab file.
 * @param append        True to append, false to overwrite the entire prefab.
 *
 * @note When overwrite mode is used, all active GameObjects in the
 *       GameObjectFactory are serialized together.
 */
void Serialization::SavePrefab(GameObject &go, const std::string &existingPath, bool append)
{
	std::string path = existingPath.empty()
						   ? "Assets/Prefabs/Prefab_" + go.GetName() + ".json"
						   : existingPath;

	json root;

	// Append — add new object to existing prefab file
	if (append && fs::exists(path))
	{
		std::ifstream in(path);
		if (in.peek() != std::ifstream::traits_type::eof())
		{
			try
			{
				in >> root;
			}
			catch (...)
			{
				root = json::object();
			}
		}
		in.close();

		if (!root.contains("GameObjects"))
			root["GameObjects"] = json::array();

		root["GameObjects"].push_back(Serialization::SerializeGameObject(&go));
	}

	// Overwrite — reserialize *all* current GameObjects
	else
	{
		root = json::object();
		root["GameObjects"] = json::array();

		const auto &objs = GameObjectFactory::Instance().GetAllGameObjects();
		for (const auto &objPtr : objs)
		{
			if (objPtr && objPtr->IsActive())
			{
				root["GameObjects"].push_back(Serialization::SerializeGameObject(objPtr.get()));
			}
		}
	}

	// Write final JSON to disk
	std::ofstream out(path, std::ios::trunc);
	if (out.is_open())
	{
		out << root.dump(4);
		out.close();
	}

	std::cout << "[SavePrefab] "
			  << (append ? "Appended to" : "Overwrote with updated scene: ")
			  << path << "\n";
}
/**
 * @brief Loads and reconstructs GameObjects from a prefab JSON file.
 *
 * Reads the prefab file from disk, parses its JSON data, and recreates
 * all GameObjects within the scene using the GameObjectFactory.
 *
 * @param path Full file path to the prefab JSON file.
 */
void Serialization::LoadPrefab(const std::string &path)
{
	if (!fs::exists(path))
	{
		std::cerr << "[Serialization] Prefab not found: " << path << "\n";
		return;
	}

	std::ifstream in(path);
	if (!in.is_open())
	{
		std::cerr << "[Serialization] Cannot open prefab: " << path << "\n";
		return;
	}

	json j;
	try
	{
		in >> j;
	}
	catch (const std::exception &e)
	{
		std::cerr << "[Serialization] Parse error: " << e.what() << "\n";
		return;
	}

	GameObjectFactory::Instance().Clear();

	if (j.contains("GameObjects"))
	{
		for (const auto &obj : j["GameObjects"])
			DeserializeGameObject(obj);
	}
	else
	{
		DeserializeGameObject(j);
	}

	std::cout << "[SavePrefab] Saved prefab: " << path << "\n";
}

// void LoadPrefab(const std::string& path)
// {
// 	if (path == currentPrefabPath) {
// 		GameObjectFactory::Instance().Clear();
// 	}
// 	else if (!currentPrefabPath.empty()) {
// 		GameObjectFactory::Instance().Clear();
// 	}
// 	std::ifstream in(path);
// 	if (!in.is_open()) {
// 		std::cerr << "[LoadPrefab] Cannot open " << path << "\n";
// 		return;
// 	}

// 	json j;
// 	try { in >> j; }
// 	catch (const std::exception& e) {
// 		std::cerr << "[LoadPrefab] Parse error: " << e.what() << "\n";
// 		return;
// 	}

// 	if (j.contains("GameObjects")) {
// 		for (const auto& obj : j["GameObjects"])
// 			DeserializeGameObject(obj);
// 	}
// 	else {
// 		DeserializeGameObject(j);
// 	}

// 	previousPrefabPath = currentPrefabPath;
// 	currentPrefabPath = path;
// 	std::cout << "[LoadPrefab] Loaded prefab: " << path << "\n";
// }

// namespace PrefabTools
// {
// 	/**
//    * @brief Takes existing GO, and turns it into a prefab!!! As of right now, we are storing it in Logs! Will move to its own folder soon!
//    * @param GO Reference to Game Object to read data of
//    *      (Will require tweaking to read component data! But as of rn, basic data works!)
//    */
// 	void CreatePrefab(GameObject& GO)
// 	{
// 		std::string filePath{ PrefabFilePath + GO.GetName() + ".txt" }; // Create a textfile in Logs folder with GOs name
// 		std::ofstream ofs(filePath);                   // New ofs

// 		if (!ofs.good()) // If stream bad, stop process immediately!
// 		{
// 			std::cerr << "File Stream: " << filePath << " is not good!\n";
// 			return;
// 		}

// 		auto* transform = GO.GetComponent<TransformComponent>();
// 		auto* render = GO.GetComponent<RenderComponent>();
// 		vec4 color = render->GetColor();

// 		// format: Name, Shape Type, PosX, PosY, ScaleX, ScaleY, Colour RGBA
// 		ofs << GO.GetName() << " " << render->GetShapeTypeName() << " " << transform->posX << " " << transform->posY << " " << transform->scaleX << " " << transform->scaleY << " " << color.x << " " << color.y << " " << color.z << " " << color.w << std::endl;

// 		std::cout << "Prefab created successfully!";
// 		ofs.clear();
// 		ofs.close();
// 	}/**
//    * @brief Takes file path to read prefab data! Actual loading of prefab data is somewhere else, will shift into here soon for cleanliness!
//    * @param destination c string for destination of where to read prefab data from
//    */
// 	std::string ReadPrefabData(const char* destination)
// 	{
// 		std::ifstream ifs(destination); // Create ifstream of destination, check if its valid (existing prefab)
// 		if (!ifs.good())
// 		{
// 			std::cerr << "File Stream: " << destination << " is not good!\n";
// 			ifs.clear();
// 			ifs.close();
// 			return ""; // Return EMPTY. No data has been read (can change to "missing data" or something)
// 		}

// 		std::string data{};
// 		std::stringstream ss;

// 		ss << ifs.rdbuf(); // Read data obtained and store into string
// 		data = ss.str();

// 		ifs.clear();
// 		ifs.close();
// 		return data; // Return data obtained (white space separated)
// 	}

// 	std::vector<std::string> LoadPrefabList()
// 	{
// 		std::vector<std::string> temp;
// 		for (auto const& prefab : std::filesystem::directory_iterator(PrefabFilePath))
// 		{
// 			if (prefab.path().extension() == ".txt")
// 			{
// 				std::string prefabName = prefab.path().stem().string();
// 				temp.push_back(prefabName);
// 			}
// 		}
// 		temp.shrink_to_fit();
// 		return temp;
// 	}

// 	void RemovePrefab(std::string const& prefabName)
// 	{
// 		for (auto const& prefab : std::filesystem::directory_iterator(PrefabFilePath))
// 		{
// 			if (prefab.path().stem().string() == prefabName)
// 			{
// 				std::filesystem::remove(prefab);
// 			}
// 			else
// 			{
// 				std::cout << "Error! No matching prefab found!\n";
// 			}
// 		}
// 	}

// 	void RenamePrefab(std::string const& oldName, std::string const& newName)
// 	{
// 		if (!std::filesystem::exists(PrefabFilePath + oldName + ".txt"))
// 		{
// 			std::cout << "File Path " + PrefabFilePath + oldName + ".txt\" cannot be found!\n";
// 			return;
// 		}

// 		if (!std::filesystem::exists(PrefabFilePath + newName + ".txt"))
// 		{
// 			std::filesystem::rename(PrefabFilePath + oldName + ".txt", PrefabFilePath + newName + ".txt");
// 		}
// 		else
// 		{
// 			std::filesystem::rename(PrefabFilePath + oldName + ".txt", PrefabFilePath + newName + "(1).txt");
// 		}

// 		std::cout << "Prefab: " << oldName << " changed to " << newName << std::endl;
// 	}
//     std::cout << "[Serialization] Loaded prefab: " << path << "\n";
// }