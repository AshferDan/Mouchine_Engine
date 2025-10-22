#pragma once
#include <iostream>
#include <cmath>
#include <string>

#include "Core.h"
#include "Factory.h"
#include "GameObjectSystem.h"
#include "GraphicsSystem.h"
#include "ImGuiSystem.h"
#include "DebugSystem.h"
#include "Transform.h"
#include "Collision.h"
#include "Collidercomponent.h"
#include "Physics.h"
#include "RenderComponent.h"
#include "Animator.hpp"
#include "ResourceManager.h"
#include "audio.h"
#include "InputManager.h"
#include "gameObject.h"


namespace API {
#pragma region Engine
    void Init_Engine(CoreEngine& engine, AudioEngine& audio);
    void StartGameLoop(CoreEngine& engine);
#pragma endregion

#pragma region Graphics
    void CreateShape(const std::string& name,
        RenderComponent::ShapeType type,
        float x, float y,
        float width, float height,
        vec4 color,
        CollisionLayer layer = CollisionLayer::Default,
        bool hasPhysics = false,
        bool isStatic = false,
        bool hasGravity = false);

    void CreateImage(const std::string& name,
        RenderComponent::ShapeType type,
        float x, float y,
        float width, float height,
        const std::string& texturePath);

    void CreateAnimatedImage(const std::string& name,
        float x, float y,
        float width, float height,
        const std::string& texturePath,
        int rows, int columns,
        int startFrame, int endFrame,
        float fps,
        CollisionLayer layer = CollisionLayer::Default,
        bool hasPhysics = false,
        bool isStatic = false,
        bool hasGravity = false);

#pragma endregion

#pragma region Physics
    void GetVelocity(const std::string&);
    void SetVelocity(const std::string& objectName, float vx, float vy);

    void IsStatic(const std::string&);
    void SetJumpVal(const std::string&, float);
    void SetGravBool(const std::string&); //set whether it have a Gravity
    void SetGravVal(const std::string&, float, float);//Setting the gravity amount
#pragma endregion

#pragma region Transform
    void SetPos(const std::string&,float,float);
    void SetRot(const std::string&,float,float);
    void SetScale(const std::string&,float,float);

    void GetRot(const std::string&);
    void GetPos(const std::string&);
    void GetScale(const std::string&);
#pragma endregion


    
        


    
}