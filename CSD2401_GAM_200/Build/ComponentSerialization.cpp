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

// ------------------- Serialize -------------------
void ComponentSerialization::SerializeTransform(json& j, const TransformComponent* t)
{
    j["TransformComponent"] = {
        {"pos",   {t->posX, t->posY}},
        {"scale", {t->scaleX, t->scaleY}},
        {"rotation", t->GetRotation()}
    };
}

void ComponentSerialization::SerializeRender(json& j, const RenderComponent* r)
{
    j["RenderComponent"] = {
        {"shapeType", r->GetShapeTypeName()},
        {"texturePath", r->GetTexturePath()},
        {"color", {r->GetColor().x, r->GetColor().y, r->GetColor().z, r->GetColor().w}}
    };
}

void ComponentSerialization::SerializePhysics(json& j, const PhysicsComponent* p)
{
    j["PhysicsComponent"] = {
        {"mass", p->GetMass()},
        {"isStatic", p->IsStatic()},
        {"useGravity", p->useGravity},
        {"velocity", {p->velocityX, p->velocityY}}
    };
}

void ComponentSerialization::SerializeCollider(json& j, const ColliderComponent* c)
{
    j["ColliderComponent"] = {
        {"shape", EnumConverters::ToString(c->GetType())},
        {"layer", EnumConverters::ToString(c->GetLayer())}
    };
}

void ComponentSerialization::SerializeSpritesheet(json& j, const Spritesheet* s)
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

void ComponentSerialization::DeserializeRender(const json& j, GameObject* go)
{
    if (!j.contains("RenderComponent")) return;
    const auto& r = j["RenderComponent"];
    auto* render = go->AddComponent<RenderComponent>();
    render->SetShapeType(RenderComponentTools::ShapeEnumToString(r.value("shapeType", "Square")));
    render->SetColor({ r["color"][0], r["color"][1], r["color"][2], r["color"][3] });
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
    if (const auto* r = go->GetComponent<RenderComponent>())    SerializeRender(j, r);
    if (const auto* p = go->GetComponent<PhysicsComponent>())   SerializePhysics(j, p);
    if (const auto* c = go->GetComponent<ColliderComponent>())  SerializeCollider(j, c);
    if (const auto* s = go->GetComponent<Spritesheet>())        SerializeSpritesheet(j, s);
}

void ComponentSerialization::DeserializeAllComponents(GameObject* go, const json& j)
{
    DeserializeTransform(j, go);
    DeserializeRender(j, go);
    DeserializePhysics(j, go);
    DeserializeCollider(j, go);
    DeserializeSpritesheet(j, go);
}
