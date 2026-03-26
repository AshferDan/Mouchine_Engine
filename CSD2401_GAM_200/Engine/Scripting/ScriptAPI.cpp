/* Start Header ************************************************************************/
/*!
\file       ScriptAPI.cpp
\author     Jonathan Fam Qian Wei [100% - 1,153 lines]
\par        Fam.q@digipen.edu
\date       Novenber, 7, 2025

Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*/
/* End Header **************************************************************************/

#include "ScriptAPI.h"
#include "LayerManager.h"
#include "ScriptingSystem.h"
#include <mono/jit/jit.h>
#include <mono/metadata/assembly.h>

namespace API
{
#pragma region Engine
    void Init_Engine(CoreEngine &engine, AudioEngine &audio)
    {
        engine.SetAudioEngine(&audio);
        std::map<std::string, double> g_systemTimings; // ms per system
        std::cout << "\n--- Initializing Engine ---" << std::endl;

        engine.Init();

        // Initialise Audio System
        std::cout << "\n--- Initialising Audio System ---" << std::endl;

        if (!audio.initialiseAudio())
        {
            std::cout << "Audio system failed to initialise, continuing without audio" << std::endl;
        }
        else
        {
            std::cout << "Audio system initialised successfully" << std::endl;
        }
        

        // Initialise Resource Manager
        std::cout << "\n--- Initialising Resource Manager ---" << std::endl;
        auto &resourceMgr = ResourceManager::Instance();
        resourceMgr.SetAudioEngine(&audio);

        // load audio files
        //resourceMgr.LoadAudio("bgm", "../Assets/audio/level_1.wav", true);            // looping
        //resourceMgr.LoadAudio("jump_sfx", "../Assets/audio/jump.wav", false);         // when jumping only
        //resourceMgr.LoadAudio("footstep_sfx", "../Assets/audio/footstep.wav", false); // footstep sound

        // load common fonts
        std::cout << "\n--- Loading Fonts ---" << std::endl;
        resourceMgr.LoadFont("default_font", "../Assets/PinyonScript-Regular.ttf", 48);
        resourceMgr.LoadFont("small_font", "../Assets/FrederickatheGreat-Regular.ttf", 24);
        resourceMgr.LoadFont("large_font", "../Assets/FrederickatheGreat-Regular.ttf", 64);

        // print loaded resources
        resourceMgr.PrintLoadedResources();
        std::cout << "\n--- Audio loaded, music will start in play mode ---" << std::endl;
    }

    void StartEngineLoop(CoreEngine &engine)
    {
        std::cout << "\n--- Starting Engine Gameloop ---" << std::endl;
        engine.GameLoop();
    }

#pragma endregion

#pragma region Creation

    void CreateShape(const std::string &name,
                     RenderComponent::ShapeType type,
                     float x, float y,
                     float width, float height,
                     vec4 color,
                     CollisionLayer layer,
                     bool hasPhysics,
                     bool isStatic,
                     bool hasGravity)
    {
        auto &factory = GameObjectFactory::Instance();
        GameObject *obj = factory.CreateShape(name, type, x, y, width, height, color, layer);
        if (hasPhysics && obj)
        {
            auto *physics = obj->AddComponent<PhysicsComponent>(1.0f, isStatic);
            physics->SetStatic(isStatic);

            if (hasGravity)
            {
                physics->SetGravity(true);
            }
        }
    }

    void CreateImage(const std::string &name,
                     RenderComponent::ShapeType type,
                     float x, float y,
                     float width, float height,
                     const std::string &texturePath)
    {
        auto &factory = GameObjectFactory::Instance();
        factory.CreateImage(name, type, x, y, width, height, texturePath.c_str());
    }

    void CreateAnimatedImage(const std::string &name,
                             float x, float y,
                             float width, float height,
                             const std::string &texturePath,
                             int rows, int columns,
                             int startRow, int startCol, int endRow, int endCol,
                             float fps,
                             CollisionLayer layer,
                             bool hasPhysics,
                             bool isStatic,
                             bool hasGravity)
    {
        auto &factory = GameObjectFactory::Instance();
        int startFrame = startRow * columns + startCol;
        int endFrame = endRow * columns + endCol;
        GameObject *obj = factory.CreateAnimatedImage(name, x, y, width, height, texturePath.c_str(),
                                                      rows, columns, startFrame, endFrame, fps, layer);

        if (hasPhysics && obj)
        {
            auto *physics = obj->AddComponent<PhysicsComponent>(1.0f, isStatic);
            physics->SetStatic(isStatic);

            if (hasGravity)
            {
                physics->SetGravity(true);
            }
        }
    }

    void DestroyByID(const EntityID id)
    {
        GameObjectFactory::Instance().QueueDestroyGameObject(id);
    }

    void DestroyByName(const std::string& name)
    {
        auto& factory = GameObjectFactory::Instance();
        GameObject* obj = factory.FindGameObject(name);
        if (obj)
            factory.QueueDestroyGameObject(obj->GetID());
    }
    
#pragma endregion

#pragma region Graphics
    vec4 GetColor(const std::string& objectName)
    {
        auto& factory = GameObjectFactory::Instance();
        GameObject* obj = factory.FindGameObject(objectName);
        if (obj)
        {
            auto* render = obj->GetComponent<RenderComponent>();
            if (render)
            {
                return render->GetColor();
            }
        }
        return vec4(0.0f, 0.0f, 0.0f, 1.0f); // Return black if not found
    }

    void SetShapeType(const std::string &objectName, RenderComponent::ShapeType newShape)
    {
        auto &factory = GameObjectFactory::Instance();

        GameObject *obj = factory.FindGameObject(objectName);

        if (obj)
        {
            auto *render = obj->GetComponent<RenderComponent>();
            if (render)
            {
                render->SetShapeType(newShape);
            }
        }
    }
    void SetTexture(const std::string &objectName, const char *filename)
    {
        auto &factory = GameObjectFactory::Instance();

        GameObject *obj = factory.FindGameObject(objectName);

        if (obj)
        {
            auto *render = obj->GetComponent<RenderComponent>();
            if (render)
            {
                GLuint newTextID = GraphicsSystem::LoadTexture(filename);
                render->SetTextureID(newTextID);
            }
        }
    }
    void setColor(const std::string &objectName, vec4 color)
    {
        auto &factory = GameObjectFactory::Instance();

        GameObject *obj = factory.FindGameObject(objectName);

        if (obj)
        {
            auto *render = obj->GetComponent<RenderComponent>();
            if (render)
            {
                render->SetColor(color);
            }
        }
    }
    void SetAlpha(const std::string &objectName, float val)
    {
        auto &factory = GameObjectFactory::Instance();

        GameObject *obj = factory.FindGameObject(objectName);

        if (obj)
        {
            auto *render = obj->GetComponent<RenderComponent>();
            if (render)
            {
                render->setAlpha(val);
            }
        }
    }

    void SetRenderLayer(const std::string &objectName, int layer)
    {
        auto &factory = GameObjectFactory::Instance();
        GameObject *obj = factory.FindGameObject(objectName);
        if (obj)
        {
            obj->SetGameLayer(layer);
        }
    }

    int GetRenderLayer(const std::string &objectName)
    {
        auto &factory = GameObjectFactory::Instance();
        GameObject *obj = factory.FindGameObject(objectName);
        if (obj)
        {
            return obj->GetGameLayer();
        }
        return 0;
    }

    void SetLayerVisible(int layer, bool visible)
    {
        LayerManager::SetLayerEnabled(layer, visible);
    }

    bool IsLayerVisible(int layer)
    {
        return LayerManager::IsLayerEnabled(layer);
    }

    void SetBackGround(char const *filename)
    {
        auto &factory = GameObjectFactory::Instance();
        GameObject *background = factory.FindGameObject("bgImage");

        if (background)
        {
            unsigned int newID = GraphicsSystem::LoadTexture(filename);
            auto *render = background->GetComponent<RenderComponent>();
            if (render)
            {
                render->SetTextureID(newID);
            }
        }
    }

    // --- EntityID overloads ---

    void GetColorByID(EntityID id, float* r, float* g, float* b, float* a)
    {
        auto& factory = GameObjectFactory::Instance();
        GameObject* obj = factory.GetGameObject(id);
        if (obj)
        {
            auto* render = obj->GetComponent<RenderComponent>();
            if (render)
            {
                vec4 c = render->GetColor();
                *r = c.x; *g = c.y; *b = c.z; *a = c.w;
                return;
            }
        }
        *r = 0.0f; *g = 0.0f; *b = 0.0f; *a = 1.0f;
    }

    int GetRenderLayerByID(EntityID id)
    {
        auto& factory = GameObjectFactory::Instance();
        GameObject* obj = factory.GetGameObject(id);
        if (obj) return obj->GetGameLayer();
        return 0;
    }

    void SetColorByID(EntityID id, vec4 color)
    {
        auto& factory = GameObjectFactory::Instance();
        GameObject* obj = factory.GetGameObject(id);
        if (obj)
        {
            auto* render = obj->GetComponent<RenderComponent>();
            if (render) render->SetColor(color);
        }
    }

    void SetAlphaByID(EntityID id, float alpha)
    {
        auto& factory = GameObjectFactory::Instance();
        GameObject* obj = factory.GetGameObject(id);
        if (obj)
        {
            auto* render = obj->GetComponent<RenderComponent>();
            if (render) render->setAlpha(alpha);
        }
    }

    void SetRenderLayerByID(EntityID id, int layer)
    {
        auto& factory = GameObjectFactory::Instance();
        GameObject* obj = factory.GetGameObject(id);
        if (obj) obj->SetGameLayer(layer);
    }

    void SetTextureByID(EntityID id, const char* filename)
    {
        auto& factory = GameObjectFactory::Instance();
        GameObject* obj = factory.GetGameObject(id);
        if (obj)
        {
            auto* render = obj->GetComponent<RenderComponent>();
            if (render) render->SetTextureID(GraphicsSystem::LoadTexture(filename));
        }
    }

#pragma endregion

#pragma region Physics
    void SetVelocity(const std::string &objectName, float vx, float vy)
    {

        auto &factory = GameObjectFactory::Instance();

        GameObject *obj = factory.FindGameObject(objectName);

        if (obj)
        {
            auto *physics = obj->GetComponent<PhysicsComponent>();
            if (physics)
            {
                physics->SetVelocity(vx, vy);
            }
        }
    }

    void SetJumpVal(const std::string &objectName, float val)
    {
        (void)val;
        auto &factory = GameObjectFactory::Instance();

        GameObject *obj = factory.FindGameObject(objectName);

        if (obj)
        {
            auto *physics = obj->GetComponent<PhysicsComponent>();
            if (physics)
            {
                physics->GetForcesManager().AddForce(ForceType::Jump);
            }
        }
    }

    void SetGravBool(const std::string &objectName, bool val)
    {
        auto &factory = GameObjectFactory::Instance();

        GameObject *obj = factory.FindGameObject(objectName);

        if (obj)
        {
            auto *physics = obj->GetComponent<PhysicsComponent>();
            if (physics)
            {
                physics->SetGravity(val);
            }
        }
    }


    void SetStatic(const std::string &objectName, bool val)
    {
        auto &factory = GameObjectFactory::Instance();

        GameObject *obj = factory.FindGameObject(objectName);

        if (obj)
        {
            auto *physics = obj->GetComponent<PhysicsComponent>();
            if (physics)
            {
                physics->SetStatic(val);
            }
        }
    }

    vec2 GetVelocity(const std::string& objectName)
    {
        auto& factory = GameObjectFactory::Instance();
        GameObject* obj = factory.FindGameObject(objectName);
        if (obj)
        {
            auto* physics = obj->GetComponent<PhysicsComponent>();
            if (physics)
            {
                return vec2(physics->velocity.x, physics->velocity.y);
            }
        }
        return vec2(0.0f, 0.0f); // Return a default value if not found
    }

    void SetFriction(const std::string& objectName, float val)
    {
        auto& factory = GameObjectFactory::Instance();

        GameObject* obj = factory.FindGameObject(objectName);

        if (obj)
        {
            auto* physics = obj->GetComponent<PhysicsComponent>();
            if (physics)
            {
                physics->SetFrictionCoefficient(val);
            }
        }
    }

    void SetMass(const std::string&objectName, float val)
    {
        auto& factory = GameObjectFactory::Instance();

        GameObject* obj = factory.FindGameObject(objectName);

        if (obj)
        {
            auto* physics = obj->GetComponent<PhysicsComponent>();
            if (physics)
            {
                physics->SetMass(val);
            }
        }
    }

    // --- EntityID overloads ---

    void GetVelocityByID(EntityID id, float* vx, float* vy)
    {
        auto& factory = GameObjectFactory::Instance();
        GameObject* obj = factory.GetGameObject(id);
        if (obj)
        {
            auto* physics = obj->GetComponent<PhysicsComponent>();
            if (physics) { *vx = physics->velocity.x; *vy = physics->velocity.y; return; }
        }
        *vx = 0.0f; *vy = 0.0f;
    }

    void SetVelocityByID(EntityID id, float x, float y)
    {
        auto& factory = GameObjectFactory::Instance();
        GameObject* obj = factory.GetGameObject(id);
        if (obj)
        {
            auto* physics = obj->GetComponent<PhysicsComponent>();
            if (physics) { physics->velocity.x = x; physics->velocity.y = y; }
        }
    }

#pragma endregion

#pragma region Transform
    void SetPos(const std::string &objectName, float x, float y)
    {
        auto &factory = GameObjectFactory::Instance();
        GameObject *obj = factory.FindGameObject(objectName);
        if (obj)
        {
            auto *transform = obj->GetComponent<TransformComponent>();
            if (transform)
            {
                transform->SetPosition(x, y);
            }
        }
    }

    void SetScale(const std::string& objectName, float x, float y)
    {
        auto& factory = GameObjectFactory::Instance();
        GameObject* obj = factory.FindGameObject(objectName);
        if (obj)
        {
            auto* transform = obj->GetComponent<TransformComponent>();
            if (transform)
            {
                transform->SetScale(x, y);
            }
        }
    }

    void SetScale(const std::string& objectName, float val)
    {
        auto& factory = GameObjectFactory::Instance();
        GameObject* obj = factory.FindGameObject(objectName);
        if (obj)
        {
            auto* transform = obj->GetComponent<TransformComponent>();
            if (transform)
            {
                transform->SetScale(val);
            }
        }
    }

    void SetRot(const std::string &objectName, float val)
    {
        auto &factory = GameObjectFactory::Instance();
        GameObject *obj = factory.FindGameObject(objectName);
        if (obj)
        {
            auto *transform = obj->GetComponent<TransformComponent>();
            if (transform)
            {
                transform->SetRotation(val);
            }
        }
    }
    void AddRot(const std::string &objectName, float val)
    {
        auto &factory = GameObjectFactory::Instance();
        GameObject *obj = factory.FindGameObject(objectName);
        if (obj)
        {
            auto *transform = obj->GetComponent<TransformComponent>();
            if (transform)
            {
                transform->Rotate(val);
            }
        }
    }



    void GetPosition(const std::string& objectName, float* x, float* y)
    {
        auto& factory = GameObjectFactory::Instance();
        GameObject* obj = factory.FindGameObject(objectName);
        if (obj)
        {
            auto* transform = obj->GetComponent<TransformComponent>();
            if (transform)
            {
                *x = transform->posX;
                *y = transform->posY;
                return;
            }
        }
        *x = 0.0f;
        *y = 0.0f;
    }

    float GetRotation(const std::string& objectName)
    {
        auto& factory = GameObjectFactory::Instance();
        GameObject* obj = factory.FindGameObject(objectName);
        if (obj)
        {
            auto* transform = obj->GetComponent<TransformComponent>();
            if (transform)
            {
                return transform->rotation;
            }
        }
        return 0.0f; // Return a default value if not found
    }

    void GetScale(const std::string& objectName, float* x, float* y)
    {
        auto& factory = GameObjectFactory::Instance();
        GameObject* obj = factory.FindGameObject(objectName);
        if (obj)
        {
            auto* transform = obj->GetComponent<TransformComponent>();
            if (transform)
            {
                *x = transform->scaleX;
                *y = transform->scaleY;
                return;
            }
        }
        *x = 0.0f;
        *y = 0.0f;
    }

    // --- EntityID overloads ---

    void GetPositionByID(EntityID id, float* x, float* y)
    {
        auto& factory = GameObjectFactory::Instance();
        GameObject* obj = factory.GetGameObject(id);
        if (obj)
        {
            auto* tf = obj->GetComponent<TransformComponent>();
            if (tf) { *x = tf->posX; *y = tf->posY; return; }
        }
        *x = 0.0f; *y = 0.0f;
    }

    void GetScaleByID(EntityID id, float* x, float* y)
    {
        auto& factory = GameObjectFactory::Instance();
        GameObject* obj = factory.GetGameObject(id);
        if (obj)
        {
            auto* tf = obj->GetComponent<TransformComponent>();
            if (tf) { *x = tf->scaleX; *y = tf->scaleY; return; }
        }
        *x = 0.0f; *y = 0.0f;
    }

    float GetRotationByID(EntityID id)
    {
        auto& factory = GameObjectFactory::Instance();
        GameObject* obj = factory.GetGameObject(id);
        if (obj)
        {
            auto* tf = obj->GetComponent<TransformComponent>();
            if (tf) return tf->rotation;
        }
        return 0.0f;
    }

    void SetPosByID(EntityID id, float x, float y)
    {
        auto& factory = GameObjectFactory::Instance();
        GameObject* obj = factory.GetGameObject(id);
        if (obj)
        {
            auto* tf = obj->GetComponent<TransformComponent>();
            if (tf) tf->SetPosition(x, y);
        }
    }

    void SetScaleByID(EntityID id, float x, float y)
    {
        auto& factory = GameObjectFactory::Instance();
        GameObject* obj = factory.GetGameObject(id);
        if (obj)
        {
            auto* tf = obj->GetComponent<TransformComponent>();
            if (tf) tf->SetScale(x, y);
        }
    }

    void SetRotByID(EntityID id, float rotation)
    {
        auto& factory = GameObjectFactory::Instance();
        GameObject* obj = factory.GetGameObject(id);
        if (obj)
        {
            auto* tf = obj->GetComponent<TransformComponent>();
            if (tf) tf->SetRotation(rotation);
        }
    }

#pragma endregion

#pragma region Text
    void SetText(const std::string& objectName, const std::string& text)
    {
        auto& factory = GameObjectFactory::Instance();
        GameObject* obj = factory.FindGameObject(objectName);
        if (obj)
        {
            auto* textComp = obj->GetComponent<TextComponent>();
            if (textComp)
            {
                textComp->SetText(text);
            }
        }
    }

    std::string GetText(const std::string& objectName)
    {
        auto& factory = GameObjectFactory::Instance();
        GameObject* obj = factory.FindGameObject(objectName);
        if (obj)
        {
            auto* textComp = obj->GetComponent<TextComponent>();
            if (textComp)
            {
                return textComp->GetText();
            }
        }
        return "";
    }

    void SetTextColor(const std::string& objectName, vec4 color)
    {
        auto& factory = GameObjectFactory::Instance();
        GameObject* obj = factory.FindGameObject(objectName);
        if (obj)
        {
            auto* textComp = obj->GetComponent<TextComponent>();
            if (textComp)
            {
                textComp->SetColor(color);
            }
        }
    }

    vec4 GetTextColor(const std::string& objectName)
    {
        auto& factory = GameObjectFactory::Instance();
        GameObject* obj = factory.FindGameObject(objectName);
        if (obj)
        {
            auto* textComp = obj->GetComponent<TextComponent>();
            if (textComp)
            {
                return textComp->GetColor();
            }
        }
        return vec4(1.0f, 1.0f, 1.0f, 1.0f);
    }

    void SetTextScale(const std::string& objectName, float scale)
    {
        auto& factory = GameObjectFactory::Instance();
        GameObject* obj = factory.FindGameObject(objectName);
        if (obj)
        {
            auto* textComp = obj->GetComponent<TextComponent>();
            if (textComp)
            {
                textComp->SetScale(scale);
            }
        }
    }

    float GetTextScale(const std::string& objectName)
    {
        auto& factory = GameObjectFactory::Instance();
        GameObject* obj = factory.FindGameObject(objectName);
        if (obj)
        {
            auto* textComp = obj->GetComponent<TextComponent>();
            if (textComp)
            {
                return textComp->GetScale();
            }
        }
        return 1.0f;
    }

    void SetTextScreenSpace(const std::string& objectName, bool screenSpace)
    {
        auto& factory = GameObjectFactory::Instance();
        GameObject* obj = factory.FindGameObject(objectName);
        if (obj)
        {
            auto* textComp = obj->GetComponent<TextComponent>();
            if (textComp)
            {
                textComp->SetScreenSpace(screenSpace);
            }
        }
    }

    bool GetTextScreenSpace(const std::string& objectName)
    {
        auto& factory = GameObjectFactory::Instance();
        GameObject* obj = factory.FindGameObject(objectName);
        if (obj)
        {
            auto* textComp = obj->GetComponent<TextComponent>();
            if (textComp)
            {
                return textComp->isScreenSpace();
            }
        }
        return true;
    }

    // --- EntityID overloads ---

    std::string GetTextByID(EntityID id)
    {
        auto& factory = GameObjectFactory::Instance();
        GameObject* obj = factory.GetGameObject(id);
        if (obj)
        {
            auto* textComp = obj->GetComponent<TextComponent>();
            if (textComp) return textComp->GetText();
        }
        return "";
    }

    void SetTextByID(EntityID id, const std::string& text)
    {
        auto& factory = GameObjectFactory::Instance();
        GameObject* obj = factory.GetGameObject(id);
        if (obj)
        {
            auto* textComp = obj->GetComponent<TextComponent>();
            if (textComp) textComp->SetText(text);
        }
    }

#pragma endregion

#pragma region Components

    void AddTransformComponent(const std::string &objectName)
    {
        auto &factory = GameObjectFactory::Instance();
        GameObject *obj = factory.FindGameObject(objectName);
        if (obj)
        {

            if (!(obj->GetComponent<TransformComponent>()))
            {
                obj->AddComponent<TransformComponent>();
            }
            else
            {
                throw std::runtime_error("Attempted to add duplicate TransformComponent to GameObject: " + objectName);
            }
        }
        else
        {
            throw std::runtime_error("GameObject not found: " + objectName);
        }
    }

    void AddRenderComponent(const std::string &objectName)
    {
        auto &factory = GameObjectFactory::Instance();
        GameObject *obj = factory.FindGameObject(objectName);
        if (obj)
        {
                
            if (!(obj->GetComponent<RenderComponent>()))
            {
                obj->AddComponent<RenderComponent>();
            }
            else
            {
                throw std::runtime_error("Attempted to add duplicate RenderComponent to GameObject: " + objectName);
            }
        }
    }

    void AddCollisionComponent(const std::string &objectName)
    {
        auto &factory = GameObjectFactory::Instance();
        GameObject *obj = factory.FindGameObject(objectName);
        if (obj)
        {
            auto transform = obj->GetComponent<TransformComponent>();
            if (!obj->GetComponent<ColliderComponent>() && transform)
            {
                obj->AddComponent<ColliderComponent>();
                auto collider = obj->GetComponent<ColliderComponent>();
                if (collider)
                {
                    float x{}, y{};
                    transform->GetScale(x, y);
                    collider->SetSize(x, y);
                }
            }
            else if (!obj->GetComponent<TransformComponent>())
            {
                throw std::runtime_error("This GameObject " + objectName + " do not have a TransformComponent");
            }
            else
            {
                throw std::runtime_error("Attempted to add duplicate ColliderComponent to GameObject: " + objectName);
            }
        }
    }

    void AddPhysicComponent(const std::string &objectName)
    {
        auto &factory = GameObjectFactory::Instance();
        GameObject *obj = factory.FindGameObject(objectName);
        if (obj)
        {

            if (!obj->GetComponent<PhysicsComponent>())
            {
                obj->AddComponent<PhysicsComponent>();
            }
            else
            {
                throw std::runtime_error("Attempted to add duplicate PhysicsComponent to GameObject: " + objectName);
            }
        }
    }

    void AddTextComponent(const std::string& objectName, const std::string& fontPath, unsigned int fontSize, const std::string& text, vec4 color, bool screenSpace)
    {
        auto& factory = GameObjectFactory::Instance();
        GameObject* obj = factory.FindGameObject(objectName);
        if (obj)
        {
            if (!obj->GetComponent<TextComponent>())
            {
                obj->AddComponent<TextComponent>(fontPath.c_str(), fontSize, text, color, screenSpace);
            }
            else
            {
                throw std::runtime_error("Attempted to add duplicate TextComponent to GameObject: " + objectName);
            }
        }
    }

    void SetComponentActive(const std::string& objectName, const std::string& componentName, bool active)
    {
        auto& factory = GameObjectFactory::Instance();
        GameObject* obj = factory.FindGameObject(objectName);
        if (obj)
        {
            if (componentName == "Transform")
            {
                if (auto* comp = obj->GetComponent<TransformComponent>()) comp->SetEnabled(active);
            }
            else if (componentName == "Render")
            {
                if (auto* comp = obj->GetComponent<RenderComponent>()) comp->SetEnabled(active);
            }
            else if (componentName == "Physics")
            {
                if (auto* comp = obj->GetComponent<PhysicsComponent>()) comp->SetEnabled(active);
            }
            else if (componentName == "Collider")
            {
                if (auto* comp = obj->GetComponent<ColliderComponent>()) comp->SetEnabled(active);
            }
            else if (componentName == "Text")
            {
                if (auto* comp = obj->GetComponent<TextComponent>()) comp->SetEnabled(active);
            }
            else
            {
                throw std::runtime_error("Attempted to set active state for unknown component: " + componentName);
            }
        }
    }

    void RemoveComponent(const std::string& objectName, const std::string& componentName)
    {
        auto& factory = GameObjectFactory::Instance();
        GameObject* obj = factory.FindGameObject(objectName);
        if (obj)
        {
            if (componentName == "Transform")
            {
                obj->RemoveComponent<TransformComponent>();
            }
            else if (componentName == "Render")
            {
                obj->RemoveComponent<RenderComponent>();
            }
            else if (componentName == "Physics")
            {
                obj->RemoveComponent<PhysicsComponent>();
            }
            else if (componentName == "Collider")
            {
                obj->RemoveComponent<ColliderComponent>();
            }
            else if (componentName == "Text")
            {
                obj->RemoveComponent<TextComponent>();
            }
            else
            {
                throw std::runtime_error("Attempted to remove unknown component: " + componentName);
            }
        }
    }
#pragma endregion

#pragma region Collision
    int GetCollisionLayer(const std::string& objectName)
    {
        auto& factory = GameObjectFactory::Instance();
        GameObject* obj = factory.FindGameObject(objectName);
        if (obj)
        {
            auto* collider = obj->GetComponent<ColliderComponent>();
            if (collider)
            {
                return static_cast<int>(collider->GetLayer());
            }
        }
        return -1; // Return -1 if not found
    }

    int GetCollisionShape(const std::string& objectName)
    {
        auto& factory = GameObjectFactory::Instance();
        GameObject* obj = factory.FindGameObject(objectName);
        if (obj)
        {
            auto* collider = obj->GetComponent<ColliderComponent>();
            if (collider)
            {
                return static_cast<int>(collider->GetType());
            }
        }
        return -1; // Return -1 if not found
    }

    void SetColisionShape(const std::string &objectName, ColliderComponent::Type newShape)
    {
        auto &factory = GameObjectFactory::Instance();
        GameObject *obj = factory.FindGameObject(objectName);

        if (obj)
        {
            auto *colli = obj->GetComponent<ColliderComponent>();
            if (colli)
            {
                colli->setColliShape(newShape);
            }
        }
    }
    void SetCollisionLayer(const std::string &objectName, CollisionLayer newlayer)
    {
        auto &factory = GameObjectFactory::Instance();

        GameObject *obj = factory.FindGameObject(objectName);

        if (obj)
        {
            auto *colli = obj->GetComponent<ColliderComponent>();
            if (colli)
            {
                colli->SetLayer(newlayer);
            }
        }
    }
    bool OnTriggerEnter(const std::string obj1_name,const std::string obj2_name)
    {
        auto& factory = GameObjectFactory::Instance();

        GameObject* obj1 = factory.FindGameObject(obj1_name);
        GameObject* obj2 = factory.FindGameObject(obj2_name);
        CollisionSystem* collisys = CORE->GetCollisionSystem();
        if (!obj1 || !obj2 || !collisys) return false;
        if (collisys->CollisionSystem::JustCollided(obj1, obj2)) {
            return true;
        }
        return false;
    }
    bool OnTriggerExit(const std::string obj1_name, const std::string obj2_name)
    {
        auto& factory = GameObjectFactory::Instance();

        GameObject* obj1 = factory.FindGameObject(obj1_name);
        GameObject* obj2 = factory.FindGameObject(obj2_name);
        CollisionSystem* collisys = CORE->GetCollisionSystem();
        if (!obj1 || !obj2 || !collisys) return false;
        if (collisys->CollisionSystem::JustStopped(obj1, obj2)) {
            return true;
        }
        return false;
    }
    bool OnTriggerStay(const std::string obj1_name, const std::string obj2_name)
    {
        auto& factory = GameObjectFactory::Instance();

        GameObject* obj1 = factory.FindGameObject(obj1_name);
        GameObject* obj2 = factory.FindGameObject(obj2_name);
        CollisionSystem* collisys = CORE->GetCollisionSystem();
        if (!obj1 || !obj2 || !collisys) return false;
        if (collisys->CollisionSystem::StillColliding(obj1, obj2)) {
            return true;
        }
        return false;
    }
    bool CheckColliderLayer(int layer1, int layer2)
    {
		return CollisionSystem::ShouldCollide(static_cast<CollisionLayer>(layer1), static_cast<CollisionLayer>(layer2));
	}

    bool IsCollidingWithLayer(const std::string& objectName, int targetLayer)
    {
        CollisionSystem* collisionSystem = CORE->GetCollisionSystem();
        if (!collisionSystem) return false;

        auto& factory = GameObjectFactory::Instance();
        GameObject* obj = factory.FindGameObject(objectName);
        if (!obj) return false;

        return collisionSystem->IsCollidingWithLayer(obj, static_cast<CollisionLayer>(targetLayer));
    }

bool Raycast(float x, float y, float dx, float dy, float distance, MonoString* selfName, MonoString** hitObjectName, float* hitDistance)
{
    CollisionSystem* collisionSystem = CORE->GetCollisionSystem();
    if (!collisionSystem)
    {
        return false;
    }

    auto& factory = GameObjectFactory::Instance();
    GameObject* self = nullptr;
    if (selfName)
    {
        self = factory.FindGameObject(mono_string_to_utf8(selfName));
    }

    RaycastHit hit;
    vec2 origin(x, y);
    vec2 direction(dx, dy);

    if (collisionSystem->Raycast(origin, direction, distance, hit, self))
    {
        if (hit.gameObject)
        {
            // Convert the hit object's name to a MonoString
            *hitObjectName = mono_string_new(mono_domain_get(), hit.gameObject->GetName().c_str());
            *hitDistance = hit.distance;
            return true;
        }
    }

    return false;
}



#pragma endregion

#pragma region Camera
    vec2 GetCameraPosition()
    {
        auto& cam = CameraManager::Instance();
        CameraComponent* camtemp = cam.GetActiveCamera();
        if (!camtemp) {
            return vec2(0.0f, 0.0f);
        }
        return camtemp->GetPosition();
    }

    float GetCameraZoom()
    {
        auto& cam = CameraManager::Instance();
        CameraComponent* camtemp = cam.GetActiveCamera();
        if (!camtemp) {
            return 0.0f;
        }
        return camtemp->getZoom();
    }

    int GetCameraMode()
    {
        auto& cam = CameraManager::Instance();
        CameraComponent* camtemp = cam.GetActiveCamera();
        if (!camtemp) {
            return -1;
        }
        return static_cast<int>(camtemp->GetMode());
    }


    void setCameraPos(vec2 pos)
    {
        auto& cam = CameraManager::Instance();
        CameraComponent* camtemp = cam.GetActiveCamera();
        if (!camtemp) {
            return;
        }

        camtemp->SetPosition(pos.x, pos.y);
    }


    void setCameraZoom(float val)
    {
        auto& cam = CameraManager::Instance();
        CameraComponent* camtemp = cam.GetActiveCamera();
        if (!camtemp) {
            return;
        }
        camtemp->SetZoom(val);
    }

    void setCameraMode(CameraComponent::CameraMode val)
    {
        auto& cam = CameraManager::Instance();
        CameraComponent* camtemp = cam.GetActiveCamera();
        if (!camtemp) {
            return;
        }
        camtemp->SetMode(val);
    }

    void CameraFlash(float duration, const vec4& color, bool fadeout, float cameraShakeStrength) {
        auto& cam = CameraManager::Instance();
        CameraManager::Instance().AddCameraTrauma(cameraShakeStrength);
        cam.FlashActiveCamera(duration, color, fadeout);
    }

    void SetCameraFollowOffset(float x, float y)
    {
        auto& cam = CameraManager::Instance();
        CameraComponent* camtemp = cam.GetActiveCamera();
        if (!camtemp) return;
        camtemp->SetFollowOffset(vec2(x, y));
    }

    vec2 GetCameraFollowOffset()
    {
        auto& cam = CameraManager::Instance();
        CameraComponent* camtemp = cam.GetActiveCamera();
        if (!camtemp) return vec2(0.0f, 0.0f);
        return camtemp->GetFollowOffset();
    }

    #pragma endregion

#pragma region Animator
    void PlayAnimation(const std::string& objectName, const std::string& animationName, bool isLooping)
    {
        auto& factory = GameObjectFactory::Instance();
        GameObject* obj = factory.FindGameObject(objectName);
        if (obj)
        {
            auto* spritesheet = obj->GetComponent<Spritesheet>();
            if (spritesheet)
            {
                spritesheet->PlayAnimation(animationName, isLooping);
            }
        }
    }

    void SetAnimationLooping(const std::string& objectName, bool isLooping)
    {
        auto& factory = GameObjectFactory::Instance();
        GameObject* obj = factory.FindGameObject(objectName);
        if (obj)
        {
            auto* spritesheet = obj->GetComponent<Spritesheet>();
            if (spritesheet)
            {
                spritesheet->SetLooping(isLooping);
            }
        }
    }

    bool IsAnimationLooping(const std::string& objectName)
    {
        auto& factory = GameObjectFactory::Instance();
        GameObject* obj = factory.FindGameObject(objectName);
        if (obj)
        {
            auto* spritesheet = obj->GetComponent<Spritesheet>();
            if (spritesheet)
            {
                return spritesheet->GetLooping();
            }
        }
        return false;
    }

    void AddAnimationToQueue(const std::string& objectName, const std::string& animationName)
    {
        auto& factory = GameObjectFactory::Instance();
        GameObject* obj = factory.FindGameObject(objectName);
        if (obj)
        {
            auto* spritesheet = obj->GetComponent<Spritesheet>();
            if (spritesheet)
            {
                spritesheet->AddToQueue(animationName);
            }
        }
    }

    void RegisterAnimation(const std::string& objectName, const std::string& animationName, const std::string& texturePath, int rows, int cols, int startRow, int startCol, int endRow, int endCol, float fps, bool pingPong, bool flipRows)
    {
        auto& factory = GameObjectFactory::Instance();
        GameObject* obj = factory.FindGameObject(objectName);
        if (obj)
        {
            auto* spritesheet = obj->GetComponent<Spritesheet>();
            if (spritesheet)
            {
                spritesheet->RegisterAnimation(animationName, texturePath, rows, cols, startRow, startCol, endRow, endCol, fps, pingPong, flipRows);
            }
        }
    }

    void SetAnimation(const std::string& objectName, int startFrame, int endFrame, float fps)
    {
        auto& factory = GameObjectFactory::Instance();
        GameObject* obj = factory.FindGameObject(objectName);
        if (obj)
        {
            auto* spritesheet = obj->GetComponent<Spritesheet>();
            if (spritesheet)
            {
                spritesheet->SetAnimation(startFrame, endFrame, fps);
            }
        }
    }

    std::string GetCurrentAnimationName(const std::string& objectName)
    {
        auto& factory = GameObjectFactory::Instance();
        GameObject* obj = factory.FindGameObject(objectName);
        if (obj)
        {
            auto* spritesheet = obj->GetComponent<Spritesheet>();
            if (spritesheet)
            {
                return spritesheet->GetCurrentAnimationName();
            }
        }
        return ""; // Return empty string if not found
    }

    void LoadNewSpritesheet(const std::string& objectName, const std::string& texturePath, int rows, int cols)
    {
        auto& factory = GameObjectFactory::Instance();
        GameObject* obj = factory.FindGameObject(objectName);
        if (obj)
        {
            auto* spritesheet = obj->GetComponent<Spritesheet>();
            if (spritesheet)
            {
                spritesheet->LoadNewSpritesheet(texturePath.c_str(), rows, cols);
            }
        }
    }

    void SetCurrentFrame(const std::string& objectName, int row, int col)
    {
        auto& factory = GameObjectFactory::Instance();
        GameObject* obj = factory.FindGameObject(objectName);
        if (obj)
        {
            auto* spritesheet = obj->GetComponent<Spritesheet>();
            if (spritesheet)
            {
                spritesheet->SetCurrentFrame(row, col);
            }
        }
    }

    int GetRows(const std::string& objectName)
    {
        auto& factory = GameObjectFactory::Instance();
        GameObject* obj = factory.FindGameObject(objectName);
        if (obj)
        {
            auto* spritesheet = obj->GetComponent<Spritesheet>();
            if (spritesheet)
            {
                return spritesheet->GetRows();
            }
        }
        return 0;
    }

    int GetCols(const std::string& objectName)
    {
        auto& factory = GameObjectFactory::Instance();
        GameObject* obj = factory.FindGameObject(objectName);
        if (obj)
        {
            auto* spritesheet = obj->GetComponent<Spritesheet>();
            if (spritesheet)
            {
                return spritesheet->GetCols();
            }
        }
        return 0;
    }

    int GetCurrentRow(const std::string& objectName)
    {
        auto& factory = GameObjectFactory::Instance();
        GameObject* obj = factory.FindGameObject(objectName);
        if (obj)
        {
            auto* spritesheet = obj->GetComponent<Spritesheet>();
            if (spritesheet)
            {
                return spritesheet->GetCurrentRow();
            }
        }
        return 0;
    }

    int GetCurrentCol(const std::string& objectName)
    {
        auto& factory = GameObjectFactory::Instance();
        GameObject* obj = factory.FindGameObject(objectName);
        if (obj)
        {
            auto* spritesheet = obj->GetComponent<Spritesheet>();
            if (spritesheet)
            {
                return spritesheet->GetCurrentCol();
            }
        }
        return 0;
    }

    // --- EntityID overloads ---

    std::string GetCurrentAnimationNameByID(EntityID id)
    {
        auto& factory = GameObjectFactory::Instance();
        GameObject* obj = factory.GetGameObject(id);
        if (obj)
        {
            auto* spritesheet = obj->GetComponent<Spritesheet>();
            if (spritesheet) return spritesheet->GetCurrentAnimationName();
        }
        return "";
    }

    int GetRowsByID(EntityID id)
    {
        auto& factory = GameObjectFactory::Instance();
        GameObject* obj = factory.GetGameObject(id);
        if (obj)
        {
            auto* spritesheet = obj->GetComponent<Spritesheet>();
            if (spritesheet) return spritesheet->GetRows();
        }
        return 0;
    }

    int GetColsByID(EntityID id)
    {
        auto& factory = GameObjectFactory::Instance();
        GameObject* obj = factory.GetGameObject(id);
        if (obj)
        {
            auto* spritesheet = obj->GetComponent<Spritesheet>();
            if (spritesheet) return spritesheet->GetCols();
        }
        return 0;
    }

    int GetCurrentRowByID(EntityID id)
    {
        auto& factory = GameObjectFactory::Instance();
        GameObject* obj = factory.GetGameObject(id);
        if (obj)
        {
            auto* spritesheet = obj->GetComponent<Spritesheet>();
            if (spritesheet) return spritesheet->GetCurrentRow();
        }
        return 0;
    }

    int GetCurrentColByID(EntityID id)
    {
        auto& factory = GameObjectFactory::Instance();
        GameObject* obj = factory.GetGameObject(id);
        if (obj)
        {
            auto* spritesheet = obj->GetComponent<Spritesheet>();
            if (spritesheet) return spritesheet->GetCurrentCol();
        }
        return 0;
    }

    void PlayAnimationByID(EntityID id, const std::string& animationName, bool isLooping)
    {
        auto& factory = GameObjectFactory::Instance();
        GameObject* obj = factory.GetGameObject(id);
        if (obj)
        {
            auto* spritesheet = obj->GetComponent<Spritesheet>();
            if (spritesheet) spritesheet->PlayAnimation(animationName, isLooping);
        }
    }

#pragma endregion

#pragma region Misc
    int RandomNumberGen(int max, int min)
    {
        int range = max - min + 1;
        int result = rand() % range + min;
        return result;
    }
    double GetDelta_Time()
    {
        return CORE->GetDeltaTime();
    }
    double GetFixedDelta_time()
    {
        return CORE->GetFixedDeltaTime();
    }
    double GetFramePerSecond() {
        return CORE->GetFPS();
    }
    float fLerp(float a, float b, float t)
    {
		return (a, b, t);
	}
    float fInverseLerp(float a, float b, float value)
    {
		return MathfInverseLerp(a, b, value);
	}
    float fClamp(float value, float min, float max)
    {
		return MathfClamp(value, min, max);
	}
    int iClamp(int value, int min, int max)
    {
		return MathiClamp(value, min, max);
	}
    std::string FindGameObjectByName(const std::string& name)
    {
        auto& factory = GameObjectFactory::Instance();
        GameObject* obj = factory.FindGameObject(name);
        if (obj && obj->GetID())
        {
            return std::to_string(obj->GetID());
        }
        return "";
    }

    std::string FindGameObjectByID(int id)
    {
        auto& factory = GameObjectFactory::Instance();
        GameObject* obj = factory.GetGameObject(id);
        if (obj)
        {
            return obj->GetName();
        }
        return "";
    }

    void DestroyGameObject(const std::string& name)
    {
        auto& factory = GameObjectFactory::Instance();
        GameObject* obj = factory.FindGameObject(name);
        if (obj)
        {
            factory.QueueDestroyGameObject(obj->GetID());
        }
    }

    int CountAllGameObjects()
    {
        return GameObjectFactory::Instance().CountAllGameObjects();
    }
#pragma endregion

#pragma region GameTimer
    float GetRemainingTime()
    {
        auto* cb = CORE->GetGameCallbacks();
        return cb ? cb->GetRemainingTime() : 0.0f;
    }

    float GetLevelLength()
    {
        auto* cb = CORE->GetGameCallbacks();
        return cb ? cb->GetLevelTimerLength() : 60.0f;
    }

    void SetLevelTimerLength(float length)
    {
        auto* cb = CORE->GetGameCallbacks();
        if (cb) cb->SetLevelTimerLength(length);
    }

    void AddTime(float amount)
    {
        auto* cb = CORE->GetGameCallbacks();
        if (cb) cb->AddTime(amount);
    }
#pragma endregion

#pragma region Audio

    void AddSound(const std::string name, const std::string filepath,
        bool loop, float volume,float pitch, std::string category) {
        SoundData newSound{ name,filepath,loop,volume,pitch,category };
        AudioManager* audioManager = CORE->GetAudioManager();
        if (audioManager)
        {
            audioManager->LoadSound(newSound);
            audioManager->AddSound(newSound);
        }
    }

    void PlaySound(const std::string soundName, const std::string channel)
    {
        AudioManager* audioManager = CORE->GetAudioManager();
        if (audioManager)
        {
            audioManager->PlaySound(soundName, channel);
        }

    }

    void StopSound(const std::string channelName)
    {
        AudioManager* audioManager = CORE->GetAudioManager();
        if (audioManager)
        {
            audioManager->StopSound(channelName);
        }
    }

    void StopAllSound()
    {
        AudioManager* audioManager = CORE->GetAudioManager();
        if (audioManager)
        {
            audioManager->StopAllSounds();
        }
    }

    void RemoveSound(const std::string soundName)
    {
        AudioManager* audioManager = CORE->GetAudioManager();
        if (audioManager)
        {
            audioManager->RemoveSound(soundName);
        }
    }

    void TriggerEvent(const std::string eventname)
    {
        AudioManager* audioManager = CORE->GetAudioManager();
        if (audioManager)
        {
            audioManager->TriggerEvent(eventname);
        }
    }

    void AddEventSound(const std::string eventName, const std::string soundName)
    {
        AudioManager* audioManager = CORE->GetAudioManager();
        if (audioManager)
        {
            audioManager->AddEventMapping(eventName, soundName);
        }
    }

    void SetVolume(const std::string soundName, float volume)
    {
        AudioManager* audioManager = CORE->GetAudioManager();
        if (audioManager)
        {
            audioManager->SetSoundVolume(soundName, volume);
        }
    }

    void SetCategoryVolume(const std::string CatName, float vol)
    {
        AudioManager* audioManager = CORE->GetAudioManager();
        if (audioManager)
        {
            audioManager->SetCategoryVolume(CatName, vol);
        }
    }

    void SetPitch(const std::string soundName, float pitch)
    {
        AudioManager* audioManager = CORE->GetAudioManager();
        if (audioManager)
        {
            audioManager->SetSoundPitch(soundName, pitch);
        }
    }

    void PauseSound(const std::string channelName, bool pause)
    {
        AudioManager* audioManager = CORE->GetAudioManager();
        if (audioManager)
        {
            AudioEngine* audioEngine = ResourceManager::Instance().GetAudioEngine();
            if (audioEngine)
            {
                audioEngine->pauseSound(channelName, pause);
            }
        }
    }

    void PauseAllSounds(bool pause)
    {
        AudioManager* audioManager = CORE->GetAudioManager();
        if (audioManager)
        {
            audioManager->PauseAllSounds(pause);
        }
    }

    void ToggleMasterMute()
    {
        AudioManager* audioManager = CORE->GetAudioManager();
        if (audioManager)
        {
            audioManager->ToggleMasterMute();
        }
    }

    void ToggleCategoryMute(const std::string category)
    {
        AudioManager* audioManager = CORE->GetAudioManager();
        if (audioManager)
        {
            audioManager->ToggleCategoryMute(category);
        }
    }

    bool IsMasterMuted()
    {
        AudioManager* audioManager = CORE->GetAudioManager();
        if (audioManager)
        {
            return audioManager->IsMuted();
        }
        return false;
    }

    bool IsCategoryMuted(const std::string category)
    {
        AudioManager* audioManager = CORE->GetAudioManager();
        if (audioManager)
        {
            return audioManager->IsCategoryMuted(category);
        }
        return false;
    }

    void FadeOutCategory(const std::string category, float duration)
    {
        AudioManager* audioManager = CORE->GetAudioManager();
        if (audioManager)
        {
            audioManager->FadeOutCategory(category, duration);
        }
    }

    void FadeInCategory(const std::string category, float duration, float targetVolume)
    {
        AudioManager* audioManager = CORE->GetAudioManager();
        if (audioManager)
        {
            audioManager->FadeInCategory(category, duration, targetVolume);
        }
    }

    void FadeOutAll(float duration)
    {
        AudioManager* audioManager = CORE->GetAudioManager();
        if (audioManager)
        {
            audioManager->FadeOutAll(duration);
        }
    }

    // 3D Audio / Spatial Sound
    void Set3DListenerPosition(float x, float y)
    {
        AudioManager* audioManager = CORE->GetAudioManager();
        if (audioManager)
        {
            audioManager->Set3DListenerPosition(x, y);
        }
    }

    void Set3DSoundPosition(const std::string channelName, float x, float y)
    {
        AudioManager* audioManager = CORE->GetAudioManager();
        if (audioManager)
        {
            audioManager->Set3DSoundPosition(channelName, x, y);
        }
    }

    void Load3DSound(const std::string name, const std::string filepath, bool loop,
                     float minDistance, float maxDistance, const std::string category,
                     float volume, float pitch)
    {
        AudioManager* audioManager = CORE->GetAudioManager();
        if (audioManager)
        {
            audioManager->Load3DSound(name, filepath, loop, minDistance, maxDistance, category, volume, pitch);
        }
    }
#pragma endregion

#pragma region Timer
    TimerID WaitForSeconds(float duration, TimerCallback callback)
    {
        TimerSystem* ts = CORE->GetTimerSystem();
        if (ts)
        {
            return ts->AddTimer(duration, callback, false, "WaitForSeconds");
        }
        return 0;
    }

    TimerID Delay(float duration, TimerCallback callback)
    {
        return WaitForSeconds(duration, callback);
    }

    TimerID InvokeRepeating(float interval, TimerCallback callback)
    {
        TimerSystem* ts = CORE->GetTimerSystem();
        if (ts)
        {
            return ts->AddTimer(interval, callback, true, "InvokeRepeating");
        }
        return 0;
    }

    TimerID Invoke(float delay, void (*functionToCall)())
    {
        TimerSystem* ts = CORE->GetTimerSystem();
        if (ts)
        {
            return ts->AddTimer(delay, functionToCall, false, "Invoke");
        }
        return 0;
    }

    TimerID Schedule(float delay, std::function<void()> callback)
    {
        TimerSystem* ts = CORE->GetTimerSystem();
        if (ts)
        {
            return ts->AddTimer(delay, callback, false, "Schedule");
        }
        return 0;
    }

    void CancelTimer(TimerID id)
    {
        TimerSystem* ts = CORE->GetTimerSystem();
        if (ts) ts->CancelTimer(id);
    }

    void CancelTimerByName(std::string name)
    {
        TimerSystem* ts = CORE->GetTimerSystem();
        if (ts) ts->CancelTimerByName(name);
    }

    void PauseTimer(TimerID id)
    {
        TimerSystem* ts = CORE->GetTimerSystem();
        if (ts) ts->PauseTimer(id);
    }

    void ResumeTimer(TimerID id)
    {
        TimerSystem* ts = CORE->GetTimerSystem();
        if (ts) ts->ResumeTimer(id);
    }
#pragma endregion

#pragma region Scripting
    bool AttachScript(const std::string& objectName, const std::string& scriptClassName)
    {
        auto& factory = GameObjectFactory::Instance();
        GameObject* obj = factory.FindGameObject(objectName);
        if (obj)
        {
            ScriptSystem* scriptSystem = CORE->GetScriptSystem();
            if (scriptSystem)
            {
                return scriptSystem->AttachScript(obj, scriptClassName);
            }
        }
        return false;
    }

    std::string GetCollidingObjectNameOnLayer(const std::string& objectName, int targetLayer)
    {
        GameObject* obj = GameObjectFactory::Instance().FindGameObject(objectName);
        CollisionSystem* cs = CORE ? CORE->GetCollisionSystem() : nullptr;
        if (!obj || !cs) return "";
        return cs->GetCollidingObjectNameOnLayer(obj, static_cast<CollisionLayer>(targetLayer));
    }

    int GetCollidingObjectIDOnLayer(const std::string& objectName, int targetLayer)
    {
        GameObject* obj = GameObjectFactory::Instance().FindGameObject(objectName);
        CollisionSystem* cs = CORE ? CORE->GetCollisionSystem() : nullptr;
        if (!obj || !cs) return 0;
        return static_cast<int>(cs->GetCollidingObjectIDOnLayer(obj, static_cast<CollisionLayer>(targetLayer)));
    }

    // --- find a MonoObject instance + MonoClassField by names ---
    static bool FindScriptField(const std::string& objectName,
                                const std::string& scriptClass,
                                const std::string& fieldName,
                                MonoObject*& outInstance,
                                MonoClassField*& outField)
    {
        ScriptSystem* ss = CORE ? CORE->GetScriptSystem() : nullptr;
        if (!ss) return false;
        GameObject* obj = GameObjectFactory::Instance().FindGameObject(objectName);
        if (!obj) return false;
        auto* instances = ss->GetScriptInstances(obj);
        if (!instances) return false;
        for (auto& inst : *instances)
        {
            if (!inst.instance) continue;
            MonoClass* klass = mono_object_get_class(inst.instance);
            if (scriptClass != mono_class_get_name(klass)) continue;
            MonoClassField* field = mono_class_get_field_from_name(klass, fieldName.c_str());
            if (!field) return false;
            outInstance = inst.instance;
            outField    = field;
            return true;
        }
        return false;
    }

    // --- Getters ---
    float GetFloatField(const std::string& objectName, const std::string& scriptClass, const std::string& fieldName)
    {
        MonoObject* inst = nullptr; MonoClassField* field = nullptr;
        if (!FindScriptField(objectName, scriptClass, fieldName, inst, field)) return 0.f;
        float val = 0.f;
        mono_field_get_value(inst, field, &val);
        return val;
    }

    int GetIntField(const std::string& objectName, const std::string& scriptClass, const std::string& fieldName)
    {
        MonoObject* inst = nullptr; MonoClassField* field = nullptr;
        if (!FindScriptField(objectName, scriptClass, fieldName, inst, field)) return 0;
        int val = 0;
        mono_field_get_value(inst, field, &val);
        return val;
    }

    bool GetBoolField(const std::string& objectName, const std::string& scriptClass, const std::string& fieldName)
    {
        MonoObject* inst = nullptr; MonoClassField* field = nullptr;
        if (!FindScriptField(objectName, scriptClass, fieldName, inst, field)) return false;
        bool val = false;
        mono_field_get_value(inst, field, &val);
        return val;
    }

    std::string GetStringField(const std::string& objectName, const std::string& scriptClass, const std::string& fieldName)
    {
        MonoObject* inst = nullptr; MonoClassField* field = nullptr;
        if (!FindScriptField(objectName, scriptClass, fieldName, inst, field)) return "";
        MonoString* monoStr = nullptr;
        mono_field_get_value(inst, field, &monoStr);
        if (!monoStr) return "";
        char* raw = mono_string_to_utf8(monoStr);
        std::string result(raw);
        mono_free(raw);
        return result;
    }

    // --- Setters ---
    void SetFloatField(const std::string& objectName, const std::string& scriptClass, const std::string& fieldName, float value)
    {
        MonoObject* inst = nullptr; MonoClassField* field = nullptr;
        if (!FindScriptField(objectName, scriptClass, fieldName, inst, field)) return;
        mono_field_set_value(inst, field, &value);
    }

    void SetIntField(const std::string& objectName, const std::string& scriptClass, const std::string& fieldName, int value)
    {
        MonoObject* inst = nullptr; MonoClassField* field = nullptr;
        if (!FindScriptField(objectName, scriptClass, fieldName, inst, field)) return;
        mono_field_set_value(inst, field, &value);
    }

    void SetBoolField(const std::string& objectName, const std::string& scriptClass, const std::string& fieldName, bool value)
    {
        MonoObject* inst = nullptr; MonoClassField* field = nullptr;
        if (!FindScriptField(objectName, scriptClass, fieldName, inst, field)) return;
        mono_field_set_value(inst, field, &value);
    }

    void SetStringField(const std::string& objectName, const std::string& scriptClass, const std::string& fieldName, const std::string& value)
    {
        MonoObject* inst = nullptr; MonoClassField* field = nullptr;
        if (!FindScriptField(objectName, scriptClass, fieldName, inst, field)) return;
        MonoString* monoStr = mono_string_new(mono_domain_get(), value.c_str());
        mono_field_set_value(inst, field, monoStr);
    }
#pragma endregion

#pragma region Input
    bool IsKeyPressed(int key)
    {
        InputManager* input = CORE->GetInputManager();
        if (input)
        {
            return input->IsKeyPressed(key);
        }
        return false;
    }

    bool IsKeyReleased(int key)
    {
        InputManager* input = CORE->GetInputManager();
        if (input)
        {
            return input->IsKeyReleased(key);
        }
        return false;
    }

    bool IsKeyHeld(int key)
    {
        InputManager* input = CORE->GetInputManager();
        if (input)
        {
            return input->IsKeyHeld(key);
        }
        return false;
    }

    bool IsMouseButtonPressed(int button)
    {
        InputManager* input = CORE->GetInputManager();
        if (input)
        {
            return input->IsMouseButtonPressed(button);
        }
        return false;
    }

    bool IsMouseButtonReleased(int button)
    {
        InputManager* input = CORE->GetInputManager();
        if (input)
        {
            return input->IsMouseButtonReleased(button);
        }
        return false;
    }

    bool IsMouseButtonHeld(int button)
    {
        InputManager* input = CORE->GetInputManager();
        if (input)
        {
            return input->IsMouseButtonHeld(button);
        }
        return false;
    }

    float GetMouseX()
    {
        InputManager* input = CORE->GetInputManager();
        if (input) return (float)input->GetMouseX();
        return 0.0f;
    }

    float GetMouseY()
    {
        InputManager* input = CORE->GetInputManager();
        if (input) return (float)input->GetMouseY();
        return 0.0f;
    }

    float GetMouseWorldX()
    {
        // InputManager static accessor from your previous code
        return (float)InputManager::GetMouseWorldX();
    }

    float GetMouseWorldY()
    {
        return (float)InputManager::GetMouseWorldY();
    }
#pragma endregion

#pragma region Particles
    void AddParticleEmitterComponent(const std::string& objectName, int maxParticles)
    {
        auto& factory = GameObjectFactory::Instance();
        GameObject* obj = factory.FindGameObject(objectName);
        if (obj)
        {
            if (!obj->GetComponent<ParticleEmitterComponent>())
            {
                obj->AddComponent<ParticleEmitterComponent>(maxParticles);
            }
            else
            {
                throw std::runtime_error("Attempted to add duplicate ParticleEmitterComponent to GameObject: " + objectName);
            }
        }
    }

    void Particles_StartEmission(const std::string& objectName)
    {
        auto& factory = GameObjectFactory::Instance();
        GameObject* obj = factory.FindGameObject(objectName);
        if (obj)
        {
            auto* emitter = obj->GetComponent<ParticleEmitterComponent>();
            if (emitter)
            {
                emitter->StartEmission();
            }
        }
    }

    void Particles_StopEmission(const std::string& objectName)
    {
        auto& factory = GameObjectFactory::Instance();
        GameObject* obj = factory.FindGameObject(objectName);
        if (obj)
        {
            auto* emitter = obj->GetComponent<ParticleEmitterComponent>();
            if (emitter)
            {
                emitter->StopEmission();
            }
        }
    }

    void Particles_Burst(const std::string& objectName, int count)
    {
        auto& factory = GameObjectFactory::Instance();
        GameObject* obj = factory.FindGameObject(objectName);
        if (obj)
        {
            auto* emitter = obj->GetComponent<ParticleEmitterComponent>();
            if (emitter)
            {
                emitter->Burst(count);
            }
        }
    }

    void Particles_Clear(const std::string& objectName)
    {
        auto& factory = GameObjectFactory::Instance();
        GameObject* obj = factory.FindGameObject(objectName);
        if (obj)
        {
            auto* emitter = obj->GetComponent<ParticleEmitterComponent>();
            if (emitter)
            {
                emitter->Clear();
            }
        }
    }

    bool Particles_IsEmitting(const std::string& objectName)
    {
        auto& factory = GameObjectFactory::Instance();
        GameObject* obj = factory.FindGameObject(objectName);
        if (obj)
        {
            auto* emitter = obj->GetComponent<ParticleEmitterComponent>();
            if (emitter)
            {
                return emitter->IsEmitting();
            }
        }
        return false;
    }

    int Particles_GetActiveCount(const std::string& objectName)
    {
        auto& factory = GameObjectFactory::Instance();
        GameObject* obj = factory.FindGameObject(objectName);
        if (obj)
        {
            auto* emitter = obj->GetComponent<ParticleEmitterComponent>();
            if (emitter)
            {
                return emitter->GetActiveParticleCount();
            }
        }
        return 0;
    }

    int Particles_GetMaxParticles(const std::string& objectName)
    {
        auto& factory = GameObjectFactory::Instance();
        GameObject* obj = factory.FindGameObject(objectName);
        if (obj)
        {
            auto* emitter = obj->GetComponent<ParticleEmitterComponent>();
            if (emitter)
            {
                return emitter->GetMaxParticles();
            }
        }
        return 0;
    }

    void Particles_SetEmissionRate(const std::string& objectName, float rate)
    {
        auto& factory = GameObjectFactory::Instance();
        GameObject* obj = factory.FindGameObject(objectName);
        if (obj)
        {
            auto* emitter = obj->GetComponent<ParticleEmitterComponent>();
            if (emitter)
            {
                emitter->SetEmissionRate(rate);
            }
        }
    }

    void Particles_SetMaxParticles(const std::string& objectName, int max)
    {
        auto& factory = GameObjectFactory::Instance();
        GameObject* obj = factory.FindGameObject(objectName);
        if (obj)
        {
            auto* emitter = obj->GetComponent<ParticleEmitterComponent>();
            if (emitter)
            {
                emitter->SetMaxParticles(max);
            }
        }
    }

    void Particles_SetDuration(const std::string& objectName, float seconds)
    {
        auto& factory = GameObjectFactory::Instance();
        GameObject* obj = factory.FindGameObject(objectName);
        if (obj)
        {
            auto* emitter = obj->GetComponent<ParticleEmitterComponent>();
            if (emitter)
            {
                emitter->SetDuration(seconds);
            }
        }
    }

    void Particles_SetLoop(const std::string& objectName, bool shouldLoop)
    {
        auto& factory = GameObjectFactory::Instance();
        GameObject* obj = factory.FindGameObject(objectName);
        if (obj)
        {
            auto* emitter = obj->GetComponent<ParticleEmitterComponent>();
            if (emitter)
            {
                emitter->SetLoop(shouldLoop);
            }
        }
    }

    void Particles_SetLifetime(const std::string& objectName, float seconds, float variation)
    {
        auto& factory = GameObjectFactory::Instance();
        GameObject* obj = factory.FindGameObject(objectName);
        if (obj)
        {
            auto* emitter = obj->GetComponent<ParticleEmitterComponent>();
            if (emitter)
            {
                emitter->SetLifetime(seconds, variation);
            }
        }
    }

    void Particles_SetSpawnOffset(const std::string& objectName, float x, float y)
    {
        auto& factory = GameObjectFactory::Instance();
        GameObject* obj = factory.FindGameObject(objectName);
        if (obj)
        {
            auto* emitter = obj->GetComponent<ParticleEmitterComponent>();
            if (emitter)
            {
                emitter->SetSpawnOffset(vec2(x, y));
            }
        }
    }

    void Particles_SetSpawnRadius(const std::string& objectName, float radius)
    {
        auto& factory = GameObjectFactory::Instance();
        GameObject* obj = factory.FindGameObject(objectName);
        if (obj)
        {
            auto* emitter = obj->GetComponent<ParticleEmitterComponent>();
            if (emitter)
            {
                emitter->SetSpawnRadius(radius);
            }
        }
    }

    void Particles_SetSpawnBox(const std::string& objectName, float halfWidth, float halfHeight)
    {
        auto& factory = GameObjectFactory::Instance();
        GameObject* obj = factory.FindGameObject(objectName);
        if (obj)
        {
            auto* emitter = obj->GetComponent<ParticleEmitterComponent>();
            if (emitter)
            {
                emitter->SetSpawnBox(vec2(halfWidth, halfHeight));
            }
        }
    }

    void Particles_SetVelocity(const std::string& objectName, float dirX, float dirY, float magnitude, float variation)
    {
        auto& factory = GameObjectFactory::Instance();
        GameObject* obj = factory.FindGameObject(objectName);
        if (obj)
        {
            auto* emitter = obj->GetComponent<ParticleEmitterComponent>();
            if (emitter)
            {
                emitter->SetVelocity(vec2(dirX, dirY), magnitude, variation);
            }
        }
    }

    void Particles_SetAngleSpread(const std::string& objectName, float degrees)
    {
        auto& factory = GameObjectFactory::Instance();
        GameObject* obj = factory.FindGameObject(objectName);
        if (obj)
        {
            auto* emitter = obj->GetComponent<ParticleEmitterComponent>();
            if (emitter)
            {
                emitter->SetAngleSpread(degrees);
            }
        }
    }

    void Particles_SetGravity(const std::string& objectName, float gravX, float gravY)
    {
        auto& factory = GameObjectFactory::Instance();
        GameObject* obj = factory.FindGameObject(objectName);
        if (obj)
        {
            auto* emitter = obj->GetComponent<ParticleEmitterComponent>();
            if (emitter)
            {
                emitter->SetGravity(vec2(gravX, gravY));
            }
        }
    }

    void Particles_SetDrag(const std::string& objectName, float drag)
    {
        auto& factory = GameObjectFactory::Instance();
        GameObject* obj = factory.FindGameObject(objectName);
        if (obj)
        {
            auto* emitter = obj->GetComponent<ParticleEmitterComponent>();
            if (emitter)
            {
                emitter->SetDrag(drag);
            }
        }
    }

    void Particles_SetStartColor(const std::string& objectName, float r, float g, float b, float a)
    {
        auto& factory = GameObjectFactory::Instance();
        GameObject* obj = factory.FindGameObject(objectName);
        if (obj)
        {
            auto* emitter = obj->GetComponent<ParticleEmitterComponent>();
            if (emitter)
            {
                emitter->SetStartColor(vec4(r, g, b, a));
            }
        }
    }

    void Particles_SetEndColor(const std::string& objectName, float r, float g, float b, float a)
    {
        auto& factory = GameObjectFactory::Instance();
        GameObject* obj = factory.FindGameObject(objectName);
        if (obj)
        {
            auto* emitter = obj->GetComponent<ParticleEmitterComponent>();
            if (emitter)
            {
                emitter->SetEndColor(vec4(r, g, b, a));
            }
        }
    }

    void Particles_SetStartSize(const std::string& objectName, float size)
    {
        auto& factory = GameObjectFactory::Instance();
        GameObject* obj = factory.FindGameObject(objectName);
        if (obj)
        {
            auto* emitter = obj->GetComponent<ParticleEmitterComponent>();
            if (emitter)
            {
                emitter->SetStartSize(size);
            }
        }
    }

    void Particles_SetEndSize(const std::string& objectName, float size)
    {
        auto& factory = GameObjectFactory::Instance();
        GameObject* obj = factory.FindGameObject(objectName);
        if (obj)
        {
            auto* emitter = obj->GetComponent<ParticleEmitterComponent>();
            if (emitter)
            {
                emitter->SetEndSize(size);
            }
        }
    }

    void Particles_SetLayer(const std::string& objectName, int layer)
    {
        auto& factory = GameObjectFactory::Instance();
        GameObject* obj = factory.FindGameObject(objectName);
        if (obj)
        {
            auto* emitter = obj->GetComponent<ParticleEmitterComponent>();
            if (emitter)
            {
                emitter->SetLayer(layer);
            }
        }
    }

    void Particles_SetFadeIn(const std::string& objectName, float duration)
    {
        auto& factory = GameObjectFactory::Instance();
        GameObject* obj = factory.FindGameObject(objectName);
        if (obj)
        {
            auto* emitter = obj->GetComponent<ParticleEmitterComponent>();
            if (emitter)
            {
                emitter->SetFadeIn(duration);
            }
        }
    }

    void Particles_SetFadeOut(const std::string& objectName, float duration)
    {
        auto& factory = GameObjectFactory::Instance();
        GameObject* obj = factory.FindGameObject(objectName);
        if (obj)
        {
            auto* emitter = obj->GetComponent<ParticleEmitterComponent>();
            if (emitter)
            {
                emitter->SetFadeOut(duration);
            }
        }
    }

    void Particles_SetStartRotation(const std::string& objectName, float degrees, float variation)
    {
        auto& factory = GameObjectFactory::Instance();
        GameObject* obj = factory.FindGameObject(objectName);
        if (obj)
        {
            auto* emitter = obj->GetComponent<ParticleEmitterComponent>();
            if (emitter)
            {
                emitter->SetStartRotation(degrees, variation);
            }
        }
    }

    void Particles_SetAngularVelocity(const std::string& objectName, float degreesPerSec, float variation)
    {
        auto& factory = GameObjectFactory::Instance();
        GameObject* obj = factory.FindGameObject(objectName);
        if (obj)
        {
            auto* emitter = obj->GetComponent<ParticleEmitterComponent>();
            if (emitter)
            {
                emitter->SetAngularVelocity(degreesPerSec, variation);
            }
        }
    }

    void Particles_SetTexture(const std::string& objectName, const char* texturePath)
    {
        auto& factory = GameObjectFactory::Instance();
        GameObject* obj = factory.FindGameObject(objectName);
        if (obj)
        {
            auto* emitter = obj->GetComponent<ParticleEmitterComponent>();
            if (emitter)
            {
                GLuint texID = GraphicsSystem::LoadTexture(texturePath);
                emitter->SetTexture(texID);
            }
        }
    }

    void Particles_ConfigureDashTrail(const std::string& objectName)
    {
        auto& factory = GameObjectFactory::Instance();
        GameObject* obj = factory.FindGameObject(objectName);
        if (obj)
        {
            auto* emitter = obj->GetComponent<ParticleEmitterComponent>();
            if (emitter)
            {
                emitter->ConfigureDashTrail();
            }
        }
    }

    void Particles_ConfigureJumpDust(const std::string& objectName)
    {
        auto& factory = GameObjectFactory::Instance();
        GameObject* obj = factory.FindGameObject(objectName);
        if (obj)
        {
            auto* emitter = obj->GetComponent<ParticleEmitterComponent>();
            if (emitter)
            {
                emitter->ConfigureJumpDust();
            }
        }
    }

    void Particles_ConfigureWallSlideParticles(const std::string& objectName)
    {
        auto& factory = GameObjectFactory::Instance();
        GameObject* obj = factory.FindGameObject(objectName);
        if (obj)
        {
            auto* emitter = obj->GetComponent<ParticleEmitterComponent>();
            if (emitter)
            {
                emitter->ConfigureWallSlideParticles();
            }
        }
    }

    void Particles_ConfigureSpeedLines(const std::string& objectName)
    {
        auto& factory = GameObjectFactory::Instance();
        GameObject* obj = factory.FindGameObject(objectName);
        if (obj)
        {
            auto* emitter = obj->GetComponent<ParticleEmitterComponent>();
            if (emitter)
            {
                emitter->ConfigureSpeedLines();
            }
        }
    }

    void Particles_ConfigureDeathExplosion(const std::string& objectName)
    {
        auto& factory = GameObjectFactory::Instance();
        GameObject* obj = factory.FindGameObject(objectName);
        if (obj)
        {
            auto* emitter = obj->GetComponent<ParticleEmitterComponent>();
            if (emitter)
            {
                emitter->ConfigureDeathExplosion();
            }
        }
    }
#pragma endregion

#pragma region Ghost/Recording
    namespace Ghost
    {
        void Configure(const std::string& liveTarget, const std::string& ghostTarget,
                       const std::string& liveVfx, const std::string& ghostVfx)
        {
            if (CORE && CORE->GetGhostSystem())
                CORE->GetGhostSystem()->Configure(liveTarget, ghostTarget, liveVfx, ghostVfx);
        }

        void StartRecording(const std::string& targetName)
        {
            if (CORE && CORE->GetGhostSystem())
                CORE->GetGhostSystem()->StartRecording(targetName);
        }

        void StopRecording()
        {
            if (CORE && CORE->GetGhostSystem())
                CORE->GetGhostSystem()->StopRecording();
        }

        bool SaveBestRun(const std::string& levelName, float finishTime)
        {
            if (CORE && CORE->GetGhostSystem())
                return CORE->GetGhostSystem()->SaveBestRun(levelName, finishTime);
            return false;
        }

        void StartPlayback(const std::string& ghostObjectName)
        {
            if (CORE && CORE->GetGhostSystem())
                CORE->GetGhostSystem()->StartPlayback(ghostObjectName);
        }

        void StopPlayback()
        {
            if (CORE && CORE->GetGhostSystem())
                CORE->GetGhostSystem()->StopPlayback();
        }

        bool LoadRecording(const std::string& levelName)
        {
            if (CORE && CORE->GetGhostSystem())
                return CORE->GetGhostSystem()->LoadRecording(levelName);
            return false;
        }

        bool IsGhostAvailable()
        {
            if (CORE && CORE->GetGhostSystem())
                return CORE->GetGhostSystem()->IsGhostAvailable();
            return false;
        }

        float GetBestRunTime()
        {
            if (CORE && CORE->GetGhostSystem())
                return CORE->GetGhostSystem()->GetBestRunTime();
            return FLT_MAX;
        }

    }
#pragma endregion

}