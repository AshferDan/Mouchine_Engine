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

// Writer
class Serialize_Writer
{
public:
	// Delete these, we want properly defined filestreams
	Serialize_Writer() = delete;
	Serialize_Writer& operator=(Serialize_Writer const&) = delete;
	Serialize_Writer& operator=(Serialize_Writer&&) = delete;

	Serialize_Writer(std::string const& filePathReference, bool isBinaryStream = false) : filePath{ filePathReference }
	{
		if (isBinaryStream)
		{
			ofs.open(filePath, std::ios::binary);
		}
		else ofs.open(filePath);
	}

	~Serialize_Writer()
	{
		// close all the shit free all the shit 
		ofs.clear();
		ofs.close();
	}

	bool IsStreamGood() const
	{
		return ofs.good();
	}

	template <typename T>
	bool WriteSimpleData(T const& data, size_t size)
	{
		if (!IsStreamGood())
		{
			std::cerr << "File Stream: " << filePath << " is not good!\n";
			return false;
		}
		ofs.write(reinterpret_cast<const char*>(&data), size);
		return true;
	}
	bool WriteGameObjectData(GameObject& GO, size_t) {
		auto* transform = GO.GetComponent<TransformComponent>();
		auto* render = GO.GetComponent<RenderComponent>();
		if (!transform || !render) {
			std::cerr << "Missing required components on GameObject\n";
			return false;
		}

		glm::vec4 color = render->GetColor();

		std::ofstream outFile("../Logs/GameObjectData.txt", std::ios::app);
		if (!outFile.is_open()) {
			std::cerr << "Failed to open file! Make sure ./Logs/ directory exists\n";
			return false;
		}

		outFile << "GO Name: " << GO.GetName() << " | Shape: " << render->GetShapeTypeName()
			<< " | Position (x,y): (" << transform->posX << "," << transform->posY << ")"
			<< " | Size (w,h): (" << transform->scaleX << "," << transform->scaleY << ")"
			<< " | Colour: (" << color.r << "," << color.g << "," << color.b << "," << color.a << ")"
			<< std::endl;

		outFile.close();
		std::cout << "Successfully wrote GameObject data to file!\n";
		return true;
	}
private:
	std::string filePath;
	std::ofstream ofs;
};


// Reader
class Serialize_Reader
{
public:
	// Delete these, we want properly defined filestreams
	Serialize_Reader() = delete;
	Serialize_Reader& operator=(Serialize_Reader const&) = delete;
	Serialize_Reader& operator=(Serialize_Reader&&) = delete;

	Serialize_Reader(std::string const& filePathReference, bool isBinaryStream = false) : filePath{ filePathReference }
	{
		if (isBinaryStream)
		{
			ifs.open(filePath, std::ios::binary);
		}
		else ifs.open(filePath);
	}

	~Serialize_Reader()
	{
		// close all the shit free all the shit 
		ifs.clear();
		ifs.close();
	}

	bool IsStreamGood() const
	{
		return ifs.good();
	}


	bool ReadSimpleData(char* destination, size_t size, std::stringstream& ss)
	{
		if (!IsStreamGood())
		{
			std::cerr << "File Stream: " << filePath << " is not good!\n";
			return false;
		}
		ifs.clear();
		ifs.read(destination, size);

		while (ifs.peek() != EOF) {
			ss << ifs.rdbuf();
		}
		//test string here will contain ur data
		std::string readData = ss.str();
		return true;
	}

	bool ReadGameObjectData(GameObject const&, size_t, std::stringstream& ss) {
		// name of GO, shape, pos (x,y), size (w, h), colour (vec4)
		if (!IsStreamGood())
		{
			std::cerr << "File Stream: " << filePath << " is not good!\n";
			return false;
		}

		std::string line;
		while (std::getline(ifs, line)) {
			ss << line << "\n";
		}
		return true;
	}

private:
	std::string filePath;
	std::ifstream ifs;
};

// Will store in logs for now
void CreatePrefab(GameObject& GO) 
{
	std::string filePath{ "../Logs/Prefab_" + GO.GetName() + ".txt" };
	std::ofstream ofs(filePath);

	if (!ofs.good()) 
	{
		std::cerr << "File Stream: " << filePath << " is not good!\n";
		return;
	}

	auto* transform = GO.GetComponent<TransformComponent>();
	auto* render = GO.GetComponent<RenderComponent>();
	glm::vec4 color = render->GetColor();

	// format: Name, Shape Type, PosX, PosY, ScaleX, ScaleY, Colour RGBA
	ofs << GO.GetName() << " " << render->GetShapeTypeName() << " " << transform->posX << " " << transform->posY << " "  << 
		transform->scaleX << " " << transform->scaleY << " " << color.r << " " << color.g << " " << color.b << " " << color.a << std::endl;

	std::cout << "Prefab created successfully!";
	ofs.clear();
	ofs.close();
}

std::string ReadPrefabData(const char* destination)
{
	std::ifstream ifs(destination);
	if (!ifs.good()) 
	{
		std::cerr << "File Stream: " << destination << " is not good!\n";
		ifs.clear();
		ifs.close();
		return "";
	}

	std::string data{};
	std::stringstream ss;

	ss << ifs.rdbuf();  // Read all content into buffer
	data = ss.str();

	ifs.clear();
	ifs.close();
	return data;
}


class Serializable {
public:
	virtual ~Serializable() = default;

	virtual json Serialize() const = 0;

	virtual void Deserialize(json const& j) = 0;

	virtual std::string GetTypeName() const = 0;
};
