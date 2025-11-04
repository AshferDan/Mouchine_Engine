/* Start Header************************************************************************ /
/*!
\file       ComponentSerialization.cpp
\author     Lee Hwee Min [100% - ?? lines]
\par        l.hweemin@digipen.edu
\date       November, 2, 2025
Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*/
/* End Header **************************************************************************/

#include "ComponentSerialization.h"
#include "Animator.hpp"

std::unordered_map<GLuint, std::string> s_TexturePathByID;
std::unordered_map<std::string, GLuint> s_IDByTexturePath;
/*!
* @brief Implements JSON serialization and deserialization logic for each
*    component type. This file isolates per-component data handling so
*    the core serializer remains clean and extendable.
*/

// ------------------- Serialize -------------------
/**
 * @brief Serializes the TransformComponent into JSON.
 * @param j JSON object to write into.
 * @param t Pointer to the TransformComponent.
 */
void ComponentSerialization::SerializeTransform(json& j, const TransformComponent* t)
{
    j["TransformComponent"] = {
        {"pos",   {t->posX, t->posY}},
        {"scale", {t->scaleX, t->scaleY}},
        {"rotation", t->GetRotation()}
    };
}

/**
 * @brief Serializes the RenderComponent into JSON, including color and texture data.
 * @param j JSON object to write into.
 * @param r Pointer to the RenderComponent.
 */
void ComponentSerialization::SerializeRender(json& j, RenderComponent* r)
{
    std::string texPath = r->GetTexturePath();

    if (texPath.empty()) {
        auto it = s_TexturePathByID.find(r->GetTextureID());
        texPath = (it != s_TexturePathByID.end()) ? it->second : "";
    }

    j["RenderComponent"] = {
        {"shapeType", r->GetShapeTypeName()},
        {"texturePath", texPath},
        {"color", {r->GetColor().x, r->GetColor().y, r->GetColor().z, r->GetColor().w}}
    };
}

/**
 * @brief Serializes the Spritesheet component into JSON.
 * @param j JSON object to write into.
 * @param s Pointer to the Spritesheet.
 */
void ComponentSerialization::SerializeSpritesheet(json& j, Spritesheet* s)
{
    j["Spritesheet"] = {
        {"rows", s->GetRows()},
        {"cols", s->GetCols()},
        {"startFrame", s->GetCurrentStartFrame()},
        {"endFrame", s->GetCols() * s->GetRows() - 1},
        {"fps", s->GetTimePerFrame()}
    };
}

/**
 * @brief Serializes the PhysicsComponent into JSON.
 * @param j JSON object to write into.
 * @param p Pointer to the PhysicsComponent.
 */
void ComponentSerialization::SerializePhysics(json& j, PhysicsComponent* p)
{
    j["PhysicsComponent"] = {
        {"mass", p->GetMass()},
        {"isStatic", p->IsStatic()},
        {"useGravity", p->useGravity},
        {"isGrounded", p->IsGrounded()},
        {"velocity", {p->velocityX, p->velocityY}}
    };
}

/**
 * @brief Serializes the ColliderComponent into JSON.
 * @param j JSON object to write into.
 * @param c Pointer to the ColliderComponent.
 */
void ComponentSerialization::SerializeCollider(json& j, ColliderComponent* c)
{
    j["ColliderComponent"] = {
        {"shape", EnumConverters::ToString(c->GetType())},
        {"layer", EnumConverters::ToString(c->GetLayer())}
    };
}

// ------------------- Deserialize -------------------
/**
 * @brief Deserializes TransformComponent data from JSON and attaches it to a GameObject.
 * @param j JSON object containing Transform data.
 * @param go Target GameObject to apply data to.
 */
void ComponentSerialization::DeserializeTransform(const json& j, GameObject* go)
{
    if (!j.contains("TransformComponent")) return;

    const auto& t = j["TransformComponent"];
    auto* tc = go->AddComponent<TransformComponent>();

    tc->SetPosition(t["pos"][0], t["pos"][1]);
    tc->SetScale(t["scale"][0], t["scale"][1]);
    tc->SetRotation(t["rotation"]);
}

/**
 * @brief Deserializes RenderComponent data from JSON and attaches it to a GameObject.
 *
 * Handles texture loading through the ResourceManager when possible.
 * Falls back to default colors if textures are unavailable.
 *
 * @param j JSON object containing Render data.
 * @param go Target GameObject to apply data to.
 */
void ComponentSerialization::DeserializeRender(const json& j, GameObject* go)
{
    if (!j.contains("RenderComponent")) return;

    const auto& r = j["RenderComponent"];
    auto* render = go->AddComponent<RenderComponent>();

    std::string shapeType = r.value("shapeType", "Square");
    render->SetShapeType(RenderComponentTools::ShapeEnumToString(shapeType));

    std::string texPath = r.value("texturePath", "");
    GLuint texID = 0;

    if (!texPath.empty()) {
        // try to get texture from resource manager
        auto& resourceMgr = ResourceManager::Instance();
        std::string goName = go->GetName();
        TextureResource* texResource = resourceMgr.GetTexture(goName);

        if (texResource && texResource->IsLoaded()) {
            // use existing texture from resource manager
            texID = texResource->GetTextureID();
            std::cout << "Serialization using texture from resource manager: " << goName << std::endl;
        }
        else {
            // not in resource manager, load it and register it for future use
            std::cout << "Serialization texture not in ResourceManager, loading and registering: "
                << goName << " -> " << texPath << std::endl;

            if (resourceMgr.LoadTexture(goName, texPath)) {
                texResource = resourceMgr.GetTexture(goName);
                if (texResource && texResource->IsLoaded()) {
                    texID = texResource->GetTextureID();
                }
            }
            else {
                // load directly
                std::cout << "Serialization warning: Failed to load via resource manager, directly loading" << std::endl;
                texID = GraphicsSystem::LoadTexture(texPath.c_str());
            }
        }

        if (texID != 0)
        {
            render->SetTextureID(texID);
            render->SetTexturePath(texPath);
            render->SetColor({ 1, 1, 1, 1 });

            glBindTexture(GL_TEXTURE_2D, texID);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glBindTexture(GL_TEXTURE_2D, 0);
        }
        else
        {
            std::cerr << "[DeserializeRender] Warning: Texture ID is 0 for " << goName
                << ", using fallback color instead.\n";
            render->SetTextureID(0);
            render->SetColor({ 1.0f, 0.5f, 0.5f, 1.0f }); // pink fallback
        }
    }
    else
    {
        if (r.contains("color"))
        {
            vec4 c(
                r["color"][0].get<float>(),
                r["color"][1].get<float>(),
                r["color"][2].get<float>(),
                r["color"][3].get<float>()
            );
            render->SetColor(c);
        }
        else
        {
            // default color if no texture and no color given
            render->SetColor({ 1.0f, 0.5f, 0.5f, 1.0f });
        }

        render->SetTextureID(0);
        render->SetTexturePath("");
    }

    // --- Final safety check ---
    if (render->GetTextureID() == 0 && texPath.empty())
    {
        std::cout << "[DeserializeRender] No texture for " << go->GetName()
            << ", rendering as pure color.\n";
    }
}

/**
 * @brief Deserializes Spritesheet data from JSON and attaches it to a GameObject.
 * @param j JSON object containing Spritesheet data.
 * @param go Target GameObject to apply data to.
 */
void ComponentSerialization::DeserializeSpritesheet(const json& j, GameObject* go)
{
    if (!j.contains("Spritesheet")) return;
    const auto& s = j["Spritesheet"];
    std::string texPath = j["RenderComponent"].value("texturePath", "");

    auto* spr = go->AddComponent<Spritesheet>(
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

/**
 * @brief Deserializes PhysicsComponent data from JSON and attaches it to a GameObject.
 * @param j JSON object containing Physics data.
 * @param go Target GameObject to apply data to.
 */
void ComponentSerialization::DeserializePhysics(const json& j, GameObject* go)
{
    if (!j.contains("PhysicsComponent")) return;

    const auto& p = j["PhysicsComponent"];
    auto* pc = go->AddComponent<PhysicsComponent>();

    pc->SetStatic(p.value("isStatic", false));
    pc->SetGravity(p.value("useGravity", true));
    pc->SetMass(p.value("mass", 1.0f));

    if (p.contains("velocity"))
        pc->SetVelocity(p["velocity"][0], p["velocity"][1]);

    pc->SetGrounded(p.value("isGrounded", false));
}

/**
 * @brief Deserializes ColliderComponent data from JSON and attaches it to a GameObject.
 * @param j JSON object containing Collider data.
 * @param go Target GameObject to apply data to.
 */
void ComponentSerialization::DeserializeCollider(const json& j, GameObject* go)
{
    if (!j.contains("ColliderComponent")) return;
    const auto& c = j["ColliderComponent"];

    ColliderComponent::Type type = EnumConverters::ToColliderType(c.value("shape", "Square"));
    CollisionLayer layer = EnumConverters::ToCollisionLayer(c.value("layer", "Default"));

    float width, height;
    if (j.contains("TransformComponent"))
    {
        const auto& t = j["TransformComponent"];
        if (t.contains("scale") && t["scale"].size() >= 2)
        {
            width = t["scale"][0].get<float>();
            height = t["scale"][1].get<float>();
        }
    }

    auto* collider = go->AddComponent<ColliderComponent>(type, width, height, layer);
}

// ------------------- Serialize / Deserialize All Componenents -------------------
/**
 * @brief Serializes all components attached to a GameObject.
 * @param go Pointer to the GameObject.
 * @param j JSON object to write serialized component data into.
 */
void ComponentSerialization::SerializeAllComponents(GameObject* go, json& j)
{
    if (const auto* t = go->GetComponent<TransformComponent>()) SerializeTransform(j, t);
    if (auto* r = go->GetComponent<RenderComponent>())    SerializeRender(j, r);
    if (auto* s = go->GetComponent<Spritesheet>())        SerializeSpritesheet(j, s);
    if (auto* p = go->GetComponent<PhysicsComponent>())   SerializePhysics(j, p);
    if (auto* c = go->GetComponent<ColliderComponent>())  SerializeCollider(j, c);
}

/**
 * @brief Deserializes all components defined in JSON and attaches them to a GameObject.
 * @param go Pointer to the GameObject to populate.
 * @param j JSON object containing serialized component data.
 */
void ComponentSerialization::DeserializeAllComponents(GameObject* go, const json& j)
{
    DeserializeTransform(j, go);
    DeserializeRender(j, go);
    DeserializeSpritesheet(j, go);
    DeserializePhysics(j, go);
    DeserializeCollider(j, go);
}