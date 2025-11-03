/* Start Header ************************************************************************/
/*!
\file       Serialization.cpp
\author     Lee Hwee Min [100% - ?? lines]
\par        l.hweemin@digipen.edu
\date       November, 3, 2025
Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*/
/* End Header **************************************************************************/

#include "serialization.h"

#include <fstream>
#include <sstream>
#include <iostream>
#include <filesystem>

namespace fs = std::filesystem;

/**
 * @brief Converts a GameObject and its components into a JSON object.
 *
 * Collects GameObject properties such as name and active state,
 * then delegates the serialization of all attached components to
 * ComponentSerialization.
 *
 * @param go Pointer to the GameObject to serialize.
 * @return JSON object representing the serialized GameObject.
 */
json Serialization::SerializeGameObject(GameObject *go)
{
	json j;
	if (!go)
		return j;

	j["name"] = go->GetName();
	j["active"] = go->IsActive();

	ComponentSerialization::SerializeAllComponents(go, j);
	return j;
}

/**
 * @brief Creates a GameObject from JSON data and registers it with the factory.
 *
 * Reads GameObject attributes such as name and active state, and reconstructs
 * all serialized components through ComponentSerialization.
 *
 * @param j JSON object describing the GameObject.
 */
void Serialization::DeserializeGameObject(const json &j)
{
	GameObjectFactory &factory = GameObjectFactory::Instance();
	std::string name = j.value("name", "Unnamed");

	GameObject *go = factory.CreateGameObject(name);
	go->SetActive(j.value("active", true));

	ComponentSerialization::DeserializeAllComponents(go, j);
}

/**
 * @brief Saves one or more GameObjects to a prefab JSON file.
 *
 * Supports two modes:
 * - **Append Mode:** Adds a single GameObject to an existing JSON prefab.
 * - **Overwrite Mode:** Serializes all active GameObjects and replaces the file.
 *
 * @param go            Reference to the GameObject to save or append.
 * @param existingPath  Optional file path to an existing prefab file.
 * @param append        True to append, false to overwrite the entire prefab.
 *
 * @note When overwrite mode is used, all active GameObjects in the
 *       GameObjectFactory are serialized together.
 */
void Serialization::SavePrefab(GameObject &go, const std::string &existingPath, bool append)
{
	std::string path = existingPath.empty()
						   ? "Assets/Prefabs/Prefab_" + go.GetName() + ".json"
						   : existingPath;

	json root;

	// Append � add new object to existing prefab file
	if (append && fs::exists(path))
	{
		std::ifstream in(path);
		if (in.peek() != std::ifstream::traits_type::eof())
		{
			try
			{
				in >> root;
			}
			catch (...)
			{
				root = json::object();
			}
		}
		in.close();

		if (!root.contains("GameObjects"))
			root["GameObjects"] = json::array();

		root["GameObjects"].push_back(Serialization::SerializeGameObject(&go));
	}

	// Overwrite � reserialize *all* current GameObjects
	else
	{
		root = json::object();
		root["GameObjects"] = json::array();

		const auto &objs = GameObjectFactory::Instance().GetAllGameObjects();
		for (const auto &objPtr : objs)
		{
			if (objPtr && objPtr->IsActive())
			{
				root["GameObjects"].push_back(Serialization::SerializeGameObject(objPtr.get()));
			}
		}
	}

	return files;
}

// ------------------------------------------------------------
// Serialization Helpers
// ------------------------------------------------------------

static void SerializeTransform(json &j, const TransformComponent *t)
{
	j["TransformComponent"] = {
		{"pos", {t->posX, t->posY}},
		{"scale", {t->scaleX, t->scaleY}},
		{"rotation", t->GetRotation()}};
}

static void SerializeRender(json &j, RenderComponent *r)
{
	j["RenderComponent"]["shapeType"] = r->GetShapeTypeName();

	std::string texPath = r->GetTexturePath();
	// if no path stored in component, try the lookup map as fallback
	if (texPath.empty())
	{
		auto it = s_TexturePathByID.find(r->GetTextureID());
		texPath = (it != s_TexturePathByID.end()) ? it->second : "";
	}

	j["RenderComponent"]["texturePath"] = texPath;

	vec4 c = r->GetColor();
	j["RenderComponent"]["color"] = {c.x, c.y, c.z, c.w};
}

static void SerializeSpritesheet(json &j, Spritesheet *s)
{
	j["Spritesheet"] = {
		{"rows", s->GetRows()},
		{"cols", s->GetCols()},
		{"startFrame", s->GetCurrentStartFrame()},
		{"endFrame", s->GetCols() * s->GetRows() - 1},
		{"fps", s->GetTimePerFrame()}};
}

static void SerializePhysics(json &j, PhysicsComponent *p)
{
	j["PhysicsComponent"] = {
		{"mass", p->GetMass()},
		{"isStatic", p->IsStatic()},
		{"useGravity", p->useGravity},
		{"isGrounded", p->IsGrounded()},
		{"velocity", {p->velocityX, p->velocityY}}};
}

static void SerializeCollider(json &j, ColliderComponent *c)
{
	j["ColliderComponent"] = {
		{"shape", ColliderTypeToString(c->GetType())},
		{"layer", CollisionLayerToString(c->GetLayer())}};
}

// ------------------------------------------------------------
// Deserialization Helpers
// ------------------------------------------------------------

static void DeserializeTransform(const json &j, GameObject *go)
{
	if (!j.contains("TransformComponent"))
		return;
	const auto &t = j["TransformComponent"];
	auto *tc = go->AddComponent<TransformComponent>();
	tc->SetPosition(t["pos"][0], t["pos"][1]);
	tc->SetScale(t["scale"][0], t["scale"][1]);
	tc->SetRotation(t["rotation"]);
}

static void DeserializeRender(const json &j, GameObject *go)
{
	if (!j.contains("RenderComponent"))
		return;
	const auto &r = j["RenderComponent"];

	std::string shapeType = r.value("shapeType", "Square");
	std::string texPath = r.value("texturePath", "");

	auto *render = go->AddComponent<RenderComponent>();
	render->SetShapeType(RenderComponentTools::ShapeEnumToString(shapeType));

	if (!texPath.empty())
	{
		// try to get texture from resource manager
		auto &resourceMgr = ResourceManager::Instance();
		std::string goName = go->GetName();
		TextureResource *texResource = resourceMgr.GetTexture(goName);
		GLuint texID = 0;

		if (texResource && texResource->IsLoaded())
		{
			// use existing texture from resource manager
			texID = texResource->GetTextureID();
			std::cout << "Serialization using texture from resource manager: " << goName << std::endl;
		}
		else
		{
			// not in resource manager, load it and register it for future use
			std::cout << "Serialization texture not in ResourceManager, loading and registering: "
					  << goName << " -> " << texPath << std::endl;

			if (resourceMgr.LoadTexture(goName, texPath))
			{
				texResource = resourceMgr.GetTexture(goName);
				if (texResource && texResource->IsLoaded())
				{
					texID = texResource->GetTextureID();
				}
			}
			else
			{
				// load directly
				std::cout << "Serialization warning: Failed to load via resource manager, directly loading" << std::endl;
				texID = GraphicsSystem::LoadTexture(texPath.c_str());
			}
		}

		render->SetTextureID(texID);
		render->SetTexturePath(texPath); // store the path for future serialization
		render->SetColor({1, 1, 1, 1});

		glBindTexture(GL_TEXTURE_2D, texID);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glBindTexture(GL_TEXTURE_2D, 0);
	}
	else if (r.contains("color"))
	{
		vec4 c(r["color"][0], r["color"][1], r["color"][2], r["color"][3]);
		render->SetColor(c);
	}
	else
	{
		render->SetColor({1, 1, 1, 1});
	}
}

static void DeserializeSpritesheet(const json &j, GameObject *go)
{
	if (!j.contains("Spritesheet"))
		return;
	const auto &s = j["Spritesheet"];
	std::string texPath = j["RenderComponent"].value("texturePath", "");

	auto *spr = go->AddComponent<Spritesheet>(
		texPath.c_str(),
		s.value("rows", 1),
		s.value("cols", 1));
	spr->LoadNewSpritesheet(texPath.c_str(),
							s.value("rows", 1),
							s.value("cols", 1));
	spr->SetAnimation(
		s.value("startFrame", 0),
		s.value("endFrame", 0),
		s.value("fps", 1.0f));
}

static void DeserializePhysics(const json &j, GameObject *go)
{
	if (!j.contains("PhysicsComponent"))
		return;
	const auto &p = j["PhysicsComponent"];

	auto *pc = go->AddComponent<PhysicsComponent>();
	pc->SetStatic(p.value("isStatic", false));
	pc->SetGravity(p.value("useGravity", true));
	pc->SetMass(p.value("mass", 1.0f));

	if (p.contains("velocity"))
		pc->SetVelocity(p["velocity"][0].get<float>(), p["velocity"][1].get<float>());

	pc->SetGrounded(p.value("isGrounded", false));
}

static void DeserializeCollider(const json &j, GameObject *go)
{
	if (!j.contains("ColliderComponent"))
		return;
	const auto &c = j["ColliderComponent"];

	ColliderComponent::Type type = StringToColliderType(c.value("shape", "Square"));
	CollisionLayer layer = StringToCollisionLayer(c.value("layer", "Default"));
	float width = j["TransformComponent"]["scale"][0].get<float>();
	float height = j["TransformComponent"]["scale"][1].get<float>();

	go->AddComponent<ColliderComponent>(type, width, height, layer);
}

// ------------------------------------------------------------
// GameObject Serialization / Deserialization
// ------------------------------------------------------------

json SerializeGameObject(GameObject *GO)
{
	json j;
	if (!GO)
		return j;

	j["name"] = GO->GetName();
	j["active"] = GO->IsActive();

	if (const auto *t = GO->GetComponent<TransformComponent>())
		SerializeTransform(j, t);
	if (auto *r = GO->GetComponent<RenderComponent>())
		SerializeRender(j, r);
	if (auto *s = GO->GetComponent<Spritesheet>())
		SerializeSpritesheet(j, s);
	if (auto *p = GO->GetComponent<PhysicsComponent>())
		SerializePhysics(j, p);
	if (auto *c = GO->GetComponent<ColliderComponent>())
		SerializeCollider(j, c);

	return j;
}

void DeserializeGameObject(const json &j)
{
	GameObjectFactory &factory = GameObjectFactory::Instance();
	std::string name = j.value("name", "Unnamed");
	GameObject *GO = factory.CreateGameObject(name);
	GO->SetActive(j.value("active", true));

	DeserializeTransform(j, GO);
	DeserializeRender(j, GO);
	DeserializeSpritesheet(j, GO);
	DeserializePhysics(j, GO);
	DeserializeCollider(j, GO);
}

// ------------------------------------------------------------
// Create GameObject Serialization / Deserialization
// ------------------------------------------------------------
GameObject *CreateObj(
	const std::string &name,
	RenderComponent::ShapeType shapeType,
	const vec2 &pos,
	const vec2 &scale,
	const vec4 &color,
	float rotation)
{
	auto &factory = GameObjectFactory::Instance();

	// --- Create GameObject ---
	GameObject *go = factory.CreateShape(
		name, shapeType,
		pos.x, pos.y,
		scale.x, scale.y,
		color,
		CollisionLayer::Default);

	// --- Transform ---
	auto *t = go->AddComponent<TransformComponent>(0, 0);
	t->SetPosition(pos.x, pos.y);
	t->SetScale(scale.x, scale.y);
	t->SetRotation(rotation);

	// --- Physics ---
	auto *physics = go->AddComponent<PhysicsComponent>();
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

// ------------------------------------------------------------
// Prefab Save / Load
// ------------------------------------------------------------
void SavePrefab(GameObject &go, const std::string &existingPath)
{
	std::string path = existingPath.empty()
						   ? "../Logs/Prefab_" + go.GetName() + ".json"
						   : existingPath;

	json root;
	if (fs::exists(path))
	{
		std::ifstream in(path);
		if (in.peek() != std::ifstream::traits_type::eof())
		{
			try
			{
				in >> root;
			}
			catch (...)
			{
				root = json::object();
			}
		}
	}

	if (!root.contains("GameObjects"))
		root["GameObjects"] = json::array();

	root["GameObjects"].push_back(SerializeGameObject(&go));

	std::ofstream out(path, std::ios::trunc);
	if (out.is_open())
	{
		out << root.dump(4);
		out.close();
	}

	std::cout << "[SavePrefab] "
			  << (append ? "Appended to" : "Overwrote with updated scene: ")
			  << path << "\n";
}

/**
 * @brief Loads and reconstructs GameObjects from a prefab JSON file.
 *
 * Reads the prefab file from disk, parses its JSON data, and recreates
 * all GameObjects within the scene using the GameObjectFactory.
 *
 * @param path Full file path to the prefab JSON file.
 */
void Serialization::LoadPrefab(const std::string &path)
{
	if (!fs::exists(path))
	{
		std::cerr << "[Serialization] Prefab not found: " << path << "\n";
		return;
	}

	std::ifstream in(path);
	if (!in.is_open())
	{
		std::cerr << "[Serialization] Cannot open prefab: " << path << "\n";
		return;
	}

	json j;
	try
	{
		in >> j;
	}
	catch (const std::exception &e)
	{
		std::cerr << "[Serialization] Parse error: " << e.what() << "\n";
		return;
	}

	GameObjectFactory::Instance().Clear();

	if (j.contains("GameObjects"))
	{
		for (const auto &obj : j["GameObjects"])
			DeserializeGameObject(obj);
	}
	else
	{
		DeserializeGameObject(j);
	}

	std::cout << "[Serialization] Loaded prefab: " << path << "\n";
}
