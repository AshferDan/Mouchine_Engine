#pragma once

#include "System.h"
#include <string>
class ScriptSystem : ISystem {

public:
	~ScriptSystem() override;

	void Initialize() override;
	
	std::string GetName() override {
		return "ScriptingSystem";
	}
	
	void Update(float) override {};
};

