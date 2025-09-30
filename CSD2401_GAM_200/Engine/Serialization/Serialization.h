#pragma once
#include <iostream>
#include <string>
#include <fstream>
#include <nlohmann/json.hpp> 
#include "gameObject.h"
#include "RenderComponent.h"
#include "transform.h"

using json = nlohmann::json;

class RenderComponent;
class TransformComponent;

// Writer
class Serialize_Writer : public IComponent
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
		auto
		if (!IsStreamGood()) 
		{
			std::cerr << "File Stream: " << filePath << " is not good!\n";
			return false;
		}
		ofs.write(static_cast<const char*>(data), size);
		return true;
	}

	bool WriteGameObjectData(GameObject const& GO, size_t size) {
		// hardcoding for M1 -> todo: automate with class functions for specific types
		// name of GO, shape, pos (x,y), size (w, h), colour (vec4)
		std::stringstream ss{};

		auto* transform = owner->GetComponent<TransformComponent>();
		auto* render = owner->GetComponent<RenderComponent>();

		ss << "GO Name: " << GO.GetName() << " | Shape: " <<  render->GetShapeTypeName() <<
			" | Position (x,y): (" <<  transform->posX << "," << transform->posY <<
			") | Size (w,h): (" << transform->scaleX << "," << transform->scaleY  <<
			") | Colour: " << render->GetColor() << std::endl;

		// Feed stingstream into char* then can write 
		//ofs.write();
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

	Serialize_Reader(std::string const& filePathReference, bool isBinaryStream) : filePath{ filePathReference }
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
		ifs.read(destination, size);

		while (ifs.peek() != EOF) {
			ss << ifs.rdbuf();
		}
		//test string here will contain ur data
		std::string readData = ss.str();
		return true;
	}

	bool ReadGameObjectData(GameObject const& GO, size_t size, std::stringstream& ss) {
		// name of GO, shape, pos (x,y), size (w, h), colour (vec4)
	}

private:
	std::string filePath;
	std::ifstream ifs;
};






class Serializable {
	virtual ~Serializable ()= default;
	
	virtual json Serialize() const = 0;
	
	virtual void Deserialize() const = 0 ;
	
	virtual std::string GetTypeName() const = 0;
};
std::ostream& operator<<(std::ostream& os, GameObject const& GO);

// TESTING STUFF AAAAAA
int test1;
short test2;
bool test3;

void Serialize(std::ostream& os) 
{
	os.write(reinterpret_cast<const char*>(&test1), sizeof(test1));
	os.write(reinterpret_cast<const char*>(&test2), sizeof(test2));
	os.write(reinterpret_cast<const char*>(&test3), sizeof(test3));
}
//std::ofstream(ofilename, std::ios::binary)

void TestPrint() {
	std::ofstream txtFile("test.txt");
	txtFile << test1 << " " << test2 << " " << test3 << "\n";

	std::ofstream binaryFile("test.bin", std::ios::binary);
	Serialize(binaryFile);
}
