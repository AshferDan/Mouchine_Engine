#include "ScriptComponent.h"
#include "core.h"
#include "ScriptingSystem.h"
#include "GameObject.h"
#include <iostream>
#include <filesystem>

namespace fs = std::filesystem;

ScriptComponent::ScriptComponent(GameObject* owner)
	: IComponent(owner), NextScript_ID(0){}
void ScriptComponent::Initialize()
{
	if (!CORE)
	{
		std::cerr << "[ScriptComponent::Initialize] ERROR: CORE is null!" << std::endl;
		return;
	}

	auto scriptSys = CORE->GetScriptSystem();
	if (!scriptSys)
	{
		std::cerr << "[ScriptComponent::Initialize] ERROR: ScriptSystem is null!" << std::endl;
		return;
	}

	if (ActiveScript.empty())
	{
		std::cout << "[ScriptComponent::Initialize] No scripts in ActiveScript map for "
			<< (owner ? owner->GetName() : "null owner") << std::endl;
		return;
	}

	std::cout << "[ScriptComponent::Initialize] Initializing " << ActiveScript.size()
		<< " scripts for " << (owner ? owner->GetName() : "null owner") << std::endl;

	// Check if scripts are already attached to ScriptSystem
	auto* existingInstances = scriptSys->GetScriptInstances(owner);
	size_t alreadyAttached = existingInstances ? existingInstances->size() : 0;

	for (const auto& [id, scriptName] : ActiveScript) {
		// Skip if this script count suggests scripts are already attached
		// This prevents duplicate script attachments
		if (alreadyAttached > 0)
		{
			std::cout << "[ScriptComponent::Initialize] Script '" << scriptName
				<< "' likely already attached (existing instances: " << alreadyAttached << "), skipping" << std::endl;
			continue;
		}

		std::cout << "[ScriptComponent::Initialize] Attaching script: " << scriptName << std::endl;
		bool success = scriptSys->AttachScript(owner, scriptName);
		if (success) {
			std::cout << "[ScriptComponent::Initialize] Successfully initialized script: " << scriptName << std::endl;
		}
		else {
			std::cerr << "[ScriptComponent::Initialize] Failed to initialize: " << scriptName << std::endl;
		}
	}
}

void ScriptComponent::Update(float dt)
{
	(void)dt;

	for (auto& slot : _propertySlot)
	{
		std::string data = slot.GetReceivedData();
		if (!data.empty())
		{
			// Extract script name from path
			fs::path scriptPath(data);
			std::string scriptName = scriptPath.stem().string();

			AddScript(scriptName);
			slot.ClearReceivedData();
		}
	}
}

void ScriptComponent::Destroy()
{
}

std::unique_ptr<IComponent> ScriptComponent::Clone(GameObject* NewClone) const
{
	auto newComp = std::make_unique<ScriptComponent>(NewClone);

	
	for (const auto& [id, scriptName] : ActiveScript)
	{
		newComp->AddScript(scriptName); 
	}

	return newComp;
}
void ScriptComponent::AddScript(const std::string& scriptName) {
	// Check if script already exists
	for (const auto& [id, existingName] : ActiveScript)
	{
		if (existingName == scriptName) {
			std::cout << "[ScriptComponent::AddScript] Script '" << scriptName << "' already exists in ActiveScript map" << std::endl;
			return; // Exit early if duplicate
		}
	}

	// Add to our internal map FIRST
	ScriptID newID = NextScript_ID++;
	ActiveScript[newID] = scriptName;
	std::cout << "[ScriptComponent::AddScript] Added '" << scriptName << "' to ActiveScript with ID: " << newID << std::endl;
	std::cout << "[ScriptComponent::AddScript] ActiveScript now has " << ActiveScript.size() << " scripts" << std::endl;

	// IMMEDIATELY attach to ScriptSystem if owner exists
	if (owner)
	{
		if (!CORE)
		{
			std::cerr << "[ScriptComponent::AddScript] ERROR: CORE is null!" << std::endl;
			return;
		}

		auto scriptSys = CORE->GetScriptSystem();
		if (scriptSys)
		{
			std::cout << "[ScriptComponent::AddScript] Attempting to attach '" << scriptName << "' to ScriptSystem for " << owner->GetName() << std::endl;
			bool success = scriptSys->AttachScript(owner, scriptName);
			if (success)
			{
				std::cout << "[ScriptComponent::AddScript] Successfully attached '" << scriptName << "' to " << owner->GetName() << std::endl;

				// Call OnStart immediately after attachment
				scriptSys->CallOnStart(owner);
			}
			else
			{
				std::cerr << "[ScriptComponent::AddScript] ERROR: Failed to attach '" << scriptName << "' to ScriptSystem" << std::endl;
			}
		}
		else
		{
			std::cerr << "[ScriptComponent::AddScript] ERROR: ScriptSystem not available!" << std::endl;
		}
	}
	else
	{
		std::cerr << "[ScriptComponent::AddScript] WARNING: owner is null, script added to ActiveScript but not attached to ScriptSystem" << std::endl;
	}
}
void ScriptComponent::RemoveScript(const std::string& scriptName)
{
	ScriptID id = nameToID(scriptName);
	if (id != -1) // Assuming -1 or MAX_UINT is invalid
	{
		RemoveScript(id);
	}
}

void ScriptComponent::RemoveScript(ScriptID id)
{
	auto it = ActiveScript.find(id);
	if (it != ActiveScript.end())
	{
		std::cout << "ScriptComponent Removing " << it->second << " (ID: " << id << ")\n";
		ActiveScript.erase(it);
	}
}

std::string ScriptComponent::idToName(ScriptID id)
{
	auto it = ActiveScript.find(id);
	if (it != ActiveScript.end())
	{
		return it->second;
	}
	return ""; 
}

ScriptID ScriptComponent::nameToID(const std::string& name)
{
	// Linear search since map is keyed by ID
	for (auto& [id, scriptName] : ActiveScript)
	{
		if (scriptName == name)
		{
			return id;
		}
	}
	return (ScriptID) - 1;
}
