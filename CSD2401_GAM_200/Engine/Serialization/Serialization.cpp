#include "Serialization.h"


Serializable::~Serializable() 
{
	// free memory/ close files streams here
}

json Serializable::Serialize() const
{

}

void Serializable::Deserialize() const
{

}

std::string Serializable::GetTypeName() const
{

}

std::ostream& operator<<(std::ostream& os, GameObject const& GO) 
{
	os << "-|| GO Name: " << GO.GetName() << " | ID: "  << GO.GetID() << " ||-\n";
}

//void Serialize(std::ostream& os) {
//	os.write(reinterpret_cast<const char*>(&test1), sizeof(test1));
//	os.write(reinterpret_cast<const char*>(&test2), sizeof(test2));
//	os.write(reinterpret_cast<const char*>(&test3), sizeof(test3));
//}