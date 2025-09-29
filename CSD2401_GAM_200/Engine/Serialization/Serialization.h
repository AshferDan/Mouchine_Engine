#pragma once
#include <iostream>
#include <string>
#include <fstream>
#include <nlohmann/json.hpp> 
#include "gameObject.h"

using json = nlohmann::json;

// IM MAKING MY OWN SERIALISATION
// Will move into cpp after i get it all working
class Serialize_Writer
{
public:
	Serialize_Writer() : filePath{}, ofs{} {}		// NOT RECOMMENDED, HELL I EVEN MIGHT DELETE THIS!!!!

	Serialize_Writer(std::string const& path) 
	{

	}

	Serialize_Writer(std::ofstream const& ofsReference)
	{

	}

	~Serialize_Writer() 
	{
		// close all the shit free all the shit 
	}

	bool IsStreamGood() const
	{
		return ofs.good();
	}

	bool WriteSimpleData(const char* data, size_t size) 
	{
		ofs.write(data, size);
		return true;
	}

	bool WriteGameObjectData(GameObject const& GO, size_t size) {
		// need a way to return a container (maybe vec? or map?) of components
		//for (auto const& comp : GameObject::GetComponent(GO)) {
		//	// ofs.write data here 
		//}
	}

private:
	std::string filePath;
	std::ofstream ofs;
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
