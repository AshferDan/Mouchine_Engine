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
	std::unordered_map<GLuint, std::string> s_TexturePathByID;
	std::unordered_map<std::string, GLuint> s_IDByTexturePath;

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
	if (const auto* t = GO->GetComponent<TransformComponent>()) {
		j["TransformComponent"] = {
			{"pos", {t->posX, t->posY}},
			{"scale", {t->scaleX, t->scaleY}},
			{"rotation", t->GetRotation()}
		};
	}

	// ---- Render ----
	if (auto* r = GO->GetComponent<RenderComponent>()) {
		j["RenderComponent"]["shapeType"] = r->GetShapeTypeName();

		std::string texPath;
		auto it = s_TexturePathByID.find(r->GetTextureID());
		texPath = (it != s_TexturePathByID.end()) ? it->second : "";

		j["RenderComponent"]["texturePath"] = texPath;

		vec4 c = r->GetColor();
		j["RenderComponent"]["color"] = { c.x, c.y, c.z, c.w };
	}

	// --- Spritesheet ---
	if (auto* s = GO->GetComponent<Spritesheet>()) {
		j["Spritesheet"] = {
			{"rows", s->GetRows()},
			{"cols", s->GetCols()},
			{"startFrame", s->GetCurrentStartFrame()},
			{"endFrame", s->GetCols() * s->GetRows() - 1},
			{"fps", s->GetTimePerFrame()}
		};
	}

	// --- Physics ---
	if (auto* p = GO->GetComponent<PhysicsComponent>()) {
		j["PhysicsComponent"] = {
			{"mass", p->GetMass()},
			{"isStatic", p->IsStatic()},
			{"useGravity", p->useGravity},
			{"isGrounded", p->IsGrounded()},
			{"velocity", {p->velocityX, p->velocityY}},
		};
	}

	// --- Collider ---
	if (auto* c = GO->GetComponent<ColliderComponent>()) {
		j["ColliderComponent"] = {
			{"shape", ColliderTypeToString(c->GetType())},
			{"layer", CollisionLayerToString(c->GetLayer())}
		};		
	}

	return j;
}

void DeserializeGameObject(const json& j)
{
	GameObjectFactory& factory = GameObjectFactory::Instance();
	std::string name = j.value("name", "Unnamed");

	GameObject* GO = factory.CreateGameObject(name);
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

	// ---- Render / Texture ----
	if (j.contains("RenderComponent")) {
		const auto& r = j["RenderComponent"];
		std::string shapeType = r.value("shapeType", "Square");
		std::string texPath = r.value("texturePath", "");
		RenderComponent::ShapeType shape;
		ShapeEnumToString(shapeType, shape);

		auto* render = GO->AddComponent<RenderComponent>();
		render->SetShapeType(shape);

		// === Textured object ===
		if (!texPath.empty()) {
			GLuint texID = GraphicsSystem::LoadTexture(texPath.c_str());
			render->SetTextureID(texID);
			render->SetColor({ 1, 1, 1, 1 });  // remove tint

			// --- Clamp texture edges (avoid half-color issue) ---
			glBindTexture(GL_TEXTURE_2D, texID);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			glBindTexture(GL_TEXTURE_2D, 0);
		}
		else {
			// === Color-only object ===
			if (r.contains("color")) {
				vec4 c(r["color"][0], r["color"][1], r["color"][2], r["color"][3]);
				render->SetColor(c);
			}
			else {
				render->SetColor({ 1.0f, 1.0f, 1.0f, 1.0f });
			}
		}
	}


	// --- Spritesheet ---
	if (j.contains("Spritesheet")) {
		const auto& s = j["Spritesheet"];
		std::string texPath = j["RenderComponent"].value("texturePath", "");

		auto* spr = GO->AddComponent<Spritesheet>(
			texPath.c_str(),
			s.value("rows", 1),
			s.value("cols", 1)
		);
		spr->LoadNewSpritesheet(texPath.c_str(),
			s.value("rows", 1),
			s.value("cols", 1));
		spr->SetAnimation(
			s.value("startFrame", 0),
			s.value("endFrame", 0),
			s.value("fps", 1.0f));
	}

	// --- Physics ---
	if (j.contains("PhysicsComponent")) {
		const auto& p = j["PhysicsComponent"];
		auto* pc = GO->AddComponent<PhysicsComponent>();
		pc->SetStatic(p.value("isStatic", false));
		pc->SetGravity(p.value("useGravity", true));
		pc->SetMass(p.value("mass", 1.0f));

		if (p.contains("velocity")) {
			pc->SetVelocity(p["velocity"][0].get<float>(), p["velocity"][1].get<float>());

			pc->SetGrounded(p.value("isGrounded", false));
		}
	}

	// --- Collider ---
	if (j.contains("ColliderComponent")) {
		const auto& c = j["ColliderComponent"];
		ColliderComponent::Type type = StringToColliderType(c.value("shape", "Square"));
		CollisionLayer layer = StringToCollisionLayer(c.value("layer", "Default"));
		float width = j["TransformComponent"]["scale"][0].get<float>();
		float height = j["TransformComponent"]["scale"][1].get<float>();
		GO->AddComponent<ColliderComponent>(type, width, height, layer);
	}

	std::cout << "[Deserialize] " << name
		<< " pos(" << j["TransformComponent"]["pos"][0]
		<< "," << j["TransformComponent"]["pos"][1]
		<< ") scale("
		<< j["TransformComponent"]["scale"][0]
		<< "," << j["TransformComponent"]["scale"][1]
		<< ")\n";
	if (auto* r = GO->GetComponent<RenderComponent>()) {
		std::cout << "[Render] " << name
			<< " TextureID: " << r->GetTextureID()
			<< " Path: " << r->GetShapeTypeName()
			<< "\n";
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
	std::string path = existingPath.empty()
		? "../Logs/Prefab_" + go.GetName() + ".json"
		: existingPath;

	json root;
	if (fs::exists(path)) {
		std::ifstream in(path);
		if (in.peek() != std::ifstream::traits_type::eof()) {
			try { in >> root; }
			catch (...) { root = json::object(); }
		}
	}

	if (!root.contains("GameObjects"))
		root["GameObjects"] = json::array();

	root["GameObjects"].push_back(SerializeGameObject(&go));

	std::ofstream out(path, std::ios::trunc);
	out << root.dump(4);
	out.close();

	std::cout << "[SavePrefab] Saved prefab: " << path << "\n";
}

void LoadPrefab(const std::string& path)
{
	std::ifstream in(path);
	if (!in.is_open()) {
		std::cerr << "[LoadPrefab] Cannot open " << path << "\n";
		return;
	}

	json j;
	try { in >> j; }
	catch (const std::exception& e) {
		std::cerr << "[LoadPrefab] Parse error: " << e.what() << "\n";
		return;
	}

	if (j.contains("GameObjects")) {
		for (const auto& obj : j["GameObjects"])
			DeserializeGameObject(obj);
	}
	else {
		DeserializeGameObject(j);
	}

	std::cout << "[LoadPrefab] Loaded prefab: " << path << "\n";
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
