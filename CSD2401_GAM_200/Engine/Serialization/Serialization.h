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
#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <nlohmann/json.hpp>
#include "gameObject.h"
#include "RenderComponent.h"
#include "transform.h"

using json = nlohmann::json;

class RenderComponent;
class TransformComponent;

const std::string PrefabFilePath{ "../Assets/Prefabs/" };

/**
 * @brief Serialising Writer Class!
 */
class Serialize_Writer
{
public:
	// Delete these, we want properly defined filestreams
	Serialize_Writer() = delete;
	Serialize_Writer &operator=(Serialize_Writer const &) = delete;
	Serialize_Writer &operator=(Serialize_Writer &&) = delete;

	/**
	 * @brief The only Ctor we allow. Sets ofs to be either write in txt or binary.
	 * @param filePathReference string containing file path to write to
	 * @param isBinaryStream	   flag for if file will write in binary
	 */
	Serialize_Writer(std::string const &filePathReference, bool isBinaryStream = false) : filePath{filePathReference}
	{
		if (isBinaryStream)
		{
			ofs.open(filePath, std::ios::binary);
		}
		else
			ofs.open(filePath);
	}

	/**
	 * @brief Dtor. Clears streams and closes file stream
	 */
	~Serialize_Writer()
	{
		ofs.clear();
		ofs.close();
	}

	/**
	 * @brief Returns if stream is valid
	 */
	bool IsStreamGood() const
	{
		return ofs.good();
	}

	/**
	* @brief Templated to write data to ofs
				(Might require tweaking!!)
	* @param data Reference to data to write
	* @param size Size of data we are writing
	* @return bool! Depending if operation was successful
	*/
	template <typename T>
	bool WriteSimpleData(T const &data, size_t size)
	{
		if (!IsStreamGood()) // If bad file path, return out
		{
			std::cerr << "File Stream: " << filePath << " is not good!\n";
			return false;
		}
		ofs.write(reinterpret_cast<const char *>(&data), size); // Write data to ofs
		return true;
	}

	/**
	 * @brief Writes data of entire game objects! As of right now, writes in a specified format for readability. Prefabs however, will take in raw data
	 *		 (Definitely requires tweaking for component data! But general GO data works fine!)
	 * @param GO Reference to Game Object to write data of
	 * @param size Size of data we are writing
	 * @return bool! Depending if operation was successful
	 */
	bool WriteGameObjectData(GameObject &GO, size_t size)
	{
		// Create temp components to get GO's data
		auto *transform = GO.GetComponent<TransformComponent>();
		auto *render = GO.GetComponent<RenderComponent>();
		if (!transform || !render)
		{
			std::cerr << "Missing required components on GameObject\n";
			return false;
		}

		vec4 color = render->GetColor();

		// Hard coding output directory for now... (shameful, dishonorable even)
		std::ofstream outFile("../Logs/GameObjectData.txt", std::ios::app);
		if (!outFile.is_open())
		{ // Check if ofs valid
			std::cerr << "Failed to open file! Make sure ./Logs/ directory exists\n";
			return false;
		}

		// Print in specified format
		outFile << "GO Name: " << GO.GetName() << " | Shape: " << render->GetShapeTypeName()
			<< " | Position (x,y): (" << transform->posX << "," << transform->posY << ")"
			<< " | Size (w,h): (" << transform->scaleX << "," << transform->scaleY << ")"
			<< " | Colour: (" << color.x << "," << color.y << "," << color.z << "," << color.w << ")"
			<< std::endl;

		outFile.close(); // Close stream and return true. Operation successful!
		std::cout << "Successfully wrote GameObject data to file!\n";
		return true;
	}
	/**
	 * @brief Private members to hold File path and output stream!
	 */
private:
	std::string filePath;
	std::ofstream ofs;
};

/**
 * @brief Serialising Reader Class!
 */
class Serialize_Reader
{
public:
	// Delete these, we want properly defined filestreams
	Serialize_Reader() = delete;
	Serialize_Reader &operator=(Serialize_Reader const &) = delete;
	Serialize_Reader &operator=(Serialize_Reader &&) = delete;

	/**
	 * @brief The only Ctor we allow. Sets ifs to be either read in txt or binary.
	 * @param filePathReference string containing file path to read from
	 * @param isBinaryStream	   flag for if file will be read in binary
	 */
	Serialize_Reader(std::string const &filePathReference, bool isBinaryStream = false) : filePath{filePathReference}
	{
		if (isBinaryStream)
		{
			ifs.open(filePath, std::ios::binary);
		}
		else
			ifs.open(filePath);
	}

	/**
	 * @brief Dtor. Clears streams and closes file stream
	 */
	~Serialize_Reader()
	{
		ifs.clear();
		ifs.close();
	}

	/**
	 * @brief Returns if stream is valid
	 */
	bool IsStreamGood() const
	{
		return ifs.good();
	}

	/**
	 * @brief The reader version of WriteSimple Data. Basically just reads the entire textfile, and stores data into a stringstream ref!
	 * @param destination Reference to data to read
	 * @param size		 Size of data we are reading
	 * @param ss			 Reference to stringsStream where data is sent to
	 * @return bool! Depending if operation was successful
	 */
	bool ReadSimpleData(char *destination, size_t size, std::stringstream &ss)
	{
		if (!IsStreamGood()) // If file bad, return out
		{
			std::cerr << "File Stream: " << filePath << " is not good!\n";
			return false;
		}
		ifs.clear(); // Else Read the data from file!
		ifs.read(destination, size);

		while (ifs.peek() != EOF)
		{
			ss << ifs.rdbuf(); // Stores data into stringstream, to be retrieved outside of this function. Return true, success!
		}
		return true;
	}

	/**
	 * @brief [WIP] Reads data of entire game objects! As of right now, reads everything, including the formatted string.
	 *		 (Definitely requires tweaking for component data! But general GO data works fine!)
	 *		 (Ideally, would be able to read data of entire gameobject and do something with the data)
	 * @param GO Reference to Game Object to write data of
	 * @param size Size of data we are writing
	 * @return bool! Depending if operation was successful
	 */
	bool ReadGameObjectData(GameObject const &GO, size_t size, std::stringstream &ss)
	{
		if (!IsStreamGood())
		{
			std::cerr << "File Stream: " << filePath << " is not good!\n";
			return false;
		}

		// As of rn, it just reads data and stores it into stringstream, same as above really... WIP!!
		std::string line;
		while (std::getline(ifs, line))
		{
			ss << line << "\n";
		}
		return true;
	}
	/**
	 * @brief Private members to hold File path and input file stream!
	 */
private:
	std::string filePath;
	std::ifstream ifs;
};

// NON MEMBER FUNCTIONS

namespace PrefabTools
{
	/**
 * @brief Takes existing GO, and turns it into a prefab!!! As of right now, we are storing it in Logs! Will move to its own folder soon!
 * @param GO Reference to Game Object to read data of
 *			(Will require tweaking to read component data! But as of rn, basic data works!)
 */
	void CreatePrefab(GameObject& GO)
	{
		std::string filePath{ PrefabFilePath + GO.GetName() + ".txt" }; // Create a textfile in Logs folder with GOs name
		std::ofstream ofs(filePath);									 // New ofs

		if (!ofs.good()) // If stream bad, stop process immediately!
		{
			std::cerr << "File Stream: " << filePath << " is not good!\n";
			return;
		}

		auto* transform = GO.GetComponent<TransformComponent>();
		auto* render = GO.GetComponent<RenderComponent>();
		vec4 color = render->GetColor();

		// format: Name, Shape Type, PosX, PosY, ScaleX, ScaleY, Colour RGBA
		ofs << GO.GetName() << " " << render->GetShapeTypeName() << " " << transform->posX << " " << transform->posY << " " << transform->scaleX << " " << transform->scaleY << " " << color.x << " " << color.y << " " << color.z << " " << color.w << std::endl;

		std::cout << "Prefab created successfully!";
		ofs.clear();
		ofs.close();
	}

	/**
	 * @brief Takes file path to read prefab data! Actual loading of prefab data is somewhere else, will shift into here soon for cleanliness!
	 * @param destination c string for destination of where to read prefab data from
	 */
	std::string ReadPrefabData(const char* destination)
	{
		std::ifstream ifs(destination); // Create ifstream of destination, check if its valid (existing prefab)
		if (!ifs.good())
		{
			std::cerr << "File Stream: " << destination << " is not good!\n";
			ifs.clear();
			ifs.close();
			return ""; // Return EMPTY. No data has been read (can change to "missing data" or something)
		}

		std::string data{};
		std::stringstream ss;

		ss << ifs.rdbuf(); // Read data obtained and store into string
		data = ss.str();

		ifs.clear();
		ifs.close();
		return data; // Return data obtained (white space separated)
	}

	std::vector<std::string> LoadPrefabList()
	{
		std::vector<std::string> temp;
		for (auto const& prefab : std::filesystem::directory_iterator(PrefabFilePath))
		{
			if (prefab.path().extension() == ".txt")
			{
				std::string prefabName = prefab.path().stem().string();
				temp.push_back(prefabName);
			}
		}
		temp.shrink_to_fit();
		return temp;
	}

	void RemovePrefab(std::string const& prefabName)
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

	void RenamePrefab(std::string const& oldName, std::string const& newName)
	{
		if (!std::filesystem::exists(PrefabFilePath + oldName + ".txt"))
		{
			std::cout << "File Path " + PrefabFilePath + oldName + ".txt\" cannot be found!\n";
			return;
		}

		// this is pretty buggy, WIP!!!!
		if (!std::filesystem::exists(PrefabFilePath + newName + ".txt"))
		{
			std::filesystem::rename(PrefabFilePath + oldName + ".txt", PrefabFilePath + newName + ".txt");
		}
		else
		{
			std::filesystem::rename(PrefabFilePath + oldName + ".txt", PrefabFilePath + newName + "(1).txt");
		}
		
		std::cout << "Prefab: " << oldName << " changed to " << newName << std::endl;
	}
}

/**
 * @brief [WIP] Original serialising class. Might scrap?
 */
class Serializable
{
public:
	virtual ~Serializable() = default;

	virtual json Serialize() const = 0;

	virtual void Deserialize(json const &j) = 0;

	virtual std::string GetTypeName() const = 0;
};
