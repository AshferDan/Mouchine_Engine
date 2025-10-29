#include "serialization.h"
#include "gameObject.h"
#include "RenderComponent.h"
#include "transform.h"
#include "physics.h"
#include "collidercomponent.h"
#include "collision.h"
#include "factory.h"

#include <fstream>
#include <sstream>
#include <iostream>
#include <filesystem>

namespace fs = std::filesystem;

// Utility helpers
namespace {
	static std::string ColliderTypeToString(ColliderComponent::Type type)
	{
		switch (type)
		{
		case ColliderComponent::Type::Square: return "Square";
		case ColliderComponent::Type::Circle: return "Circle";
		default: return "Unknown";
		}
	}

	static ColliderComponent::Type StringToColliderType(const std::string& str)
	{
		if (str == "Square") return ColliderComponent::Type::Square;
		if (str == "Circle") return ColliderComponent::Type::Circle;
		return ColliderComponent::Type::Square;
	}

	static std::string CollisionLayerToString(CollisionLayer layer)
	{
		switch (layer)
		{
		case CollisionLayer::Player:   return "Player";
		case CollisionLayer::Ground:   return "Ground";
		case CollisionLayer::Wall:     return "Wall";
		case CollisionLayer::Enemy:    return "Enemy";
		case CollisionLayer::Platform: return "Platform";
		default:                       return "Default";
		}
	}

	static CollisionLayer StringToCollisionLayer(const std::string& s)
	{
		if (s == "Player")   return CollisionLayer::Player;
		if (s == "Ground")   return CollisionLayer::Ground;
		if (s == "Wall")     return CollisionLayer::Wall;
		if (s == "Enemy")    return CollisionLayer::Enemy;
		if (s == "Platform") return CollisionLayer::Platform;
		return CollisionLayer::Default;
	}
}

std::vector<std::string> GetAllPrefabJSONFiles() {
	std::vector<std::string> files;
	const std::string dir = "../Logs/";

	for (const auto& entry : fs::directory_iterator(dir)) {
		if (entry.is_regular_file()) {
			std::string name = entry.path().filename().string();
			if (name.find("Prefab_") != std::string::npos && name.ends_with(".json")) {
				files.push_back(name);
			}
		}
	}

	return files;
}

json SerializeGameObject(GameObject* GO)
{
	json j;
	if (!GO) return j;

	j["name"] = GO->GetName();
	j["active"] = GO->IsActive();

	// ---- Transform ----
	if (const auto* t = GO->GetComponent<TransformComponent>())
	{
		j["TransformComponent"] = {
			{"pos", {t->posX, t->posY}},
			{"scale", {t->scaleX, t->scaleY}},
			{"rotation", t->GetRotation()}
		};
	}

	// ---- Render ----
	if (auto* r = GO->GetComponent<RenderComponent>())
	{
		auto color = r->GetColor();
		j["RenderComponent"] = {
			{"shapeType", r->GetShapeTypeName()},
			{"color", {color.x, color.y, color.z, color.w}}
		};
	}

	// --- Physics ---
	if (auto* p = GO->GetComponent<PhysicsComponent>())
	{
		j["PhysicsComponent"] = {
			{"mass", p->GetMass()},
			{"isStatic", p->IsStatic()},
			{"useGravity", p->useGravity},
			{"isGrounded", p->IsGrounded()},
			{"isJumping", p->isJumping},
			{"wasGrounded", p->wasGrounded},
			{"velocity", {p->velocityX, p->velocityY}},
			{"gravity",  {p->gravityX,  p->gravityY}}
		};
	}

	// --- Collider ---
	if (auto* c = GO->GetComponent<ColliderComponent>())
	{
		j["ColliderComponent"] = {
			{"shape", ColliderTypeToString(c->GetType())},
			{"layer", CollisionLayerToString(c->GetLayer())}
		};

		if (c->GetType() == ColliderComponent::Type::Square)
		{
			AABB box = c->GetAABB();
			float width = box.max.x - box.min.x;
			float height = box.max.y - box.min.y;
			j["ColliderComponent"]["width"] = width;
			j["ColliderComponent"]["height"] = height;
		}
		else if (c->GetType() == ColliderComponent::Type::Circle)
		{
			Circle circle = c->GetCircle();
			j["ColliderComponent"]["radius"] = circle.radius;
		}
	}

	return j;
}

void DeserializeGameObject(const json& j)
{
	GameObjectFactory& factory = GameObjectFactory::Instance();
	GameObject* GO = factory.CreateGameObject(j.value("name", "Unnamed"));
	GO->SetActive(j.value("active", true));

	// ---- Transform ----
	if (j.contains("TransformComponent"))
	{
		const auto& t = j["TransformComponent"];
		auto* tc = GO->AddComponent<TransformComponent>();
		tc->SetPosition(t["pos"][0], t["pos"][1]);
		tc->SetScale(t["scale"][0], t["scale"][1]);
		tc->SetRotation(t["rotation"]);
	}

	// ---- Render ----
	if (j.contains("RenderComponent"))
	{
		const auto& r = j["RenderComponent"];
		auto* rc = GO->AddComponent<RenderComponent>();

		std::string shape = r.value("shapeType", "Square");
		vec4 color(r["color"][0], r["color"][1], r["color"][2], r["color"][3]);

		RenderComponent::ShapeType shapeEnum{};
		ShapeEnumToString(shape, shapeEnum);
		rc->SetShapeType(shapeEnum);
		rc->SetColor(color);
	}

	// --- Physics ---
	if (j.contains("PhysicsComponent"))
	{
		const auto& p = j["PhysicsComponent"];
		auto* pc = GO->AddComponent<PhysicsComponent>();
		pc->useGravity = p.value("useGravity", true);
		pc->velocityX = p["velocity"][0];
		pc->velocityY = p["velocity"][1];
		pc->gravityX = p["gravity"][0];
		pc->gravityY = p["gravity"][1];
		pc->SetGrounded(p.value("isGrounded", false));
		pc->isJumping = p.value("isJumping", false);
		pc->wasGrounded = p.value("wasGrounded", false);
	}


	// --- Collider ---
	if (j.contains("ColliderComponent"))
	{
		const auto& c = j["ColliderComponent"];
		ColliderComponent::Type type = StringToColliderType(c.value("shape", "Square"));
		CollisionLayer layer = StringToCollisionLayer(c.value("layer", "Default"));

		float w = c.value("width", 0.0f);
		float h = c.value("height", 0.0f);

		if (type == ColliderComponent::Type::Circle)
			w = c.value("radius", 0.0f);

		GO->AddComponent<ColliderComponent>(type, w, h, layer);
	}
}

GameObject* CreateObj(
	const std::string& name,
	RenderComponent::ShapeType shapeType,
	const vec2& pos,
	const vec2& scale,
	const vec4& color,
	float rotation)
{
	auto& factory = GameObjectFactory::Instance();

	// --- Create GameObject ---
	GameObject* go = factory.CreateShape(
		name, shapeType,
		pos.x, pos.y,
		scale.x, scale.y,
		color,
		CollisionLayer::Default
	);

	// --- Transform ---
	auto* t = go->AddComponent<TransformComponent>(0, 0);
	t->SetPosition(pos.x, pos.y);
	t->SetScale(scale.x, scale.y);
	t->SetRotation(rotation);

	// --- Physics ---
	auto* physics = go->AddComponent<PhysicsComponent>();
	physics->SetMass(1.0f);
	physics->SetStatic(false);
	physics->useGravity = true;
	physics->SetGrounded(false);
	physics->isJumping = false;
	physics->wasGrounded = false;
	physics->velocityX = 0.0f;
	physics->velocityY = 0.0f;
	physics->gravityX = 0.0f;
	physics->gravityY = -9.8f;

	// --- Collider ---
	ColliderComponent::Type colliderType =
		(shapeType == RenderComponent::ShapeType::Circle)
		? ColliderComponent::Type::Circle
		: ColliderComponent::Type::Square;

	float width = scale.x;
	float height = scale.y;

	CollisionLayer layer = CollisionLayer::Player; // you can adjust this depending on context

	go->AddComponent<ColliderComponent>(colliderType, width, height, layer);

	std::cout << "[CreateObj] Created full GameObject '" << name
		<< "' with Transform, Render, Physics, and Collider.\n";

	return go;
}

void SavePrefab(GameObject& go, const std::string& existingPath)
{
	json j = SerializeGameObject(&go);

	std::string path = existingPath.empty()
		? "../Logs/Prefab_" + go.GetName() + ".json"
		: existingPath;

	json root;

	// If file exists and has content, load it first
	if (fs::exists(path))
	{
		std::ifstream inFile(path);
		if (inFile.is_open() && inFile.peek() != std::ifstream::traits_type::eof()) // not empty
		{
			try
			{
				inFile >> root;
			}
			catch (const std::exception& e)
			{
				std::cerr << "[SavePrefab] Warning: Invalid JSON detected, recreating file. (" << e.what() << ")\n";
				root = json::object();
			}
		}
		inFile.close();
	}

	if (!root.is_object())
		root = json::object();

	if (!root.contains("GameObjects") || !root["GameObjects"].is_array())
		root["GameObjects"] = json::array();

	root["GameObjects"].push_back(j);

	std::ofstream outFile(path, std::ios::trunc); // trunc clears old contents
	if (!outFile)
	{
		std::cerr << "[SavePrefab] Error: Cannot open file for writing: " << path << '\n';
		return;
	}

	outFile << root.dump(4);
	outFile.close();   // ensure file is properly closed

	std::cout << "[SavePrefab] Prefab saved or appended to: " << path << std::endl;
}

void LoadPrefab(const std::string& filePath)
{
	std::ifstream file(filePath);
	if (!file)
	{
		std::cerr << "[Serialization] Cannot open prefab: " << filePath << '\n';
		return;
	}

	json j;
	try { file >> j; }
	catch (const std::exception& e)
	{
		std::cerr << "[Serialization] Parse error: " << e.what() << '\n';
		return;
	}

	if (j.contains("GameObjects") && j["GameObjects"].is_array())
	{
		for (const auto& obj : j["GameObjects"])
		{
			DeserializeGameObject(obj);
		}
		std::cout << "[Serialization] Loaded prefab file with "
			<< j["GameObjects"].size() << " GameObjects.\n";
	}
	else if (j.contains("GameObject"))
	{
		DeserializeGameObject(j["GameObject"]);
		std::cout << "[Serialization] Loaded single GameObject prefab.\n";
	}
	else
	{
		DeserializeGameObject(j);
		std::cout << "[Serialization] Loaded simple JSON prefab (no array key).\n";
	}
}

// ========================= SCENE I/O =========================
void SaveScene(const std::string& filePath)
{
	json scene;
	scene["GameObjects"] = json::array();

	GameObjectFactory& factory = GameObjectFactory::Instance();
	for (const std::unique_ptr<GameObject>& goPtr : factory.GetAllGameObjects())
	{
		GameObject* go = goPtr.get();
		if (go && go->IsActive())
			scene["GameObjects"].push_back(SerializeGameObject(go));
	}


	std::ofstream file(filePath);
	if (!file)
	{
		std::cerr << "[Serialization] Cannot save scene: " << filePath << '\n';
		return;
	}

	file << scene.dump(4);
	std::cout << "[Serialization] Scene saved: " << filePath << '\n';
}

void LoadScene(const std::string& filePath)
{
	std::ifstream file(filePath);
	if (!file)
	{
		std::cerr << "[Serialization] Cannot open scene JSON: " << filePath << '\n';
		return;
	}

	json scene;
	try { file >> scene; }
	catch (const std::exception& e)
	{
		std::cerr << "[Serialization] Parse error: " << e.what() << '\n';
		return;
	}

	if (!scene.contains("GameObjects"))
	{
		std::cerr << "[Serialization] Invalid scene JSON\n";
		return;
	}

	for (const auto& obj : scene["GameObjects"])
		DeserializeGameObject(obj);

	std::cout << "[Serialization] Scene loaded: " << filePath << '\n';
}
