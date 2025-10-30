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

#include "serialization.h"
#include "gameObject.h"
#include "RenderComponent.h"
#include "transform.h"
#include "physics.h"
#include "collidercomponent.h"
#include "collision.h"
#include "factory.h"

class GameObject;
using json = nlohmann::json;

// --- Object Serialization ---
json SerializeGameObject(GameObject *GO);
void DeserializeGameObject(const json &j);

// --- Game Object Creation ---
GameObject *CreateObj(
	const std::string &name,
	RenderComponent::ShapeType shapeType,
	const vec2 &pos,
	const vec2 &scale,
	const vec4 &color,
	float rotation = 0.f);

const std::string PrefabFilePath{"../Assets/Prefabs/"};

/**
 * @brief Save one GameObject as a prefab JSON file.
 */
void SavePrefab(GameObject &GO, const std::string &existingFile = "");

/**
 * @brief Load one prefab JSON file into the current scene.
 */
void LoadPrefab(const std::string &filePath);

std::vector<std::string> GetAllPrefabJSONFiles();
// 	/**
// 	 * @brief The only Ctor we allow. Sets ifs to be either read in txt or binary.
// 	 * @param filePathReference string containing file path to read from
// 	 * @param isBinaryStream	   flag for if file will be read in binary
// 	 */
// 	Serialize_Reader(std::string const &filePathReference, bool isBinaryStream = false) : filePath{filePathReference}
// 	{
// 		if (isBinaryStream)
// 		{
// 			ifs.open(filePath, std::ios::binary);
// 		}
// 		else
// 			ifs.open(filePath);
// 	}

// 	/**
// 	 * @brief Dtor. Clears streams and closes file stream
// 	 */
// 	~Serialize_Reader()
// 	{
// 		ifs.clear();
// 		ifs.close();
// 	}

// 	/**
// 	 * @brief Returns if stream is valid
// 	 */
// 	bool IsStreamGood() const
// 	{
// 		return ifs.good();
// 	}

// 	/**
// 	 * @brief The reader version of WriteSimple Data. Basically just reads the entire textfile, and stores data into a stringstream ref!
// 	 * @param destination Reference to data to read
// 	 * @param size		 Size of data we are reading
// 	 * @param ss			 Reference to stringsStream where data is sent to
// 	 * @return bool! Depending if operation was successful
// 	 */
// 	bool ReadSimpleData(char *destination, size_t size, std::stringstream &ss)
// 	{
// 		if (!IsStreamGood()) // If file bad, return out
// 		{
// 			std::cerr << "File Stream: " << filePath << " is not good!\n";
// 			return false;
// 		}
// 		ifs.clear(); // Else Read the data from file!
// 		ifs.read(destination, size);

// 		while (ifs.peek() != EOF)
// 		{
// 			ss << ifs.rdbuf(); // Stores data into stringstream, to be retrieved outside of this function. Return true, success!
// 		}
// 		return true;
// 	}

// 	/**
// 	 * @brief [WIP] Reads data of entire game objects! As of right now, reads everything, including the formatted string.
// 	 *		 (Definitely requires tweaking for component data! But general GO data works fine!)
// 	 *		 (Ideally, would be able to read data of entire gameobject and do something with the data)
// 	 * @param GO Reference to Game Object to write data of
// 	 * @param size Size of data we are writing
// 	 * @return bool! Depending if operation was successful
// 	 */
// 	bool ReadGameObjectData(GameObject const &GO, size_t size, std::stringstream &ss)
// 	{
// 		if (!IsStreamGood())
// 		{
// 			std::cerr << "File Stream: " << filePath << " is not good!\n";
// 			return false;
// 		}

// 		// As of rn, it just reads data and stores it into stringstream, same as above really... WIP!!
// 		std::string line;
// 		while (std::getline(ifs, line))
// 		{
// 			ss << line << "\n";
// 		}
// 		return true;
// 	}
// 	/**
// 	 * @brief Private members to hold File path and input file stream!
// 	 */
// private:
// 	std::string filePath;
// 	std::ifstream ifs;
// };

// // NON MEMBER FUNCTIONS

// namespace PrefabTools
// {
// 	/**
//  * @brief Takes existing GO, and turns it into a prefab!!! As of right now, we are storing it in Logs! Will move to its own folder soon!
//  * @param GO Reference to Game Object to read data of
//  *			(Will require tweaking to read component data! But as of rn, basic data works!)
//  */
// 	void CreatePrefab(GameObject& GO)
// 	{
// 		std::string filePath{ PrefabFilePath + GO.GetName() + ".txt" }; // Create a textfile in Logs folder with GOs name
// 		std::ofstream ofs(filePath);									 // New ofs

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
// 	}

// 	/**
// 	 * @brief Takes file path to read prefab data! Actual loading of prefab data is somewhere else, will shift into here soon for cleanliness!
// 	 * @param destination c string for destination of where to read prefab data from
// 	 */
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

// 		// this is pretty buggy, WIP!!!!
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
// }

// /**
//  * @brief [WIP] Original serialising class. Might scrap?
//  */
// class Serializable
// {
// public:
// 	virtual ~Serializable() = default;

// 	virtual json Serialize() const = 0;

// 	virtual void Deserialize(json const &j) = 0;

// 	virtual std::string GetTypeName() const = 0;
// };
