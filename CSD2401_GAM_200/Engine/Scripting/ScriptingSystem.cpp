/* Start Header ************************************************************************/
/*!
\file       ScriptingSystem.cpp
\author     Jonathan Fam Qian Wei [100% - 825 lines]
\par        Fam.q@digipen.edu
\date       Novenber, 7, 2025

Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*/
/* End Header **************************************************************************/

#include "ScriptingSystem.h"
#include "gameObject.h"
#include "ScriptAPI.h"
#include "LayerManager.h"
#include <iostream>
#include <filesystem>
#include <mono/metadata/attrdefs.h>
#include "logic.h" // Include logic.h for EventSystem and EventType

// Internal call functions - these will be called from C#
// === Debug ===
static void Internal_Log(MonoString *message)
{
    char *str = mono_string_to_utf8(message);
    std::cout << "[C# Script] " << str << std::endl;
    mono_free(str);
}

// === GameObject Creation ===
static void Internal_CreateShape(MonoString *name, int type, float x, float y,
                                 float width, float height,
                                 float r, float g, float b, float a,
                                 int layer, bool hasPhysics, bool isStatic, bool hasGravity)
{
    char *objName = mono_string_to_utf8(name);
    vec4 color = {r, g, b, a};
    API::CreateShape(objName, (RenderComponent::ShapeType)type, x, y, width, height,
                     color, (CollisionLayer)layer, hasPhysics, isStatic, hasGravity);
    mono_free(objName);
}

static void Internal_CreateImage(MonoString *name, int type, float x, float y,
                                 float width, float height, MonoString *texturePath)
{
    char *objName = mono_string_to_utf8(name);
    char *texPath = mono_string_to_utf8(texturePath);
    API::CreateImage(objName, (RenderComponent::ShapeType)type, x, y, width, height, texPath);
    mono_free(objName);
    mono_free(texPath);
}

static void Internal_CreateAnimatedImage(MonoString *name, float x, float y,
                                         float width, float height, MonoString *texturePath,
                                         int rows, int columns, int startRow, int startCol, int endRow, int endCol, float fps,
                                         int layer, bool hasPhysics, bool isStatic, bool hasGravity)
{
    char *objName = mono_string_to_utf8(name);
    char *texPath = mono_string_to_utf8(texturePath);
    API::CreateAnimatedImage(objName, x, y, width, height, texPath,
                             rows, columns, startRow, startCol, endRow, endCol, fps,
                             (CollisionLayer)layer, hasPhysics, isStatic, hasGravity);
    mono_free(objName);
    mono_free(texPath);
}

static void Internal_DestroyGameObject(MonoString* name)
{
    char* objName = mono_string_to_utf8(name);
    API::DestroyGameObject(objName);
    mono_free(objName);
}

static void Internal_DestroyByID(int id)
{
    API::DestroyByID(static_cast<EntityID>(id));
}

static void Internal_DestroyByName(MonoString* name)
{
    char* objName = mono_string_to_utf8(name);
    API::DestroyByName(objName);
    mono_free(objName);
}

static MonoString* Internal_FindGameObjectByName(MonoString* name)
{
    char* objName = mono_string_to_utf8(name);
    std::string result = API::FindGameObjectByName(objName);
    mono_free(objName);
    return mono_string_new(mono_domain_get(), result.c_str());
}

static MonoString* Internal_FindGameObjectByID(int id)
{
    std::string result = API::FindGameObjectByID(id);
    return mono_string_new(mono_domain_get(), result.c_str());
}

// === Transform ===
static void Internal_SetPosition(MonoString *objectName, float x, float y)
{
    char *name = mono_string_to_utf8(objectName);
    API::SetPos(name, x, y);
    mono_free(name);
}

static void Internal_SetRotation(MonoString *objectName, float rotation)
{
    char *name = mono_string_to_utf8(objectName);
    API::SetRot(name, rotation);
    mono_free(name);
}

static void Internal_AddRotation(MonoString *objectName, float rotation)
{
    char *name = mono_string_to_utf8(objectName);
    API::AddRot(name, rotation);
    mono_free(name);
}

static void Internal_SetScale_Single(MonoString *objectName, float scale)
{
    char *name = mono_string_to_utf8(objectName);
    API::SetScale(name, scale);
    mono_free(name);
}

static void Internal_SetScale_XY(MonoString* objectName, float x, float y)
{
    char* name = mono_string_to_utf8(objectName);
    API::SetScale(name, x, y);
    mono_free(name);
}

static float Internal_GetRotation(MonoString* objectName)
{
    char* name = mono_string_to_utf8(objectName);
    float result = API::GetRotation(name);
    mono_free(name);
    return result;
}

static void Internal_GetPosition(MonoString* objectName, float* x, float* y)
{
    char* name = mono_string_to_utf8(objectName);
    API::GetPosition(name, x, y);
    mono_free(name);
}

static void Internal_GetScale(MonoString* objectName, float* x, float* y)
{
    char* name = mono_string_to_utf8(objectName);
    API::GetScale(name, x, y);
    mono_free(name);
}

// === Graphics ===
static void Internal_SetShapeType(MonoString *objectName, int shapeType)
{
    char *name = mono_string_to_utf8(objectName);
    API::SetShapeType(name, (RenderComponent::ShapeType)shapeType);
    mono_free(name);
}

static void Internal_SetTexture(MonoString *objectName, MonoString *filename)
{
    char *name = mono_string_to_utf8(objectName);
    char *file = mono_string_to_utf8(filename);
    API::SetTexture(name, file);
    mono_free(name);
    mono_free(file);
}

static void Internal_SetColor(MonoString *objectName, float r, float g, float b, float a)
{
    char *name = mono_string_to_utf8(objectName);
    vec4 color = {r, g, b, a};
    API::setColor(name, color);
    mono_free(name);
}

static void Internal_GetColor(MonoString* objectName, float* r, float* g, float* b, float* a)
{
    if (!objectName)
    {
        std::cerr << "[ScriptSystem] ERROR: GetColor called with null objectName\n";
        *r = 0.0f; *g = 0.0f; *b = 0.0f; *a = 1.0f;
        return;
    }

    char* name = mono_string_to_utf8(objectName);
    if (!name)
    {
        std::cerr << "[ScriptSystem] ERROR: mono_string_to_utf8 returned null\n";
        *r = 0.0f; *g = 0.0f; *b = 0.0f; *a = 1.0f;
        return;
    }

    vec4 result = API::GetColor(name);
    *r = result.x;
    *g = result.y;
    *b = result.z;
    *a = result.w;
    mono_free(name);
}

static void Internal_SetAlpha(MonoString *objectName, float alpha)
{
    char *name = mono_string_to_utf8(objectName);
    API::SetAlpha(name, alpha);
    mono_free(name);
}

static void Internal_SetRenderLayer(MonoString *objectName, int layer)
{
    char *name = mono_string_to_utf8(objectName);
    API::SetRenderLayer(name, layer);
    mono_free(name);
}

static int Internal_GetRenderLayer(MonoString *objectName)
{
    char *name = mono_string_to_utf8(objectName);
    int result = API::GetRenderLayer(name);
    mono_free(name);
    return result;
}

static void Internal_SetLayerVisible(int layer, bool visible)
{
    API::SetLayerVisible(layer, visible);
}

static bool Internal_IsLayerVisible(int layer)
{
    return API::IsLayerVisible(layer);
}

// === Physics ===
static void Internal_SetVelocity(MonoString *objectName, float vx, float vy)
{
    char *name = mono_string_to_utf8(objectName);
    API::SetVelocity(name, vx, vy);
    mono_free(name);
}

static void Internal_SetJumpValue(MonoString *objectName, float jumpForce)
{
    char *name = mono_string_to_utf8(objectName);
    API::SetJumpVal(name, jumpForce);
    mono_free(name);
}

static void Internal_SetGravity(MonoString *objectName, bool enabled)
{
    char *name = mono_string_to_utf8(objectName);
    API::SetGravBool(name, enabled);
    mono_free(name);
}


static void Internal_GetVelocity(MonoString* objectName, float* x, float* y)
{
    if (!objectName)
    {
        std::cerr << "[ScriptSystem] ERROR: GetVelocity called with null objectName\n";
        *x = 0.0f;
        *y = 0.0f;
        return;
    }

    char* name = mono_string_to_utf8(objectName);
    if (!name)
    {
        std::cerr << "[ScriptSystem] ERROR: mono_string_to_utf8 returned null\n";
        *x = 0.0f;
        *y = 0.0f;
        return;
    }

    vec2 result = API::GetVelocity(name);
    *x = result.x;
    *y = result.y;
    mono_free(name);
}

static void Internal_SetMass(MonoString* objectName, float mass)
{
    char* name = mono_string_to_utf8(objectName);
    API::SetMass(name, mass);
    mono_free(name);
}

static void Internal_SetFriction(MonoString* objectName, float friction)
{
    char* name = mono_string_to_utf8(objectName);
    API::SetFriction(name, friction);
    mono_free(name);
}

// === Collision ===
static void Internal_SetStatic(MonoString *objectName, bool isStatic)
{
    char *name = mono_string_to_utf8(objectName);
    API::SetStatic(name, isStatic);
    mono_free(name);
}

// === Collision ===
static void Internal_SetCollisionShape(MonoString *objectName, int shape)
{
    char *name = mono_string_to_utf8(objectName);
    API::SetColisionShape(name, (ColliderComponent::Type)shape);
    mono_free(name);
}

static void Internal_SetCollisionLayer(MonoString* objectName, int layer)
{
    char* name = mono_string_to_utf8(objectName);
    API::SetCollisionLayer(name, (CollisionLayer)layer);
    mono_free(name);
}
static int Internal_GetCollisionLayer(MonoString* objectName)
{
    char* name = mono_string_to_utf8(objectName);
    int result = API::GetCollisionLayer(name);
    mono_free(name);
    return result;
}

static int Internal_GetCollisionShape(MonoString* objectName)
{
    char* name = mono_string_to_utf8(objectName);
    int result = API::GetCollisionShape(name);
    mono_free(name);
    return result;
}

static bool Internal_CheckColliderLayer(int layer1, int layer2)
{
    return API::CheckColliderLayer(layer1, layer2);
}

static bool Internal_IsCollidingWithLayer(MonoString* objectName, int targetLayer)
{
    char* name = mono_string_to_utf8(objectName);
    bool result = API::IsCollidingWithLayer(name, targetLayer);
    mono_free(name);
    return result;
}

// === Components ===
static bool Internal_AttachScript(MonoString* objectName, MonoString* scriptClassName)
{
    char* objName = mono_string_to_utf8(objectName);
    char* scriptName = mono_string_to_utf8(scriptClassName);
    bool result = API::AttachScript(objName, scriptName);
    mono_free(objName);
    mono_free(scriptName);
    return result;
}

static MonoString* Internal_GetCollidingObjectNameOnLayer(MonoString* objectName, int targetLayer)
{
    char* obj = mono_string_to_utf8(objectName);
    std::string result = API::GetCollidingObjectNameOnLayer(obj, targetLayer);
    mono_free(obj);
    return mono_string_new(mono_domain_get(), result.c_str());
}

static int Internal_GetCollidingObjectIDOnLayer(MonoString* objectName, int targetLayer)
{
    char* obj = mono_string_to_utf8(objectName);
    int result = API::GetCollidingObjectIDOnLayer(obj, targetLayer);
    mono_free(obj);
    return result;
}

// === Script field getters ===
static float Internal_GetFloatField(MonoString* objectName, MonoString* scriptClass, MonoString* fieldName)
{
    char* obj = mono_string_to_utf8(objectName);
    char* cls = mono_string_to_utf8(scriptClass);
    char* fld = mono_string_to_utf8(fieldName);
    float result = API::GetFloatField(obj, cls, fld);
    mono_free(obj); mono_free(cls); mono_free(fld);
    return result;
}

static int Internal_GetIntField(MonoString* objectName, MonoString* scriptClass, MonoString* fieldName)
{
    char* obj = mono_string_to_utf8(objectName);
    char* cls = mono_string_to_utf8(scriptClass);
    char* fld = mono_string_to_utf8(fieldName);
    int result = API::GetIntField(obj, cls, fld);
    mono_free(obj); mono_free(cls); mono_free(fld);
    return result;
}

static bool Internal_GetBoolField(MonoString* objectName, MonoString* scriptClass, MonoString* fieldName)
{
    char* obj = mono_string_to_utf8(objectName);
    char* cls = mono_string_to_utf8(scriptClass);
    char* fld = mono_string_to_utf8(fieldName);
    bool result = API::GetBoolField(obj, cls, fld);
    mono_free(obj); mono_free(cls); mono_free(fld);
    return result;
}

static MonoString* Internal_GetStringField(MonoString* objectName, MonoString* scriptClass, MonoString* fieldName)
{
    char* obj = mono_string_to_utf8(objectName);
    char* cls = mono_string_to_utf8(scriptClass);
    char* fld = mono_string_to_utf8(fieldName);
    std::string result = API::GetStringField(obj, cls, fld);
    mono_free(obj); mono_free(cls); mono_free(fld);
    return mono_string_new(mono_domain_get(), result.c_str());
}

// === Script field setters ===
static void Internal_SetFloatField(MonoString* objectName, MonoString* scriptClass, MonoString* fieldName, float value)
{
    char* obj = mono_string_to_utf8(objectName);
    char* cls = mono_string_to_utf8(scriptClass);
    char* fld = mono_string_to_utf8(fieldName);
    API::SetFloatField(obj, cls, fld, value);
    mono_free(obj); mono_free(cls); mono_free(fld);
}

static void Internal_SetIntField(MonoString* objectName, MonoString* scriptClass, MonoString* fieldName, int value)
{
    char* obj = mono_string_to_utf8(objectName);
    char* cls = mono_string_to_utf8(scriptClass);
    char* fld = mono_string_to_utf8(fieldName);
    API::SetIntField(obj, cls, fld, value);
    mono_free(obj); mono_free(cls); mono_free(fld);
}

static void Internal_SetBoolField(MonoString* objectName, MonoString* scriptClass, MonoString* fieldName, bool value)
{
    char* obj = mono_string_to_utf8(objectName);
    char* cls = mono_string_to_utf8(scriptClass);
    char* fld = mono_string_to_utf8(fieldName);
    API::SetBoolField(obj, cls, fld, value);
    mono_free(obj); mono_free(cls); mono_free(fld);
}

static void Internal_SetStringField(MonoString* objectName, MonoString* scriptClass, MonoString* fieldName, MonoString* value)
{
    char* obj = mono_string_to_utf8(objectName);
    char* cls = mono_string_to_utf8(scriptClass);
    char* fld = mono_string_to_utf8(fieldName);
    char* val = mono_string_to_utf8(value);
    API::SetStringField(obj, cls, fld, val);
    mono_free(obj); mono_free(cls); mono_free(fld); mono_free(val);
}

static void Internal_SetComponentActive(MonoString* objectName, MonoString* componentName, bool active)
{
    char* objName = mono_string_to_utf8(objectName);
    char* compName = mono_string_to_utf8(componentName);
    API::SetComponentActive(objName, compName, active);
    mono_free(objName);
    mono_free(compName);
}

static void Internal_RemoveComponent(MonoString* objectName, MonoString* componentName)
{
    char* objName = mono_string_to_utf8(objectName);
    char* compName = mono_string_to_utf8(componentName);
    API::RemoveComponent(objName, compName);
    mono_free(objName);
    mono_free(compName);
}

static void Internal_AddTransformComponent(MonoString *objectName)
{
    char *name = mono_string_to_utf8(objectName);
    API::AddTransformComponent(name);
    mono_free(name);
}

static void Internal_AddRenderComponent(MonoString *objectName)
{
    char *name = mono_string_to_utf8(objectName);
    API::AddRenderComponent(name);
    mono_free(name);
}

static void Internal_AddCollisionComponent(MonoString *objectName)
{
    char *name = mono_string_to_utf8(objectName);
    API::AddCollisionComponent(name);
    mono_free(name);
}

static void Internal_AddPhysicsComponent(MonoString *objectName)
{
    char *name = mono_string_to_utf8(objectName);
    API::AddPhysicComponent(name);
    mono_free(name);
}

static void Internal_AddTextComponent(MonoString* objectName, MonoString* fontPath, unsigned int fontSize, MonoString* text, float r, float g, float b, float a, bool screenSpace)
{
    char* objName = mono_string_to_utf8(objectName);
    char* font = mono_string_to_utf8(fontPath);
    char* txt = mono_string_to_utf8(text);
    vec4 color = {r, g, b, a};
    API::AddTextComponent(objName, font, fontSize, txt, color, screenSpace);
    mono_free(objName);
    mono_free(font);
    mono_free(txt);
}

// === Text ===
static void Internal_SetText(MonoString* objectName, MonoString* text)
{
    char* name = mono_string_to_utf8(objectName);
    char* txt = mono_string_to_utf8(text);
    API::SetText(name, txt);
    mono_free(name);
    mono_free(txt);
}

static MonoString* Internal_GetText(MonoString* objectName)
{
    char* name = mono_string_to_utf8(objectName);
    std::string result = API::GetText(name);
    mono_free(name);
    return mono_string_new(mono_domain_get(), result.c_str());
}

static void Internal_SetTextColor(MonoString* objectName, float r, float g, float b, float a)
{
    char* name = mono_string_to_utf8(objectName);
    vec4 color = {r, g, b, a};
    API::SetTextColor(name, color);
    mono_free(name);
}

static void Internal_GetTextColor(MonoString* objectName, float* r, float* g, float* b, float* a)
{
    if (!objectName)
    {
        std::cerr << "[ScriptSystem] ERROR: GetTextColor called with null objectName\n";
        *r = 1.0f; *g = 1.0f; *b = 1.0f; *a = 1.0f;
        return;
    }

    char* name = mono_string_to_utf8(objectName);
    if (!name)
    {
        std::cerr << "[ScriptSystem] ERROR: mono_string_to_utf8 returned null\n";
        *r = 1.0f; *g = 1.0f; *b = 1.0f; *a = 1.0f;
        return;
    }

    vec4 result = API::GetTextColor(name);
    *r = result.x;
    *g = result.y;
    *b = result.z;
    *a = result.w;
    mono_free(name);
}

static void Internal_SetTextScale(MonoString* objectName, float scale)
{
    char* name = mono_string_to_utf8(objectName);
    API::SetTextScale(name, scale);
    mono_free(name);
}

static float Internal_GetTextScale(MonoString* objectName)
{
    char* name = mono_string_to_utf8(objectName);
    float result = API::GetTextScale(name);
    mono_free(name);
    return result;
}

static void Internal_SetTextScreenSpace(MonoString* objectName, bool screenSpace)
{
    char* name = mono_string_to_utf8(objectName);
    API::SetTextScreenSpace(name, screenSpace);
    mono_free(name);
}

static bool Internal_GetTextScreenSpace(MonoString* objectName)
{
    char* name = mono_string_to_utf8(objectName);
    bool result = API::GetTextScreenSpace(name);
    mono_free(name);
    return result;
}

static void Internal_AddSound(MonoString* name, MonoString* filepath, bool loop, float volume, float pitch,MonoString* category) {
    char* cName = mono_string_to_utf8(name);
    char* cPath = mono_string_to_utf8(filepath);
    char* cCat = mono_string_to_utf8(category);
    API::AddSound(cName, cPath, loop, volume, pitch, cCat);
    mono_free(cName);
    mono_free(cPath);
    mono_free(cCat);
}

static void Internal_PlaySound(MonoString* soundName, MonoString* channel) {
    char* cName = mono_string_to_utf8(soundName);
    char* cChannel = mono_string_to_utf8(channel);
    API::PlaySound(cName, cChannel);
    mono_free(cName);
    mono_free(cChannel);
}

static void Internal_StopSound(MonoString* channelName) {
    char* cChannelName = mono_string_to_utf8(channelName);
    API::StopSound(cChannelName);
    mono_free(cChannelName);
}

static void Internal_StopAllSounds() {
    API::StopAllSound();
    
}

static void Internal_RemoveSound(MonoString* soundName) {
    char* cName = mono_string_to_utf8(soundName);
    API::RemoveSound(cName);
    mono_free(cName);

}

static void Internal_TriggerEvent(MonoString* eventName) {
    char* cName = mono_string_to_utf8(eventName);
    API::TriggerEvent(cName);
    mono_free(cName);
}

static void Internal_AddEventSound(MonoString* eventName, MonoString* soundName) {
    char* cEvent = mono_string_to_utf8(eventName);
    char* cSound = mono_string_to_utf8(soundName);
    API::AddEventSound(cEvent, cSound);
    mono_free(cEvent);
    mono_free(cSound);
}

static void Internal_SetVolume(MonoString* soundName, float volume) {
    char* cName = mono_string_to_utf8(soundName);
    API::SetVolume(cName, volume);
    mono_free(cName);
}

static void Internal_SetCategoryVolume(MonoString* catName, float volume) {
    char* cName = mono_string_to_utf8(catName);
    API::SetCategoryVolume(cName, volume);
    mono_free(cName);
}

static void Internal_SetPitch(MonoString* soundName, float pitch) {
    char* cName = mono_string_to_utf8(soundName);
    API::SetPitch(cName, pitch);
    mono_free(cName);
}

static void Internal_PauseSound(MonoString* channelName, bool pause) {
    char* cName = mono_string_to_utf8(channelName);
    API::PauseSound(cName, pause);
    mono_free(cName);
}

static void Internal_PauseAllSounds(bool pause) {
    API::PauseAllSounds(pause);
}

static void Internal_ToggleMasterMute() {
    API::ToggleMasterMute();
}

static void Internal_ToggleCategoryMute(MonoString* category) {
    char* cName = mono_string_to_utf8(category);
    API::ToggleCategoryMute(cName);
    mono_free(cName);
}

static bool Internal_IsMasterMuted() {
    return API::IsMasterMuted();
}

static bool Internal_IsCategoryMuted(MonoString* category) {
    char* cName = mono_string_to_utf8(category);
    bool result = API::IsCategoryMuted(cName);
    mono_free(cName);
    return result;
}

static void Internal_FadeOutCategory(MonoString* category, float duration) {
    char* cName = mono_string_to_utf8(category);
    API::FadeOutCategory(cName, duration);
    mono_free(cName);
}

static void Internal_FadeInCategory(MonoString* category, float duration, float targetVolume) {
    char* cName = mono_string_to_utf8(category);
    API::FadeInCategory(cName, duration, targetVolume);
    mono_free(cName);
}

static void Internal_FadeOutAll(float duration) {
    API::FadeOutAll(duration);
}

// === 3D Audio / Spatial Sound ===
static void Internal_Set3DListenerPosition(float x, float y) {
    API::Set3DListenerPosition(x, y);
}

static void Internal_Set3DSoundPosition(MonoString* channelName, float x, float y) {
    char* cChannelName = mono_string_to_utf8(channelName);
    API::Set3DSoundPosition(cChannelName, x, y);
    mono_free(cChannelName);
}

static void Internal_Load3DSound(MonoString* name, MonoString* filepath, bool loop,
                                 float minDistance, float maxDistance, MonoString* category,
                                 float volume, float pitch) {
    char* cName = mono_string_to_utf8(name);
    char* cFilepath = mono_string_to_utf8(filepath);
    char* cCategory = mono_string_to_utf8(category);

    API::Load3DSound(cName, cFilepath, loop, minDistance, maxDistance, cCategory, volume, pitch);

    mono_free(cName);
    mono_free(cFilepath);
    mono_free(cCategory);
}

// === Camera ===
static void Internal_SetCameraPosition(float x, float y) {
    vec2 pos = { x, y };
    API::setCameraPos(pos);
}

static void Internal_SetCameraZoom(float zoom) {
    API::setCameraZoom(zoom);
}

static void Internal_GetCameraPosition(float* outX, float* outY)
{
    vec2 pos = API::GetCameraPosition();
    if (outX) *outX = pos.x;
    if (outY) *outY = pos.y;
}

static float Internal_GetCameraZoom()
{
    return API::GetCameraZoom();
}

static int Internal_GetCameraMode()
{
    return API::GetCameraMode();
}

static void Internal_SetCameraMode(int mode) {
    API::setCameraMode((CameraComponent::CameraMode)mode);
}

static void Internal_CameraFlash(float duration, float r, float g, float b, float a, bool fadeout, float camShakeStrength)
{
    API::CameraFlash(duration, {r, g, b, a}, fadeout, camShakeStrength);
}

static void Internal_SetCameraFollowOffset(float x, float y)
{
    API::SetCameraFollowOffset(x, y);
}

static void Internal_GetCameraFollowOffset(float* outX, float* outY)
{
    vec2 offset = API::GetCameraFollowOffset();
    if (outX) *outX = offset.x;
    if (outY) *outY = offset.y;
}

static int Internal_CountAllGameObjects()
{
    return GameObjectFactory::Instance().CountAllGameObjects();
}

static int Internal_GetHighestLocalID()
{
    return static_cast<int>(GameObjectFactory::Instance().GetHighestLocalID());
}

static MonoArray* Internal_GetAllGameObjectIDs()
{
    std::vector<EntityID> ids = GameObjectFactory::Instance().GetAllGameObjectIDs();
    MonoArray* arr = mono_array_new(mono_domain_get(), mono_get_int32_class(), ids.size());
    for (size_t i = 0; i < ids.size(); ++i)
        mono_array_set(arr, int, i, static_cast<int>(ids[i]));
    return arr;
}

static float Internal_GetRemainingTime()
{
    return API::GetRemainingTime();
}

static float Internal_GetLevelLength()
{
    return API::GetLevelLength();
}

static void Internal_SetLevelTimerLength(float length)
{
    API::SetLevelTimerLength(length);
}

static void Internal_AddTime(float amount)
{
    API::AddTime(amount);
}

static bool Internal_Raycast(MonoString* selfName, float x, float y, float dx, float dy, float distance, MonoString** hitObjectName, float* hitDistance)
{
    return API::Raycast(x, y, dx, dy, distance, selfName, hitObjectName, hitDistance);
}

// === Collision Queries ===
static bool Internal_OnTriggerEnter(MonoString* obj1, MonoString* obj2) {
    char* n1 = mono_string_to_utf8(obj1);
    char* n2 = mono_string_to_utf8(obj2);
    bool result = API::OnTriggerEnter(n1, n2);
    mono_free(n1);
    mono_free(n2);
    return result;
}

static bool Internal_OnTriggerExit(MonoString* obj1, MonoString* obj2) {
    char* n1 = mono_string_to_utf8(obj1);
    char* n2 = mono_string_to_utf8(obj2);
    bool result = API::OnTriggerExit(n1, n2);
    mono_free(n1);
    mono_free(n2);
    return result;
}

static bool Internal_OnTriggerStay(MonoString* obj1, MonoString* obj2) {
    char* n1 = mono_string_to_utf8(obj1);
    char* n2 = mono_string_to_utf8(obj2);
    bool result = API::OnTriggerStay(n1, n2);
    mono_free(n1);
    mono_free(n2);
    return result;
}

static void Internal_PlayAnimation(MonoString* objectName, MonoString* animationName, bool isLooping)
{
    char* objName = mono_string_to_utf8(objectName);
    char* animName = mono_string_to_utf8(animationName);
    API::PlayAnimation(objName, animName, isLooping);
    mono_free(objName);
    mono_free(animName);
}

static void Internal_SetAnimationLooping(MonoString* objectName, bool isLooping)
{
    char* objName = mono_string_to_utf8(objectName);
    API::SetAnimationLooping(objName, isLooping);
    mono_free(objName);
}

static bool Internal_IsAnimationLooping(MonoString* objectName)
{
    char* objName = mono_string_to_utf8(objectName);
    bool result = API::IsAnimationLooping(objName);
    mono_free(objName);
    return result;
}

static void Internal_AddAnimationToQueue(MonoString* objectName, MonoString* animationName)
{
    char* objName = mono_string_to_utf8(objectName);
    char* animName = mono_string_to_utf8(animationName);
    API::AddAnimationToQueue(objName, animName);
    mono_free(objName);
    mono_free(animName);
}

static void Internal_RegisterAnimation(MonoString* objectName, MonoString* animationName, MonoString* texturePath, int rows, int cols, int startRow, int startCol, int endRow, int endCol, float fps, bool pingPong, bool flipRows)
{
    char* objName = mono_string_to_utf8(objectName);
    char* animName = mono_string_to_utf8(animationName);
    char* texPath = mono_string_to_utf8(texturePath);
    API::RegisterAnimation(objName, animName, texPath, rows, cols, startRow, startCol, endRow, endCol, fps, pingPong, flipRows);
    mono_free(objName);
    mono_free(animName);
    mono_free(texPath);
}

static void Internal_SetAnimation(MonoString* objectName, int startFrame, int endFrame, float fps)
{
    char* objName = mono_string_to_utf8(objectName);
    API::SetAnimation(objName, startFrame, endFrame, fps);
    mono_free(objName);
}

static MonoString* Internal_GetCurrentAnimationName(MonoString* objectName)
{
    char* objName = mono_string_to_utf8(objectName);
    std::string animName = API::GetCurrentAnimationName(objName);
    mono_free(objName);
    return mono_string_new(mono_domain_get(), animName.c_str());
}

static void Internal_LoadNewSpritesheet(MonoString* objectName, MonoString* texturePath, int rows, int cols)
{
    char* objName = mono_string_to_utf8(objectName);
    char* texPath = mono_string_to_utf8(texturePath);
    API::LoadNewSpritesheet(objName, texPath, rows, cols);
    mono_free(objName);
    mono_free(texPath);
}

static void Internal_SetCurrentFrame(MonoString* objectName, int row, int col)
{
    char* objName = mono_string_to_utf8(objectName);
    API::SetCurrentFrame(objName, row, col);
    mono_free(objName);
}

static int Internal_GetRows(MonoString* objectName)
{
    char* objName = mono_string_to_utf8(objectName);
    int result = API::GetRows(objName);
    mono_free(objName);
    return result;
}

static int Internal_GetCols(MonoString* objectName)
{
    char* objName = mono_string_to_utf8(objectName);
    int result = API::GetCols(objName);
    mono_free(objName);
    return result;
}

static int Internal_GetCurrentRow(MonoString* objectName)
{
    char* objName = mono_string_to_utf8(objectName);
    int result = API::GetCurrentRow(objName);
    mono_free(objName);
    return result;
}

static int Internal_GetCurrentCol(MonoString* objectName)
{
    char* objName = mono_string_to_utf8(objectName);
    int result = API::GetCurrentCol(objName);
    mono_free(objName);
    return result;
}

// === EntityID-based getters/setters ===

// Transform
static void Internal_GetPositionByID(int id, float* x, float* y)
{
    API::GetPositionByID(static_cast<EntityID>(id), x, y);
}
static void Internal_GetScaleByID(int id, float* x, float* y)
{
    API::GetScaleByID(static_cast<EntityID>(id), x, y);
}
static float Internal_GetRotationByID(int id)
{
    return API::GetRotationByID(static_cast<EntityID>(id));
}
static void Internal_SetPosByID(int id, float x, float y)
{
    API::SetPosByID(static_cast<EntityID>(id), x, y);
}
static void Internal_SetScaleByID(int id, float x, float y)
{
    API::SetScaleByID(static_cast<EntityID>(id), x, y);
}
static void Internal_SetRotByID(int id, float rotation)
{
    API::SetRotByID(static_cast<EntityID>(id), rotation);
}

// Graphics
static void Internal_GetColorByID(int id, float* r, float* g, float* b, float* a)
{
    API::GetColorByID(static_cast<EntityID>(id), r, g, b, a);
}
static int Internal_GetRenderLayerByID(int id)
{
    return API::GetRenderLayerByID(static_cast<EntityID>(id));
}
static void Internal_SetColorByID(int id, float r, float g, float b, float a)
{
    API::SetColorByID(static_cast<EntityID>(id), {r, g, b, a});
}
static void Internal_SetAlphaByID(int id, float alpha)
{
    API::SetAlphaByID(static_cast<EntityID>(id), alpha);
}
static void Internal_SetRenderLayerByID(int id, int layer)
{
    API::SetRenderLayerByID(static_cast<EntityID>(id), layer);
}
static void Internal_SetTextureByID(int id, MonoString* filename)
{
    char* file = mono_string_to_utf8(filename);
    API::SetTextureByID(static_cast<EntityID>(id), file);
    mono_free(file);
}

// Physics
static void Internal_GetVelocityByID(int id, float* vx, float* vy)
{
    API::GetVelocityByID(static_cast<EntityID>(id), vx, vy);
}
static void Internal_SetVelocityByID(int id, float x, float y)
{
    API::SetVelocityByID(static_cast<EntityID>(id), x, y);
}

// Text
static MonoString* Internal_GetTextByID(int id)
{
    std::string result = API::GetTextByID(static_cast<EntityID>(id));
    return mono_string_new(mono_domain_get(), result.c_str());
}
static void Internal_SetTextByID(int id, MonoString* text)
{
    char* str = mono_string_to_utf8(text);
    API::SetTextByID(static_cast<EntityID>(id), str);
    mono_free(str);
}

// Animator
static MonoString* Internal_GetCurrentAnimationNameByID(int id)
{
    std::string result = API::GetCurrentAnimationNameByID(static_cast<EntityID>(id));
    return mono_string_new(mono_domain_get(), result.c_str());
}
static int Internal_GetRowsByID(int id)       { return API::GetRowsByID(static_cast<EntityID>(id)); }
static int Internal_GetColsByID(int id)       { return API::GetColsByID(static_cast<EntityID>(id)); }
static int Internal_GetCurrentRowByID(int id) { return API::GetCurrentRowByID(static_cast<EntityID>(id)); }
static int Internal_GetCurrentColByID(int id) { return API::GetCurrentColByID(static_cast<EntityID>(id)); }
static void Internal_PlayAnimationByID(int id, MonoString* animationName, bool isLooping)
{
    char* animName = mono_string_to_utf8(animationName);
    API::PlayAnimationByID(static_cast<EntityID>(id), animName, isLooping);
    mono_free(animName);
}

// === Misc ===
static int Internal_RandomRange(int min, int max) {
    return API::RandomNumberGen(max, min); 
}
static float Internal_GetDelta_Time() {
    return static_cast<float>(API::GetDelta_Time());
}
static float Internal_GetFixed_Delta_Time()
{
    return static_cast<float>(API::GetFixedDelta_time());
}
static float Internal_GetFramesPerSecond() {
    return static_cast<float>(API::GetFramePerSecond());
}


// === Input ===
static bool Internal_IsKeyPressed(int key)
{
    return API::IsKeyPressed(key);
}
static bool Internal_IsKeyReleased(int key)
{
    return API::IsKeyReleased(key);
}
static bool Internal_IsKeyHeld(int key)
{
    return API::IsKeyHeld(key);
}

static bool Internal_IsMouseButtonPressed(int button)
{
    return API::IsMouseButtonPressed(button);
}
static bool Internal_IsMouseButtonReleased(int button)
{
    return API::IsMouseButtonReleased(button);
}
static bool Internal_IsMouseButtonHeld(int button)
{
    return API::IsMouseButtonHeld(button);
}

static float Internal_GetMouseX()
{
    return API::GetMouseX();
}

static float Internal_GetMouseY()
{
    return API::GetMouseY();
}

static float Internal_GetMouseWorldX()
{
    return API::GetMouseWorldX();
}

static float Internal_GetMouseWorldY()
{
    return API::GetMouseWorldY();
}

static bool Internal_IsGamepadButtonPressed(int button)  { return API::IsGamepadButtonPressed(button); }
static bool Internal_IsGamepadButtonReleased(int button) { return API::IsGamepadButtonReleased(button); }
static bool Internal_IsGamepadButtonHeld(int button)     { return API::IsGamepadButtonHeld(button); }
static float Internal_GetGamepadAxis(int axis)           { return API::GetGamepadAxis(axis); }
static bool Internal_IsGamepadConnected()                { return API::IsGamepadConnected(); }

// === Pickup System ===
static void Internal_CollectCoin()
{
    if (CORE && CORE->GetGameCallbacks())
        CORE->GetGameCallbacks()->CollectCoin();
}

static void Internal_CollectShell()
{
    if (CORE && CORE->GetGameCallbacks())
        CORE->GetGameCallbacks()->CollectShell();
}

static int Internal_GetCurrentCoins()
{
    if (CORE && CORE->GetGameCallbacks())
        return CORE->GetGameCallbacks()->GetCurrentCoins();
    return 0;
}

static int Internal_GetCurrentShells()
{
    if (CORE && CORE->GetGameCallbacks())
        return CORE->GetGameCallbacks()->GetCurrentShells();
    return 0;
}

static int Internal_GetTotalCoins()
{
    if (CORE && CORE->GetGameCallbacks())
        return CORE->GetGameCallbacks()->GetTotalCoins();
    return 0;
}

static int Internal_GetTotalShells()
{
    if (CORE && CORE->GetGameCallbacks())
        return CORE->GetGameCallbacks()->GetTotalShells();
    return 0;
}

// === WinLose System ===
static void Internal_TriggerLoseZone()
{
    if (CORE && CORE->GetGameCallbacks())
        CORE->GetGameCallbacks()->TriggerLose();
}

static bool Internal_IsWinScreenActive()
{
    return CORE && CORE->GetGameCallbacks() && CORE->GetGameCallbacks()->IsWinScreenActive();
}

static bool Internal_IsLoseScreenActive()
{
    return CORE && CORE->GetGameCallbacks() && CORE->GetGameCallbacks()->IsLoseScreenActive();
}

// === Particles ===
static void Internal_AddParticleEmitterComponent(MonoString* objectName, int maxParticles)
{
    char* name = mono_string_to_utf8(objectName);
    API::AddParticleEmitterComponent(name, maxParticles);
    mono_free(name);
}

static void Internal_Particles_StartEmission(MonoString* objectName)
{
    char* name = mono_string_to_utf8(objectName);
    API::Particles_StartEmission(name);
    mono_free(name);
}

static void Internal_Particles_StopEmission(MonoString* objectName)
{
    char* name = mono_string_to_utf8(objectName);
    API::Particles_StopEmission(name);
    mono_free(name);
}

static void Internal_Particles_Burst(MonoString* objectName, int count)
{
    char* name = mono_string_to_utf8(objectName);
    API::Particles_Burst(name, count);
    mono_free(name);
}

static void Internal_Particles_Clear(MonoString* objectName)
{
    char* name = mono_string_to_utf8(objectName);
    API::Particles_Clear(name);
    mono_free(name);
}

static bool Internal_Particles_IsEmitting(MonoString* objectName)
{
    char* name = mono_string_to_utf8(objectName);
    bool result = API::Particles_IsEmitting(name);
    mono_free(name);
    return result;
}

static int Internal_Particles_GetActiveCount(MonoString* objectName)
{
    char* name = mono_string_to_utf8(objectName);
    int result = API::Particles_GetActiveCount(name);
    mono_free(name);
    return result;
}

static int Internal_Particles_GetMaxParticles(MonoString* objectName)
{
    char* name = mono_string_to_utf8(objectName);
    int result = API::Particles_GetMaxParticles(name);
    mono_free(name);
    return result;
}

static void Internal_Particles_SetEmissionRate(MonoString* objectName, float rate)
{
    char* name = mono_string_to_utf8(objectName);
    API::Particles_SetEmissionRate(name, rate);
    mono_free(name);
}

static void Internal_Particles_SetMaxParticles(MonoString* objectName, int max)
{
    char* name = mono_string_to_utf8(objectName);
    API::Particles_SetMaxParticles(name, max);
    mono_free(name);
}

static void Internal_Particles_SetDuration(MonoString* objectName, float seconds)
{
    char* name = mono_string_to_utf8(objectName);
    API::Particles_SetDuration(name, seconds);
    mono_free(name);
}

static void Internal_Particles_SetLoop(MonoString* objectName, bool shouldLoop)
{
    char* name = mono_string_to_utf8(objectName);
    API::Particles_SetLoop(name, shouldLoop);
    mono_free(name);
}

static void Internal_Particles_SetLifetime(MonoString* objectName, float seconds, float variation)
{
    char* name = mono_string_to_utf8(objectName);
    API::Particles_SetLifetime(name, seconds, variation);
    mono_free(name);
}

static void Internal_Particles_SetSpawnOffset(MonoString* objectName, float x, float y)
{
    char* name = mono_string_to_utf8(objectName);
    API::Particles_SetSpawnOffset(name, x, y);
    mono_free(name);
}

static void Internal_Particles_SetSpawnRadius(MonoString* objectName, float radius)
{
    char* name = mono_string_to_utf8(objectName);
    API::Particles_SetSpawnRadius(name, radius);
    mono_free(name);
}

static void Internal_Particles_SetSpawnBox(MonoString* objectName, float halfWidth, float halfHeight)
{
    char* name = mono_string_to_utf8(objectName);
    API::Particles_SetSpawnBox(name, halfWidth, halfHeight);
    mono_free(name);
}

static void Internal_Particles_SetVelocity(MonoString* objectName, float dirX, float dirY, float magnitude, float variation)
{
    char* name = mono_string_to_utf8(objectName);
    API::Particles_SetVelocity(name, dirX, dirY, magnitude, variation);
    mono_free(name);
}

static void Internal_Particles_SetAngleSpread(MonoString* objectName, float degrees)
{
    char* name = mono_string_to_utf8(objectName);
    API::Particles_SetAngleSpread(name, degrees);
    mono_free(name);
}

static void Internal_Particles_SetGravity(MonoString* objectName, float gravX, float gravY)
{
    char* name = mono_string_to_utf8(objectName);
    API::Particles_SetGravity(name, gravX, gravY);
    mono_free(name);
}

static void Internal_Particles_SetDrag(MonoString* objectName, float drag)
{
    char* name = mono_string_to_utf8(objectName);
    API::Particles_SetDrag(name, drag);
    mono_free(name);
}

static void Internal_Particles_SetStartColor(MonoString* objectName, float r, float g, float b, float a)
{
    char* name = mono_string_to_utf8(objectName);
    API::Particles_SetStartColor(name, r, g, b, a);
    mono_free(name);
}

static void Internal_Particles_SetEndColor(MonoString* objectName, float r, float g, float b, float a)
{
    char* name = mono_string_to_utf8(objectName);
    API::Particles_SetEndColor(name, r, g, b, a);
    mono_free(name);
}

static void Internal_Particles_SetStartSize(MonoString* objectName, float size)
{
    char* name = mono_string_to_utf8(objectName);
    API::Particles_SetStartSize(name, size);
    mono_free(name);
}

static void Internal_Particles_SetEndSize(MonoString* objectName, float size)
{
    char* name = mono_string_to_utf8(objectName);
    API::Particles_SetEndSize(name, size);
    mono_free(name);
}

static void Internal_Particles_SetLayer(MonoString* objectName, int layer)
{
    char* name = mono_string_to_utf8(objectName);
    API::Particles_SetLayer(name, layer);
    mono_free(name);
}

static void Internal_Particles_SetFadeIn(MonoString* objectName, float duration)
{
    char* name = mono_string_to_utf8(objectName);
    API::Particles_SetFadeIn(name, duration);
    mono_free(name);
}

static void Internal_Particles_SetFadeOut(MonoString* objectName, float duration)
{
    char* name = mono_string_to_utf8(objectName);
    API::Particles_SetFadeOut(name, duration);
    mono_free(name);
}

static void Internal_Particles_SetStartRotation(MonoString* objectName, float degrees, float variation)
{
    char* name = mono_string_to_utf8(objectName);
    API::Particles_SetStartRotation(name, degrees, variation);
    mono_free(name);
}

static void Internal_Particles_SetAngularVelocity(MonoString* objectName, float degreesPerSec, float variation)
{
    char* name = mono_string_to_utf8(objectName);
    API::Particles_SetAngularVelocity(name, degreesPerSec, variation);
    mono_free(name);
}

static void Internal_Particles_SetTexture(MonoString* objectName, MonoString* texturePath)
{
    char* name = mono_string_to_utf8(objectName);
    char* texPath = mono_string_to_utf8(texturePath);
    API::Particles_SetTexture(name, texPath);
    mono_free(name);
    mono_free(texPath);
}

static void Internal_Particles_ConfigureDashTrail(MonoString* objectName)
{
    char* name = mono_string_to_utf8(objectName);
    API::Particles_ConfigureDashTrail(name);
    mono_free(name);
}

static void Internal_Particles_ConfigureJumpDust(MonoString* objectName)
{
    char* name = mono_string_to_utf8(objectName);
    API::Particles_ConfigureJumpDust(name);
    mono_free(name);
}

static void Internal_Particles_ConfigureWallSlideParticles(MonoString* objectName)
{
    char* name = mono_string_to_utf8(objectName);
    API::Particles_ConfigureWallSlideParticles(name);
    mono_free(name);
}

static void Internal_Particles_ConfigureSpeedLines(MonoString* objectName)
{
    char* name = mono_string_to_utf8(objectName);
    API::Particles_ConfigureSpeedLines(name);
    mono_free(name);
}

static void Internal_Particles_ConfigureDeathExplosion(MonoString* objectName)
{
    char* name = mono_string_to_utf8(objectName);
    API::Particles_ConfigureDeathExplosion(name);
    mono_free(name);
}

// === Ghost ===
static void Internal_Ghost_Configure(MonoString* liveTarget, MonoString* ghostTarget,
                                     MonoString* liveVfx, MonoString* ghostVfx)
{
    char* live  = mono_string_to_utf8(liveTarget);
    char* ghost = mono_string_to_utf8(ghostTarget);
    char* lvfx  = mono_string_to_utf8(liveVfx);
    char* gvfx  = mono_string_to_utf8(ghostVfx);
    API::Ghost::Configure(live, ghost, lvfx, gvfx);
    mono_free(live); mono_free(ghost); mono_free(lvfx); mono_free(gvfx);
}

static void Internal_Ghost_StartRecording(MonoString* targetName)
{
    char* name = mono_string_to_utf8(targetName);
    API::Ghost::StartRecording(name);
    mono_free(name);
}

static void Internal_Ghost_StopRecording()
{
    API::Ghost::StopRecording();
}

static bool Internal_Ghost_SaveBestRun(MonoString* levelName, float finishTime)
{
    char* name = mono_string_to_utf8(levelName);
    bool result = API::Ghost::SaveBestRun(name, finishTime);
    mono_free(name);
    return result;
}

static void Internal_Ghost_StartPlayback(MonoString* ghostObjectName)
{
    char* name = mono_string_to_utf8(ghostObjectName);
    API::Ghost::StartPlayback(name);
    mono_free(name);
}

static void Internal_Ghost_StopPlayback()
{
    API::Ghost::StopPlayback();
}

static bool Internal_Ghost_LoadRecording(MonoString* levelName)
{
    char* name = mono_string_to_utf8(levelName);
    bool result = API::Ghost::LoadRecording(name);
    mono_free(name);
    return result;
}

static bool Internal_Ghost_IsGhostAvailable()
{
    return API::Ghost::IsGhostAvailable();
}

static float Internal_Ghost_GetBestRunTime()
{
    return API::Ghost::GetBestRunTime();
}

MonoObject* ScriptSystem::CreateScriptInstance(const std::string& scriptClassName, GameObject* owner)
{
    if (!coreAssembly || !coreImage)
    {
        std::cerr << "[ScriptSystem] ERROR: Assembly not loaded!\n";
        return nullptr;
    }

    if (!owner)
    {
        std::cerr << "[ScriptSystem] ERROR: Owner GameObject is null!\n";
        return nullptr;
    }

    std::cout << "[ScriptSystem] Creating instance of " << scriptClassName
        << " for " << owner->GetName() << "\n";

    // Find or cache the MonoClass
    if (scriptClasses.find(scriptClassName) == scriptClasses.end())
    {
        std::cout << "[ScriptSystem] Class not cached, loading: " << scriptClassName << "\n";

        MonoClass* monoClass = mono_class_from_name(coreImage, "CSharpScript", scriptClassName.c_str());

        if (!monoClass)
        {
            std::cerr << "[ScriptSystem] ERROR: Could not find class 'CSharpScript."
                << scriptClassName << "' in assembly!\n";
            return nullptr;
        }

        // Cache the class
        auto sc = std::make_unique<ScriptClass>();
        sc->monoClass = monoClass;
        sc->onAwake = mono_class_get_method_from_name(monoClass, "OnAwake", 0);
        sc->onStart = mono_class_get_method_from_name(monoClass, "OnStart", 0);
        sc->onUpdate = mono_class_get_method_from_name(monoClass, "OnUpdate", 1);
        sc->onDestroy = mono_class_get_method_from_name(monoClass, "OnDestroy", 0);

        scriptClasses[scriptClassName] = std::move(sc);

        std::cout << "[ScriptSystem] Cached class: " << scriptClassName << "\n";
    }

    // Get the cached class
    ScriptClass* scriptClass = scriptClasses[scriptClassName].get();

    // Create instance
    MonoObject* instance = InstantiateClass(scriptClass->monoClass);

    if (!instance)
    {
        std::cerr << "[ScriptSystem] ERROR: Failed to instantiate " << scriptClassName << "\n";
        return nullptr;
    }

    std::cout << "[ScriptSystem] Successfully instantiated " << scriptClassName << "\n";

    // Inject OwnerName
    MonoClass* instanceClass = mono_object_get_class(instance);
    MonoClassField* ownerNameField = mono_class_get_field_from_name(instanceClass, "OwnerName");

    if (ownerNameField)
    {
        std::string ownerName = owner->GetName();
        MonoString* monoName = mono_string_new(mono_domain_get(), ownerName.c_str());
        mono_field_set_value(instance, ownerNameField, monoName);

        std::cout << "[ScriptSystem] Injected OwnerName: " << ownerName << "\n";
    }
    else
    {
        std::cerr << "[ScriptSystem] WARNING: Could not find 'OwnerName' field in "
            << scriptClassName << ". Does it inherit from GameScript?\n";
    }

    return instance;
}

ScriptSystem::~ScriptSystem()
{
    Shutdown();
}

void ScriptSystem::Initialize()
{
    std::cout << "\n--- Initializing Scripting System (Mono) ---" << std::endl;

    // Set Mono directories
    mono_set_dirs("mono/lib", "mono/etc");

    // Initialize Mono
    rootDomain = mono_jit_init("GameEngineRuntime");
    if (!rootDomain)
    {
        std::cerr << "[ScriptSystem] FATAL: Failed to initialize Mono JIT!" << std::endl;
        return;
    }

    std::cout << "[ScriptSystem] Root domain created successfully\n";

    // Create app domain for script reloading
    char domainName[] = "GameScriptDomain";
    appDomain = mono_domain_create_appdomain(domainName, nullptr);

    if (!appDomain)
    {
        std::cerr << "[ScriptSystem] FATAL: Failed to create app domain!\n";
        return;
    }

    mono_domain_set(appDomain, true);
    std::cout << "[ScriptSystem] App domain created and set successfully\n";

    // Register internal calls
    RegisterInternalCalls();
    std::cout << "[ScriptSystem] Internal calls registered\n";

    // Load core assembly
    if (!LoadScriptAssembly("./Scripts/GameScripts.dll"))
    {
        std::cerr << "[ScriptSystem] ERROR: Could not load GameScripts.dll\n";
    }

    std::cout << "[ScriptSystem] Scripting System initialized successfully" << std::endl;
}

void ScriptSystem::Shutdown()
{
    // Clean up all script instances
    for (auto &[gameObject, instances] : gameObjectScripts)
    {
        for (auto &instance : instances)
        {
            if (instance.gcHandle != 0)
            {
                mono_gchandle_free(instance.gcHandle);
            }
        }
    }
    gameObjectScripts.clear();
    scriptClasses.clear();

    // Shutdown Mono
    if (appDomain)
    {
        mono_domain_set(mono_get_root_domain(), false);
        mono_domain_unload(appDomain);
        appDomain = nullptr;
    }

    if (rootDomain)
    {
        mono_jit_cleanup(rootDomain);
        rootDomain = nullptr;
    }
}

void ScriptSystem::RegisterInternalCalls()
{
    // === Debug ===
    mono_add_internal_call("CSharpScript.Debug::Log", (void *)Internal_Log);

    // === GameObject Creation ===
    mono_add_internal_call("CSharpScript.GameObject::CreateShape", (void *)Internal_CreateShape);
    mono_add_internal_call("CSharpScript.GameObject::CreateImage", (void *)Internal_CreateImage);
    mono_add_internal_call("CSharpScript.GameObject::CreateAnimatedImage", (void *)Internal_CreateAnimatedImage);
    mono_add_internal_call("CSharpScript.GameObject::Destroy",             (void *)Internal_DestroyGameObject);
    mono_add_internal_call("CSharpScript.GameObject::DestroyByID",         (void *)Internal_DestroyByID);
    mono_add_internal_call("CSharpScript.GameObject::DestroyByName",       (void *)Internal_DestroyByName);
    mono_add_internal_call("CSharpScript.GameObject::FindGameObjectByName", (void *)Internal_FindGameObjectByName);
    mono_add_internal_call("CSharpScript.GameObject::FindGameObjectByID",   (void *)Internal_FindGameObjectByID);
    mono_add_internal_call("CSharpScript.GameObject::CountAllGameObjects",  (void *)Internal_CountAllGameObjects);
    mono_add_internal_call("CSharpScript.GameObject::GetHighestLocalID",   (void *)Internal_GetHighestLocalID);

    // === GameTimer ===
    mono_add_internal_call("CSharpScript.GameTimer::GetRemainingTime",    (void *)Internal_GetRemainingTime);
    mono_add_internal_call("CSharpScript.GameTimer::GetLevelLength",      (void *)Internal_GetLevelLength);
    mono_add_internal_call("CSharpScript.GameTimer::SetLevelTimerLength", (void *)Internal_SetLevelTimerLength);
    mono_add_internal_call("CSharpScript.GameTimer::AddTime",             (void *)Internal_AddTime);
    mono_add_internal_call("CSharpScript.GameObject::CountAll",             (void *)Internal_CountAllGameObjects);
    mono_add_internal_call("CSharpScript.GameObject::GetAllIDs",            (void *)Internal_GetAllGameObjectIDs);

    // === Transform ===
    mono_add_internal_call("CSharpScript.Transform::SetPosition", (void *)Internal_SetPosition);
    mono_add_internal_call("CSharpScript.Transform::SetRotation", (void *)Internal_SetRotation);
    mono_add_internal_call("CSharpScript.Transform::AddRotation", (void *)Internal_AddRotation);
    mono_add_internal_call("CSharpScript.Transform::SetScale_Single", (void *)Internal_SetScale_Single);
    mono_add_internal_call("CSharpScript.Transform::SetScale_XY", (void *)Internal_SetScale_XY);
    mono_add_internal_call("CSharpScript.Transform::GetPosition_Internal", (void*)Internal_GetPosition);
    mono_add_internal_call("CSharpScript.Transform::GetRotation", (void*)Internal_GetRotation);
    mono_add_internal_call("CSharpScript.Transform::GetScale_Internal", (void*)Internal_GetScale);
    mono_add_internal_call("CSharpScript.Transform::GetPositionByID_Internal", (void*)Internal_GetPositionByID);
    mono_add_internal_call("CSharpScript.Transform::GetScaleByID_Internal",    (void*)Internal_GetScaleByID);
    mono_add_internal_call("CSharpScript.Transform::GetRotationByID",          (void*)Internal_GetRotationByID);
    mono_add_internal_call("CSharpScript.Transform::SetPositionByID",          (void*)Internal_SetPosByID);
    mono_add_internal_call("CSharpScript.Transform::SetScaleByID",             (void*)Internal_SetScaleByID);
    mono_add_internal_call("CSharpScript.Transform::SetRotationByID",          (void*)Internal_SetRotByID);

    // === Graphics ===
    mono_add_internal_call("CSharpScript.Graphics::SetShapeType", (void *)Internal_SetShapeType);
    mono_add_internal_call("CSharpScript.Graphics::SetTexture", (void *)Internal_SetTexture);
    mono_add_internal_call("CSharpScript.Graphics::SetColor", (void *)Internal_SetColor);
    mono_add_internal_call("CSharpScript.Graphics::SetAlpha", (void *)Internal_SetAlpha);
    mono_add_internal_call("CSharpScript.Graphics::GetColor_Internal", (void*)Internal_GetColor);
    mono_add_internal_call("CSharpScript.Graphics::SetRenderLayer", (void *)Internal_SetRenderLayer);
    mono_add_internal_call("CSharpScript.Graphics::GetRenderLayer", (void *)Internal_GetRenderLayer);
    mono_add_internal_call("CSharpScript.Graphics::SetLayerVisible", (void *)Internal_SetLayerVisible);
    mono_add_internal_call("CSharpScript.Graphics::IsLayerVisible", (void *)Internal_IsLayerVisible);
    mono_add_internal_call("CSharpScript.Graphics::GetColorByID_Internal", (void*)Internal_GetColorByID);
    mono_add_internal_call("CSharpScript.Graphics::GetRenderLayerByID",    (void*)Internal_GetRenderLayerByID);
    mono_add_internal_call("CSharpScript.Graphics::SetColorByID",          (void*)Internal_SetColorByID);
    mono_add_internal_call("CSharpScript.Graphics::SetAlphaByID",          (void*)Internal_SetAlphaByID);
    mono_add_internal_call("CSharpScript.Graphics::SetRenderLayerByID",    (void*)Internal_SetRenderLayerByID);
    mono_add_internal_call("CSharpScript.Graphics::SetTextureByID",        (void*)Internal_SetTextureByID);

    // === Physics ===
    mono_add_internal_call("CSharpScript.Physics::SetVelocity", (void *)Internal_SetVelocity);
    mono_add_internal_call("CSharpScript.Physics::SetJumpValue", (void *)Internal_SetJumpValue);
    mono_add_internal_call("CSharpScript.Physics::SetGravity", (void *)Internal_SetGravity);
    mono_add_internal_call("CSharpScript.Physics::SetStatic", (void *)Internal_SetStatic);
    mono_add_internal_call("CSharpScript.Physics::GetVelocity_Internal", (void*)Internal_GetVelocity);
    mono_add_internal_call("CSharpScript.Physics::SetMass", (void*)Internal_SetMass);
    mono_add_internal_call("CSharpScript.Physics::SetFriction", (void*)Internal_SetFriction);
    mono_add_internal_call("CSharpScript.Physics::GetVelocityByID_Internal", (void*)Internal_GetVelocityByID);
    mono_add_internal_call("CSharpScript.Physics::SetVelocityByID",          (void*)Internal_SetVelocityByID);

    // === Collision ===
    mono_add_internal_call("CSharpScript.Collision::SetCollisionShape", (void *)Internal_SetCollisionShape);
    mono_add_internal_call("CSharpScript.Collision::SetCollisionLayer", (void *)Internal_SetCollisionLayer);
    mono_add_internal_call("CSharpScript.Collision::GetCollisionLayer", (void*)Internal_GetCollisionLayer);
    mono_add_internal_call("CSharpScript.Collision::GetCollisionShape", (void*)Internal_GetCollisionShape);
    mono_add_internal_call("CSharpScript.Collision::CheckColliderLayer", (void*)Internal_CheckColliderLayer);
    mono_add_internal_call("CSharpScript.Collision::IsCollidingWithLayer", (void*)Internal_IsCollidingWithLayer);

    // === Scripting ===
    mono_add_internal_call("CSharpScript.Scripting::AttachScript", (void*)Internal_AttachScript);
    mono_add_internal_call("CSharpScript.Scripting::GetCollidingObjectName", (void*)Internal_GetCollidingObjectNameOnLayer);
    mono_add_internal_call("CSharpScript.Scripting::GetCollidingObjectID",   (void*)Internal_GetCollidingObjectIDOnLayer);
    mono_add_internal_call("CSharpScript.Scripting::GetFloatField",  (void*)Internal_GetFloatField);
    mono_add_internal_call("CSharpScript.Scripting::GetIntField",    (void*)Internal_GetIntField);
    mono_add_internal_call("CSharpScript.Scripting::GetBoolField",   (void*)Internal_GetBoolField);
    mono_add_internal_call("CSharpScript.Scripting::GetStringField", (void*)Internal_GetStringField);
    mono_add_internal_call("CSharpScript.Scripting::SetFloatField",  (void*)Internal_SetFloatField);
    mono_add_internal_call("CSharpScript.Scripting::SetIntField",    (void*)Internal_SetIntField);
    mono_add_internal_call("CSharpScript.Scripting::SetBoolField",   (void*)Internal_SetBoolField);
    mono_add_internal_call("CSharpScript.Scripting::SetStringField", (void*)Internal_SetStringField);

    // === Components ===
    mono_add_internal_call("CSharpScript.Components::SetComponentActive", (void*)Internal_SetComponentActive);
    mono_add_internal_call("CSharpScript.Components::RemoveComponent", (void*)Internal_RemoveComponent);
    mono_add_internal_call("CSharpScript.Components::AddTransformComponent", (void *)Internal_AddTransformComponent);
    mono_add_internal_call("CSharpScript.Components::AddRenderComponent", (void *)Internal_AddRenderComponent);
    mono_add_internal_call("CSharpScript.Components::AddCollisionComponent", (void *)Internal_AddCollisionComponent);
    mono_add_internal_call("CSharpScript.Components::AddPhysicsComponent", (void *)Internal_AddPhysicsComponent);
    mono_add_internal_call("CSharpScript.Components::AddTextComponent", (void*)Internal_AddTextComponent);

    // === Text ===
    mono_add_internal_call("CSharpScript.Text::SetText", (void*)Internal_SetText);
    mono_add_internal_call("CSharpScript.Text::GetText", (void*)Internal_GetText);
    mono_add_internal_call("CSharpScript.Text::SetTextColor", (void*)Internal_SetTextColor);
    mono_add_internal_call("CSharpScript.Text::GetTextColor_Internal", (void*)Internal_GetTextColor);
    mono_add_internal_call("CSharpScript.Text::SetTextScale", (void*)Internal_SetTextScale);
    mono_add_internal_call("CSharpScript.Text::GetTextScale", (void*)Internal_GetTextScale);
    mono_add_internal_call("CSharpScript.Text::SetTextScreenSpace", (void*)Internal_SetTextScreenSpace);
    mono_add_internal_call("CSharpScript.Text::GetTextScreenSpace", (void*)Internal_GetTextScreenSpace);
    mono_add_internal_call("CSharpScript.Text::GetTextByID",        (void*)Internal_GetTextByID);
    mono_add_internal_call("CSharpScript.Text::SetTextByID",        (void*)Internal_SetTextByID);

    // === Audio ===
    mono_add_internal_call("CSharpScript.Audio::AddSound", (void*)Internal_AddSound);
    mono_add_internal_call("CSharpScript.Audio::PlaySound", (void*)Internal_PlaySound);
    mono_add_internal_call("CSharpScript.Audio::StopSound", (void*)Internal_StopSound);
    mono_add_internal_call("CSharpScript.Audio::StopAllSounds", (void*)Internal_StopAllSounds);
    mono_add_internal_call("CSharpScript.Audio::RemoveSound", (void*)Internal_RemoveSound);
    mono_add_internal_call("CSharpScript.Audio::TriggerEvent", (void*)Internal_TriggerEvent);
    mono_add_internal_call("CSharpScript.Audio::AddEventSound", (void*)Internal_AddEventSound);
    mono_add_internal_call("CSharpScript.Audio::SetVolume", (void*)Internal_SetVolume);
    mono_add_internal_call("CSharpScript.Audio::SetCategoryVolume", (void*)Internal_SetCategoryVolume);
    mono_add_internal_call("CSharpScript.Audio::SetPitch", (void*)Internal_SetPitch);
    mono_add_internal_call("CSharpScript.Audio::PauseSound", (void*)Internal_PauseSound);
    mono_add_internal_call("CSharpScript.Audio::PauseAllSounds", (void*)Internal_PauseAllSounds);
    mono_add_internal_call("CSharpScript.Audio::ToggleMasterMute", (void*)Internal_ToggleMasterMute);
    mono_add_internal_call("CSharpScript.Audio::ToggleCategoryMute", (void*)Internal_ToggleCategoryMute);
    mono_add_internal_call("CSharpScript.Audio::IsMasterMuted", (void*)Internal_IsMasterMuted);
    mono_add_internal_call("CSharpScript.Audio::IsCategoryMuted", (void*)Internal_IsCategoryMuted);
    mono_add_internal_call("CSharpScript.Audio::FadeOutCategory", (void*)Internal_FadeOutCategory);
    mono_add_internal_call("CSharpScript.Audio::FadeInCategory", (void*)Internal_FadeInCategory);
    mono_add_internal_call("CSharpScript.Audio::FadeOutAll", (void*)Internal_FadeOutAll);
    mono_add_internal_call("CSharpScript.Audio::Set3DListenerPosition", (void*)Internal_Set3DListenerPosition);
    mono_add_internal_call("CSharpScript.Audio::Set3DSoundPosition", (void*)Internal_Set3DSoundPosition);
    mono_add_internal_call("CSharpScript.Audio::Load3DSound", (void*)Internal_Load3DSound);

    // === Animation ===
    mono_add_internal_call("CSharpScript.Animation::PlayAnimation", (void*)Internal_PlayAnimation);
    mono_add_internal_call("CSharpScript.Animation::SetLooping", (void*)Internal_SetAnimationLooping);
    mono_add_internal_call("CSharpScript.Animation::IsLooping", (void*)Internal_IsAnimationLooping);
    mono_add_internal_call("CSharpScript.Animation::AddToQueue", (void*)Internal_AddAnimationToQueue);
    mono_add_internal_call("CSharpScript.Animation::RegisterAnimation", (void*)Internal_RegisterAnimation);
    mono_add_internal_call("CSharpScript.Animation::SetAnimation", (void*)Internal_SetAnimation);
    mono_add_internal_call("CSharpScript.Animation::GetCurrentAnimationName", (void*)Internal_GetCurrentAnimationName);
    mono_add_internal_call("CSharpScript.Animation::LoadNewSpritesheet", (void*)Internal_LoadNewSpritesheet);
    mono_add_internal_call("CSharpScript.Animation::SetCurrentFrame", (void*)Internal_SetCurrentFrame);
    mono_add_internal_call("CSharpScript.Animation::GetRows", (void*)Internal_GetRows);
    mono_add_internal_call("CSharpScript.Animation::GetCols", (void*)Internal_GetCols);
    mono_add_internal_call("CSharpScript.Animation::GetCurrentRow", (void*)Internal_GetCurrentRow);
    mono_add_internal_call("CSharpScript.Animation::GetCurrentCol", (void*)Internal_GetCurrentCol);
    mono_add_internal_call("CSharpScript.Animation::GetCurrentAnimationNameByID", (void*)Internal_GetCurrentAnimationNameByID);
    mono_add_internal_call("CSharpScript.Animation::GetRowsByID",       (void*)Internal_GetRowsByID);
    mono_add_internal_call("CSharpScript.Animation::GetColsByID",       (void*)Internal_GetColsByID);
    mono_add_internal_call("CSharpScript.Animation::GetCurrentRowByID", (void*)Internal_GetCurrentRowByID);
    mono_add_internal_call("CSharpScript.Animation::GetCurrentColByID", (void*)Internal_GetCurrentColByID);
    mono_add_internal_call("CSharpScript.Animation::PlayAnimationByID", (void*)Internal_PlayAnimationByID);


    // === Camera ===
    mono_add_internal_call("CSharpScript.Camera::SetPosition", (void*)Internal_SetCameraPosition);
    mono_add_internal_call("CSharpScript.Camera::SetZoom", (void*)Internal_SetCameraZoom);
    mono_add_internal_call("CSharpScript.Camera::SetMode", (void*)Internal_SetCameraMode);
    mono_add_internal_call("CSharpScript.Camera::GetPosition_Internal", (void*)Internal_GetCameraPosition);
    mono_add_internal_call("CSharpScript.Camera::GetZoom", (void*)Internal_GetCameraZoom);
    mono_add_internal_call("CSharpScript.Camera::GetMode", (void*)Internal_GetCameraMode);
    mono_add_internal_call("CSharpScript.Camera::CameraFlash", (void*)Internal_CameraFlash);
    mono_add_internal_call("CSharpScript.Camera::SetFollowOffset", (void*)Internal_SetCameraFollowOffset);
    mono_add_internal_call("CSharpScript.Camera::GetFollowOffset_Internal", (void*)Internal_GetCameraFollowOffset);

    // === Collision Queries ===
    mono_add_internal_call("CSharpScript.Collision::Raycast_Internal", (void*)Internal_Raycast);
    mono_add_internal_call("CSharpScript.Collision::OnTriggerEnter", (void*)Internal_OnTriggerEnter);
    mono_add_internal_call("CSharpScript.Collision::OnTriggerExit", (void*)Internal_OnTriggerExit);
    mono_add_internal_call("CSharpScript.Collision::OnTriggerStay", (void*)Internal_OnTriggerStay);

    // === Utils ===
    mono_add_internal_call("CSharpScript.Utils::RandomRange", (void*)Internal_RandomRange);
    mono_add_internal_call("CSharpScript.Utils::GetDelta_Time", (void*)Internal_GetDelta_Time);
    mono_add_internal_call("CSharpScript.Utils::GetFixed_Delta_Time", (void*)Internal_GetFixed_Delta_Time);
    mono_add_internal_call("CSharpScript.Utils::GetFramesPerSecond", (void*)Internal_GetFramesPerSecond);
	mono_add_internal_call("CSharpScript.Utils::MathfLerp", (void*)API::fLerp);
	mono_add_internal_call("CSharpScript.Utils::MathfInverseLerp", (void*)API::fInverseLerp);
	mono_add_internal_call("CSharpScript.Utils::MathfClamp", (void*)API::fClamp);
	mono_add_internal_call("CSharpScript.Utils::MathiClamp", (void*)API::iClamp);

    // === Input ===
    mono_add_internal_call("CSharpScript.Input::IsKeyPressed", (void*)Internal_IsKeyPressed);
    mono_add_internal_call("CSharpScript.Input::IsKeyReleased", (void*)Internal_IsKeyReleased);
    mono_add_internal_call("CSharpScript.Input::IsKeyHeld", (void*)Internal_IsKeyHeld);
    mono_add_internal_call("CSharpScript.Input::IsMouseButtonPressed", (void*)Internal_IsMouseButtonPressed);
    mono_add_internal_call("CSharpScript.Input::IsMouseButtonReleased", (void*)Internal_IsMouseButtonReleased);
    mono_add_internal_call("CSharpScript.Input::IsMouseButtonHeld", (void*)Internal_IsMouseButtonHeld);

    mono_add_internal_call("CSharpScript.Input::GetMouseX", (void*)Internal_GetMouseX);
    mono_add_internal_call("CSharpScript.Input::GetMouseY", (void*)Internal_GetMouseY);
    mono_add_internal_call("CSharpScript.Input::GetMouseWorldX", (void*)Internal_GetMouseWorldX);
    mono_add_internal_call("CSharpScript.Input::GetMouseWorldY", (void*)Internal_GetMouseWorldY);
    mono_add_internal_call("CSharpScript.Input::IsGamepadButtonPressed",  (void*)Internal_IsGamepadButtonPressed);
    mono_add_internal_call("CSharpScript.Input::IsGamepadButtonReleased", (void*)Internal_IsGamepadButtonReleased);
    mono_add_internal_call("CSharpScript.Input::IsGamepadButtonHeld",     (void*)Internal_IsGamepadButtonHeld);
    mono_add_internal_call("CSharpScript.Input::GetGamepadAxis",          (void*)Internal_GetGamepadAxis);
    mono_add_internal_call("CSharpScript.Input::IsGamepadConnected",      (void*)Internal_IsGamepadConnected);

    // === Pickup System ===
    mono_add_internal_call("CSharpScript.Pickups::CollectCoin",      (void*)Internal_CollectCoin);
    mono_add_internal_call("CSharpScript.Pickups::CollectShell",     (void*)Internal_CollectShell);
    mono_add_internal_call("CSharpScript.Pickups::GetCurrentCoins",  (void*)Internal_GetCurrentCoins);
    mono_add_internal_call("CSharpScript.Pickups::GetCurrentShells", (void*)Internal_GetCurrentShells);
    mono_add_internal_call("CSharpScript.Pickups::GetTotalCoins",    (void*)Internal_GetTotalCoins);
    mono_add_internal_call("CSharpScript.Pickups::GetTotalShells",   (void*)Internal_GetTotalShells);

    // === WinLose System ===
    mono_add_internal_call("CSharpScript.WinLoseSystem::TriggerLose", (void*)Internal_TriggerLoseZone);
    mono_add_internal_call("CSharpScript.WinLoseSystem::IsWinScreenActive", (void*)Internal_IsWinScreenActive);
    mono_add_internal_call("CSharpScript.WinLoseSystem::IsLoseScreenActive", (void*)Internal_IsLoseScreenActive);

    // === Particles ===
    mono_add_internal_call("CSharpScript.Particles::AddParticleEmitterComponent", (void*)Internal_AddParticleEmitterComponent);
    mono_add_internal_call("CSharpScript.Particles::StartEmission", (void*)Internal_Particles_StartEmission);
    mono_add_internal_call("CSharpScript.Particles::StopEmission", (void*)Internal_Particles_StopEmission);
    mono_add_internal_call("CSharpScript.Particles::Burst", (void*)Internal_Particles_Burst);
    mono_add_internal_call("CSharpScript.Particles::Clear", (void*)Internal_Particles_Clear);
    mono_add_internal_call("CSharpScript.Particles::IsEmitting", (void*)Internal_Particles_IsEmitting);
    mono_add_internal_call("CSharpScript.Particles::GetActiveParticleCount", (void*)Internal_Particles_GetActiveCount);
    mono_add_internal_call("CSharpScript.Particles::GetMaxParticles", (void*)Internal_Particles_GetMaxParticles);
    mono_add_internal_call("CSharpScript.Particles::SetEmissionRate", (void*)Internal_Particles_SetEmissionRate);
    mono_add_internal_call("CSharpScript.Particles::SetMaxParticles", (void*)Internal_Particles_SetMaxParticles);
    mono_add_internal_call("CSharpScript.Particles::SetDuration", (void*)Internal_Particles_SetDuration);
    mono_add_internal_call("CSharpScript.Particles::SetLoop", (void*)Internal_Particles_SetLoop);
    mono_add_internal_call("CSharpScript.Particles::SetLifetime", (void*)Internal_Particles_SetLifetime);
    mono_add_internal_call("CSharpScript.Particles::SetSpawnOffset", (void*)Internal_Particles_SetSpawnOffset);
    mono_add_internal_call("CSharpScript.Particles::SetSpawnRadius", (void*)Internal_Particles_SetSpawnRadius);
    mono_add_internal_call("CSharpScript.Particles::SetSpawnBox", (void*)Internal_Particles_SetSpawnBox);
    mono_add_internal_call("CSharpScript.Particles::SetVelocity", (void*)Internal_Particles_SetVelocity);
    mono_add_internal_call("CSharpScript.Particles::SetAngleSpread", (void*)Internal_Particles_SetAngleSpread);
    mono_add_internal_call("CSharpScript.Particles::SetGravity", (void*)Internal_Particles_SetGravity);
    mono_add_internal_call("CSharpScript.Particles::SetDrag", (void*)Internal_Particles_SetDrag);
    mono_add_internal_call("CSharpScript.Particles::SetStartColor", (void*)Internal_Particles_SetStartColor);
    mono_add_internal_call("CSharpScript.Particles::SetEndColor", (void*)Internal_Particles_SetEndColor);
    mono_add_internal_call("CSharpScript.Particles::SetStartSize", (void*)Internal_Particles_SetStartSize);
    mono_add_internal_call("CSharpScript.Particles::SetEndSize", (void*)Internal_Particles_SetEndSize);
    mono_add_internal_call("CSharpScript.Particles::SetLayer", (void*)Internal_Particles_SetLayer);
    mono_add_internal_call("CSharpScript.Particles::SetFadeIn", (void*)Internal_Particles_SetFadeIn);
    mono_add_internal_call("CSharpScript.Particles::SetFadeOut", (void*)Internal_Particles_SetFadeOut);
    mono_add_internal_call("CSharpScript.Particles::SetStartRotation", (void*)Internal_Particles_SetStartRotation);
    mono_add_internal_call("CSharpScript.Particles::SetAngularVelocity", (void*)Internal_Particles_SetAngularVelocity);
    mono_add_internal_call("CSharpScript.Particles::SetTexture", (void*)Internal_Particles_SetTexture);
    mono_add_internal_call("CSharpScript.Particles::ConfigureDashTrail", (void*)Internal_Particles_ConfigureDashTrail);
    mono_add_internal_call("CSharpScript.Particles::ConfigureJumpDust", (void*)Internal_Particles_ConfigureJumpDust);
    mono_add_internal_call("CSharpScript.Particles::ConfigureWallSlideParticles", (void*)Internal_Particles_ConfigureWallSlideParticles);
    mono_add_internal_call("CSharpScript.Particles::ConfigureSpeedLines", (void*)Internal_Particles_ConfigureSpeedLines);
    mono_add_internal_call("CSharpScript.Particles::ConfigureDeathExplosion", (void*)Internal_Particles_ConfigureDeathExplosion);

    // === Ghost ===
    mono_add_internal_call("CSharpScript.Ghost::Configure",         (void*)Internal_Ghost_Configure);
    mono_add_internal_call("CSharpScript.Ghost::StartRecording",    (void*)Internal_Ghost_StartRecording);
    mono_add_internal_call("CSharpScript.Ghost::StopRecording",     (void*)Internal_Ghost_StopRecording);
    mono_add_internal_call("CSharpScript.Ghost::SaveBestRun",       (void*)Internal_Ghost_SaveBestRun);
    mono_add_internal_call("CSharpScript.Ghost::StartPlayback",     (void*)Internal_Ghost_StartPlayback);
    mono_add_internal_call("CSharpScript.Ghost::StopPlayback",      (void*)Internal_Ghost_StopPlayback);
    mono_add_internal_call("CSharpScript.Ghost::LoadRecording",     (void*)Internal_Ghost_LoadRecording);
    mono_add_internal_call("CSharpScript.Ghost::IsGhostAvailable",  (void*)Internal_Ghost_IsGhostAvailable);
    mono_add_internal_call("CSharpScript.Ghost::GetBestRunTime",    (void*)Internal_Ghost_GetBestRunTime);
}

MonoAssembly *ScriptSystem::LoadAssembly(const std::string &filepath)
{
    if (!std::filesystem::exists(filepath))
    {
        std::cerr << "Assembly file not found: " << filepath << std::endl;
        return nullptr;
    }

    MonoAssembly *assembly = mono_domain_assembly_open(appDomain, filepath.c_str());
    if (!assembly)
    {
        std::cerr << "Failed to load assembly: " << filepath << std::endl;
        return nullptr;
    }

    return assembly;
}

bool ScriptSystem::LoadScriptAssembly(const std::string& filepath)
{
    std::cout << "[ScriptSystem] Loading assembly from: " << filepath << std::endl;

    if (!std::filesystem::exists(filepath))
    {
        std::cerr << "[ScriptSystem] ERROR: Assembly file does not exist: " << filepath << std::endl;
        return false;
    }

    coreAssembly = LoadAssembly(filepath);
    if (!coreAssembly)
    {
        std::cerr << "[ScriptSystem] ERROR: Failed to load assembly\n";
        return false;
    }

    coreImage = mono_assembly_get_image(coreAssembly);

    if (!coreImage)
    {
        std::cerr << "[ScriptSystem] ERROR: Failed to get image from assembly\n";
        return false;
    }

    std::cout << "[ScriptSystem] Loaded script assembly: " << filepath << std::endl;

    // List all classes in the assembly for debugging (Alternative method)
    std::cout << "[ScriptSystem] Scanning for classes in 'CSharpScript' namespace...\n";

    
    const char* testClasses[] = {
        "GameScript",
        "SpinningSquare",
        "TestGameObject",
        "TestGameObjects2"
    };

    for (const char* className : testClasses)
    {
        MonoClass* testClass = mono_class_from_name(coreImage, "CSharpScript", className);
        if (testClass)
        {
            std::cout << "  Found: CSharpScript." << className << std::endl;
        }
    }

    return true;
}
MonoClass *ScriptSystem::GetClassInAssembly(MonoAssembly *assembly, const char *namespaceName, const char *className)
{
    MonoImage *image = mono_assembly_get_image(assembly);
    MonoClass *monoClass = mono_class_from_name(image, namespaceName, className);

    if (!monoClass)
    {
        std::cerr << "Failed to find class: " << namespaceName << "." << className << std::endl;
    }

    return monoClass;
}

MonoObject *ScriptSystem::InstantiateClass(MonoClass *monoClass)
{
    MonoObject *instance = mono_object_new(appDomain, monoClass);

    if (!instance)
    {
        std::cerr << "Failed to instantiate class" << std::endl;
        return nullptr;
    }

    // Call constructor
    mono_runtime_object_init(instance);

    return instance;
}

bool ScriptSystem::AttachScript(GameObject* gameObject, const std::string& scriptClassName)
{
    if (!gameObject || !coreAssembly)
    {
        std::cerr << "[ScriptSystem] AttachScript failed: null gameObject or coreAssembly\n";
        return false;
    }

    std::cout << "[ScriptSystem] Attempting to attach '" << scriptClassName << "' to '"
        << gameObject->GetName() << "'\n";

    // CRITICAL: Check if the script class exists, if not, try to load it
    if (scriptClasses.find(scriptClassName) == scriptClasses.end())
    {
        std::cout << "[ScriptSystem] Script class not cached, attempting to load: "
            << scriptClassName << std::endl;

        // Try to find and cache the class
        MonoClass* monoClass = GetClassInAssembly(coreAssembly, "CSharpScript", scriptClassName.c_str());

        if (!monoClass)
        {
            std::cerr << "[ScriptSystem] ERROR: Could not find class '" << scriptClassName
                << "' in namespace 'CSharpScript'\n";
            return false;
        }

        // Cache it
        auto sc = std::make_unique<ScriptClass>();
        sc->monoClass = monoClass;
        sc->onAwake = mono_class_get_method_from_name(monoClass, "OnAwake", 0);
        sc->onStart = mono_class_get_method_from_name(monoClass, "OnStart", 0);
        sc->onUpdate = mono_class_get_method_from_name(monoClass, "OnUpdate", 1);
        sc->onDestroy = mono_class_get_method_from_name(monoClass, "OnDestroy", 0);

        std::cout << "[ScriptSystem] Successfully cached class: " << scriptClassName << std::endl;
        std::cout << "  OnAwake: " << (sc->onAwake ? "Found" : "Not Found") << std::endl;
        std::cout << "  OnStart: " << (sc->onStart ? "Found" : "Not Found") << std::endl;
        std::cout << "  OnUpdate: " << (sc->onUpdate ? "Found" : "Not Found") << std::endl;
        std::cout << "  OnDestroy: " << (sc->onDestroy ? "Found" : "Not Found") << std::endl;

        scriptClasses[scriptClassName] = std::move(sc);
    }


    ScriptClass* scriptClass = scriptClasses[scriptClassName].get();


    MonoObject* instance = InstantiateClass(scriptClass->monoClass);

    if (!instance)
    {
        std::cerr << "[ScriptSystem] ERROR: Failed to instantiate " << scriptClassName << std::endl;
        return false;
    }

    std::cout << "[ScriptSystem] Successfully instantiated " << scriptClassName << std::endl;

    // Inject OwnerName
    MonoClass* instanceClass = mono_object_get_class(instance);
    MonoClassField* ownerNameField = mono_class_get_field_from_name(instanceClass, "OwnerName");

    if (ownerNameField)
    {
        std::string goName = gameObject->GetName();
        MonoString* monoName = mono_string_new(appDomain, goName.c_str());
        mono_field_set_value(instance, ownerNameField, monoName);

        std::cout << "[ScriptSystem] Injected OwnerName: " << goName << std::endl;
    }
    else
    {
        std::cerr << "[ScriptSystem] WARNING: Could not find 'OwnerName' field in "
            << scriptClassName << std::endl;
    }


    ScriptInstance scriptInstance;
    scriptInstance.instance = instance;
    scriptInstance.scriptClass = scriptClass;
    scriptInstance.gcHandle = mono_gchandle_new(instance, false);

    gameObjectScripts[gameObject].push_back(scriptInstance);

    std::cout << "[ScriptSystem] Successfully attached " << scriptClassName
        << " to " << gameObject->GetName() << std::endl;

    // OnAwake fires immediately on creation (editor or runtime).
    // OnStart is deferred: EnterPlayMode fires it in debug, or the Update
    // safety-net fires it in release before the first OnUpdate.
    CallOnAwake(gameObject);

    return true;
}
void ScriptSystem::RemoveScript(GameObject *gameObject, const std::string &scriptClassName)
{
    auto it = gameObjectScripts.find(gameObject);
    if (it == gameObjectScripts.end())
    {
        return;
    }

    auto &instances = it->second;
    for (auto instIt = instances.begin(); instIt != instances.end(); ++instIt)
    {
        MonoClass *monoClass = mono_object_get_class(instIt->instance);
        const char *className = mono_class_get_name(monoClass);

        if (scriptClassName == className)
        {
            if (instIt->gcHandle != 0)
            {
                mono_gchandle_free(instIt->gcHandle);
            }
            instances.erase(instIt);
            break;
        }
    }
}

void ScriptSystem::CallOnAwake(GameObject *gameObject)
{
    auto it = gameObjectScripts.find(gameObject);
    if (it == gameObjectScripts.end())
        return;

    for (auto &scriptInstance : it->second)
    {
        if (scriptInstance.isAwakened)
            continue;

        if (!scriptInstance.scriptClass || !scriptInstance.instance)
            continue;

        scriptInstance.isAwakened = true;

        if (!scriptInstance.scriptClass->onAwake)
            continue;

        MonoObject* exception = nullptr;
        mono_runtime_invoke(scriptInstance.scriptClass->onAwake, scriptInstance.instance, nullptr, &exception);

        if (exception)
        {
            MonoClass* exceptionClass = mono_object_get_class(exception);
            const char* exceptionName = mono_class_get_name(exceptionClass);
            MonoProperty* messageProp = mono_class_get_property_from_name(exceptionClass, "Message");
            if (messageProp)
            {
                MonoMethod* getter = mono_property_get_get_method(messageProp);
                MonoString* messageStr = (MonoString*)mono_runtime_invoke(getter, exception, nullptr, nullptr);
                char* message = mono_string_to_utf8(messageStr);
                std::cerr << "[ScriptSystem] Exception in OnAwake for " << gameObject->GetName()
                          << ": " << exceptionName << " - " << message << std::endl;
                mono_free(message);
            }
            else
            {
                std::cerr << "[ScriptSystem] Exception in OnAwake for " << gameObject->GetName()
                          << ": " << exceptionName << std::endl;
            }
        }
    }
}

void ScriptSystem::CallOnStart(GameObject *gameObject)
{
    auto it = gameObjectScripts.find(gameObject);
    if (it == gameObjectScripts.end())
    {
        return;
    }

    for (auto &scriptInstance : it->second)
    {

        if (scriptInstance.isStarted)
        {
            continue;
        }

      
        if (!scriptInstance.scriptClass)
        {
            std::cerr << "[ScriptSystem] ERROR: scriptClass is null for " << gameObject->GetName() << std::endl;
            continue;
        }

        if (!scriptInstance.instance)
        {
            std::cerr << "[ScriptSystem] ERROR: script instance is null for " << gameObject->GetName() << std::endl;
            continue;
        }

        if (!scriptInstance.scriptClass->onStart)
        {

            scriptInstance.isStarted = true;
            continue;
        }

        MonoObject* exception = nullptr;
        mono_runtime_invoke(scriptInstance.scriptClass->onStart, scriptInstance.instance, nullptr, &exception);

        // Mark as started after calling OnStart
        scriptInstance.isStarted = true;

        if (exception)
        {
            MonoClass* exceptionClass = mono_object_get_class(exception);
            const char* exceptionName = mono_class_get_name(exceptionClass);

            MonoProperty* messageProp = mono_class_get_property_from_name(exceptionClass, "Message");
            if (messageProp)
            {
                MonoMethod* getter = mono_property_get_get_method(messageProp);
                MonoString* messageStr = (MonoString*)mono_runtime_invoke(getter, exception, nullptr, nullptr);
                char* message = mono_string_to_utf8(messageStr);

                std::cerr << "[ScriptSystem] Exception in OnStart for " << gameObject->GetName()
                          << ": " << exceptionName << " - " << message << std::endl;

                mono_free(message);
            }
            else
            {
                std::cerr << "[ScriptSystem] Exception in OnStart for " << gameObject->GetName()
                          << ": " << exceptionName << std::endl;
            }
        }
    }
}

void ScriptSystem::CallOnUpdate(GameObject *gameObject, float deltaTime)
{
    auto it = gameObjectScripts.find(gameObject);
    if (it == gameObjectScripts.end())
    {
        return;
    }

    // Get the ScriptComponent from the GameObject and check its enabled state
    ScriptComponent* scriptComponent = gameObject->GetComponent<ScriptComponent>();
    if (!scriptComponent || !scriptComponent->IsEnabled())
    {
        return;
    }

    void *args[1];
    args[0] = &deltaTime;

    for (auto &scriptInstance : it->second)
    {
        // Validate all pointers before calling mono_runtime_invoke
        if (!scriptInstance.scriptClass)
        {
            std::cerr << "[ScriptSystem] ERROR: scriptClass is null for " << gameObject->GetName() << std::endl;
            continue;
        }

        if (!scriptInstance.instance)
        {
            std::cerr << "[ScriptSystem] ERROR: script instance is null for " << gameObject->GetName() << std::endl;
            continue;
        }

        if (!scriptInstance.scriptClass->onUpdate)
        {
            // OnUpdate method not found or doesn't exist - this is normal, skip silently
            continue;
        }

        MonoObject* exception = nullptr;
        mono_runtime_invoke(scriptInstance.scriptClass->onUpdate, scriptInstance.instance, args, &exception);

        if (exception)
        {
            MonoClass* exceptionClass = mono_object_get_class(exception);
            const char* exceptionName = mono_class_get_name(exceptionClass);

            MonoProperty* messageProp = mono_class_get_property_from_name(exceptionClass, "Message");
            if (messageProp)
            {
                MonoMethod* getter = mono_property_get_get_method(messageProp);
                MonoString* messageStr = (MonoString*)mono_runtime_invoke(getter, exception, nullptr, nullptr);
                char* message = mono_string_to_utf8(messageStr);

                std::cerr << "[ScriptSystem] Exception in OnUpdate for " << gameObject->GetName()
                          << ": " << exceptionName << " - " << message << std::endl;

                mono_free(message);
            }
            else
            {
                std::cerr << "[ScriptSystem] Exception in OnUpdate for " << gameObject->GetName()
                          << ": " << exceptionName << std::endl;
            }
        }
    }
}

void ScriptSystem::CallOnDestroy(GameObject *gameObject)
{
    auto it = gameObjectScripts.find(gameObject);
    if (it == gameObjectScripts.end())
    {
        return;
    }

    for (auto &scriptInstance : it->second)
    {
     
        if (!scriptInstance.scriptClass)
        {
            std::cerr << "[ScriptSystem] ERROR: scriptClass is null for " << gameObject->GetName() << std::endl;
            continue;
        }

        if (!scriptInstance.instance)
        {
            std::cerr << "[ScriptSystem] ERROR: script instance is null for " << gameObject->GetName() << std::endl;
            continue;
        }

        if (!scriptInstance.scriptClass->onDestroy)
        {
           
            continue;
        }

        MonoObject* exception = nullptr;
        mono_runtime_invoke(scriptInstance.scriptClass->onDestroy, scriptInstance.instance, nullptr, &exception);

        if (exception)
        {
            MonoClass* exceptionClass = mono_object_get_class(exception);
            const char* exceptionName = mono_class_get_name(exceptionClass);

            MonoProperty* messageProp = mono_class_get_property_from_name(exceptionClass, "Message");
            if (messageProp)
            {
                MonoMethod* getter = mono_property_get_get_method(messageProp);
                MonoString* messageStr = (MonoString*)mono_runtime_invoke(getter, exception, nullptr, nullptr);
                char* message = mono_string_to_utf8(messageStr);

                std::cerr << "[ScriptSystem] Exception in OnDestroy for " << gameObject->GetName()
                          << ": " << exceptionName << " - " << message << std::endl;

                mono_free(message);
            }
            else
            {
                std::cerr << "[ScriptSystem] Exception in OnDestroy for " << gameObject->GetName()
                          << ": " << exceptionName << std::endl;
            }
        }
    }
}

void ScriptSystem::Update(float deltaTime)
{
   
    std::vector<QueuedEvent> eventsCopy = EventSystem::events;
    EventSystem::Clear();

    for (auto& e : eventsCopy)
    {
        if (e.type == EventType::OnDestroy)
        {
            auto it = gameObjectScripts.find(e.target);
            if (it != gameObjectScripts.end())
            {
                for (auto& scriptInstance : it->second)
                {
                    if (scriptInstance.gcHandle != 0)
                    {
                        mono_gchandle_free(scriptInstance.gcHandle);
                    }
                }
                gameObjectScripts.erase(it);
            }
        }
        else
        {
           
            EventSystem::Raise(e.type, e.target, e.other);
        }
    }

    // Update all scripts attached to game objects
    for (auto &[gameObject, instances] : gameObjectScripts)
    {
        if (!LayerManager::IsLayerEnabled(gameObject->GetGameLayer()))
            continue;

        // OnAwake already fired in AttachScript (on creation).
        // Fire OnStart here as a safety-net for release mode:
        // EnterPlayMode calls it for debug, but the first Update tick
        // catches any script that slipped through (e.g. runtime spawns).
        CallOnStart(gameObject);
        CallOnUpdate(gameObject, deltaTime);
    }
}

void ScriptSystem::ReloadScriptAssembly()
{
    std::cout << "Reloading script assembly..." << std::endl;

    for (auto &[gameObject, instances] : gameObjectScripts)
    {
        for (auto &instance : instances)
        {
            if (instance.gcHandle != 0)
            {
                mono_gchandle_free(instance.gcHandle);
            }
        }
    }
    gameObjectScripts.clear();
    scriptClasses.clear();


    mono_domain_set(rootDomain, false);
    mono_domain_unload(appDomain);


    char domainName[] = "GameScriptDomain";
    appDomain = mono_domain_create_appdomain(domainName, nullptr);
    mono_domain_set(appDomain, true);


    RegisterInternalCalls();

 
    LoadScriptAssembly("./Scripts/GameScripts.dll");

    std::cout << "Script assembly reloaded" << std::endl;
}

std::vector<ScriptField> ScriptSystem::GetPublicFields(ScriptInstance& instance) {
    std::vector<ScriptField> fields;
    MonoClass* klass = instance.scriptClass->monoClass;
    void* iter = nullptr;
    while (MonoClassField* field = mono_class_get_fields(klass, &iter)) {
        if (mono_field_get_flags(field) & MONO_FIELD_ATTR_PUBLIC) {
            ScriptField scriptField;
            scriptField.name = mono_field_get_name(field);

            MonoType* type = mono_field_get_type(field);
            int monoType = mono_type_get_type(type);

            MonoObject* value = mono_field_get_value_object(appDomain, field, instance.instance);
            if (!value) continue;

            switch (monoType) {
            case MONO_TYPE_R4:
                scriptField.type = ScriptFieldType::Float;
                scriptField.value = *(float*)mono_object_unbox(value);
                break;
            case MONO_TYPE_I4:
                scriptField.type = ScriptFieldType::Int;
                scriptField.value = *(int*)mono_object_unbox(value);
                break;
            case MONO_TYPE_BOOLEAN:
                scriptField.type = ScriptFieldType::Bool;
                scriptField.value = *(bool*)mono_object_unbox(value);
                break;
            case MONO_TYPE_STRING:
                scriptField.type = ScriptFieldType::String;
                scriptField.value = mono_string_to_utf8((MonoString*)value);
                break;
            default:
                scriptField.type = ScriptFieldType::None;
                break;
            }

            if (scriptField.type != ScriptFieldType::None) {
                fields.push_back(scriptField);
            }
        }
    }
    return fields;
}

void ScriptSystem::SetPublicField(ScriptInstance& instance, const std::string& fieldName, const float& value) {
    MonoClass* klass = instance.scriptClass->monoClass;
    MonoClassField* field = mono_class_get_field_from_name(klass, fieldName.c_str());
    if (field) {
        mono_field_set_value(instance.instance, field, (void*)&value);
    }
}

void ScriptSystem::SetPublicField(ScriptInstance& instance, const std::string& fieldName, const int& value) {
    MonoClass* klass = instance.scriptClass->monoClass;
    MonoClassField* field = mono_class_get_field_from_name(klass, fieldName.c_str());
    if (field) {
        mono_field_set_value(instance.instance, field, (void*)&value);
    }
}

void ScriptSystem::SetPublicField(ScriptInstance& instance, const std::string& fieldName, const bool& value) {
    MonoClass* klass = instance.scriptClass->monoClass;
    MonoClassField* field = mono_class_get_field_from_name(klass, fieldName.c_str());
    if (field) {
        mono_field_set_value(instance.instance, field, (void*)&value);
    }
}

void ScriptSystem::SetPublicField(ScriptInstance& instance, const std::string& fieldName, const std::string& value) {
    MonoClass* klass = instance.scriptClass->monoClass;
    MonoClassField* field = mono_class_get_field_from_name(klass, fieldName.c_str());
    if (field) {
        MonoString* monoStr = mono_string_new(appDomain, value.c_str());
        mono_field_set_value(instance.instance, field, monoStr);
    }
}

void ScriptSystem::NotifyGameObjectDestroyed(GameObject* gameObject)
{
    if (!gameObject)
    {
        std::cerr << "[ScriptSystem] ERROR: NotifyGameObjectDestroyed called with null GameObject!\n";
        return;
    }

    std::cout << "[ScriptSystem] Notifying destruction of GameObject: " << gameObject->GetName() << std::endl;


    CallOnDestroy(gameObject);

    auto it = gameObjectScripts.find(gameObject);
    if (it != gameObjectScripts.end())
    {
       
        for (auto& scriptInstance : it->second)
        {
            if (scriptInstance.gcHandle != 0)
            {
                mono_gchandle_free(scriptInstance.gcHandle);
            }
        }
        gameObjectScripts.erase(it);
        std::cout << "[ScriptSystem] Removed references for GameObject: " << gameObject->GetName() << std::endl;
    }
    else
    {
        std::cout << "[ScriptSystem] No script references found for GameObject: " << gameObject->GetName() << std::endl;
    }
}