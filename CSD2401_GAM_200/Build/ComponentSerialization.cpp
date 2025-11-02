/* Start Header ************************************************************************/
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

/*!
* @brief Implements JSON serialization and deserialization logic for each
*    component type. This file isolates per-component data handling so
*    the core serializer remains clean and extendable.
*/

std::unordered_map<GLuint, std::string> s_TexturePathByID;
std::unordered_map<std::string, GLuint> s_IDByTexturePath;

// ------------------- Serialize -------------------
void ComponentSerialization::SerializeTransform(json& j, const TransformComponent* t)
{
    j["TransformComponent"] = {
        {"pos",   {t->posX, t->posY}},
        {"scale", {t->scaleX, t->scaleY}},
        {"rotation", t->GetRotation()}
    };
}

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

void ComponentSerialization::SerializePhysics(json& j, PhysicsComponent* p)
{
    j["PhysicsComponent"] = {
        {"mass", p->GetMass()},
        {"isStatic", p->IsStatic()},
        {"useGravity", p->useGravity},
        {"velocity", {p->velocityX, p->velocityY}}
    };
}

void ComponentSerialization::SerializeCollider(json& j, ColliderComponent* c)
{
    j["ColliderComponent"] = {
        {"shape", EnumConverters::ToString(c->GetType())},
        {"layer", EnumConverters::ToString(c->GetLayer())}
    };
}

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

// ------------------- Deserialize -------------------
void ComponentSerialization::DeserializeTransform(const json& j, GameObject* go)
{
    if (!j.contains("TransformComponent")) return;
    const auto& t = j["TransformComponent"];
    auto* tc = go->AddComponent<TransformComponent>();
    tc->SetPosition(t["pos"][0], t["pos"][1]);
    tc->SetScale(t["scale"][0], t["scale"][1]);
    tc->SetRotation(t["rotation"]);
}

void ComponentSerialization::DeserializeRender(const json& j, GameObject* go) {
    if (!j.contains("RenderComponent")) return;
    const auto& r = j["RenderComponent"];

    std::string shapeType = r.value("shapeType", "Square");
    std::string texPath = r.value("texturePath", "");



    auto* render = go->AddComponent<RenderComponent>();
    render->SetShapeType(RenderComponentTools::ShapeEnumToString(shapeType));

    if (!texPath.empty()) {
        // try to get texture from resource manager
        auto& resourceMgr = ResourceManager::Instance();
        std::string goName = go->GetName();
        TextureResource* texResource = resourceMgr.GetTexture(goName);
        GLuint texID = 0;

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

        render->SetTextureID(texID);
        render->SetTexturePath(texPath);  // store the path for future serialization
        render->SetColor({ 1,1,1,1 });

        glBindTexture(GL_TEXTURE_2D, texID);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glBindTexture(GL_TEXTURE_2D, 0);
    }
    else if (r.contains("color")) {
        vec4 c(r["color"][0], r["color"][1], r["color"][2], r["color"][3]);
        render->SetColor(c);
    }
    else {
        render->SetColor({ 1,1,1,1 });
    }
}

void ComponentSerialization::DeserializePhysics(const json& j, GameObject* go)
{
    if (!j.contains("PhysicsComponent")) return;
    const auto& p = j["PhysicsComponent"];
    auto* pc = go->AddComponent<PhysicsComponent>();
    pc->SetMass(p.value("mass", 1.0f));
    pc->SetStatic(p.value("isStatic", false));
    pc->SetGravity(p.value("useGravity", true));
    if (p.contains("velocity"))
        pc->SetVelocity(p["velocity"][0], p["velocity"][1]);
}

void ComponentSerialization::DeserializeCollider(const json& j, GameObject* go)
{
    if (!j.contains("ColliderComponent")) return;
    const auto& c = j["ColliderComponent"];
    ColliderComponent::Type type = EnumConverters::ToColliderType(c.value("shape", "Square"));
    CollisionLayer layer = EnumConverters::ToCollisionLayer(c.value("layer", "Default"));
    auto* collider = go->AddComponent<ColliderComponent>(type, 1.0f, 1.0f, layer);
}

void ComponentSerialization::DeserializeSpritesheet(const json& j, GameObject* go)
{
    if (!j.contains("Spritesheet")) return;
    const auto& s = j["Spritesheet"];
    auto* sheet = go->AddComponent<Spritesheet>(
        "", s.value("rows", 1), s.value("cols", 1));
    sheet->SetAnimation(s.value("startFrame", 0), s.value("endFrame", 0), s.value("fps", 1.0f));
}

// ------------------- Serialize / Deserialize All Componenents -------------------
void ComponentSerialization::SerializeAllComponents(GameObject* go, json& j)
{
    if (const auto* t = go->GetComponent<TransformComponent>()) SerializeTransform(j, t);
    if (auto* r = go->GetComponent<RenderComponent>())    SerializeRender(j, r);
    if (auto* p = go->GetComponent<PhysicsComponent>())   SerializePhysics(j, p);
    if (auto* c = go->GetComponent<ColliderComponent>())  SerializeCollider(j, c);
    if (auto* s = go->GetComponent<Spritesheet>())        SerializeSpritesheet(j, s);
}

void ComponentSerialization::DeserializeAllComponents(GameObject* go, const json& j)
{
    DeserializeTransform(j, go);
    DeserializeRender(j, go);
    DeserializePhysics(j, go);
    DeserializeCollider(j, go);
    DeserializeSpritesheet(j, go);
}
