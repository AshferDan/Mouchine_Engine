#pragma once
#include <iostream>
#include <string>
#include <nlohmann/json.hpp> 

using json = nlohmann::json;
class Serializable {
	virtual ~Serializable ()= default;
	
	virtual json Serialize() const = 0;
	
	virtual void Deserialize() const = 0 ;
	
	virtual std::string GetTypeName() const = 0;

};