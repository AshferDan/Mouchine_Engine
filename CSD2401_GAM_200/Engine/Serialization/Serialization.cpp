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

using namespace std;
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
}

void CreatePrefabJSON(GameObject& GO)
{
	json j = SerializeGameObject(&GO);
	string path = "../Logs/Prefab_" + GO.GetName() + ".json";

	ofstream file(path);
	if (!file)
	{
		cerr << "[Serialization] Cannot write prefab: " << path << '\n';
		return;
	}
	file << j.dump(4);
	cout << "[Serialization] Prefab saved: " << path << '\n';
}

void LoadPrefabJSON(const std::string& filePath)
{
	ifstream file(filePath);
	if (!file)
	{
		cerr << "[Serialization] Cannot open prefab: " << filePath << '\n';
		return;
	}

	json j;
	try { file >> j; }
	catch (const exception& e)
	{
		cerr << "[Serialization] Parse error: " << e.what() << '\n';
		return;
	}

	if (j.contains("GameObject"))
		DeserializeGameObject(j["GameObject"]);
	else
		DeserializeGameObject(j);
}

// ========================= SCENE I/O =========================
void SaveSceneJSON(const std::string& filePath)
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


	ofstream file(filePath);
	if (!file)
	{
		cerr << "[Serialization] Cannot save scene: " << filePath << '\n';
		return;
	}

	file << scene.dump(4);
	cout << "[Serialization] Scene saved: " << filePath << '\n';
}

void LoadSceneJSON(const std::string& filePath)
{
	ifstream file(filePath);
	if (!file)
	{
		cerr << "[Serialization] Cannot open scene JSON: " << filePath << '\n';
		return;
	}

	json scene;
	try { file >> scene; }
	catch (const exception& e)
	{
		cerr << "[Serialization] Parse error: " << e.what() << '\n';
		return;
	}

	if (!scene.contains("GameObjects"))
	{
		cerr << "[Serialization] Invalid scene JSON\n";
		return;
	}

	for (const auto& obj : scene["GameObjects"])
		DeserializeGameObject(obj);

	cout << "[Serialization] Scene loaded: " << filePath << '\n';
}
