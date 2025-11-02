/* Start Header ************************************************************************/
/*!
\file       factory.cpp
\author     Jonathan Fam Qian Wei [100% - 392 lines]
\par        Fam.q@digipen.edu
\date       October, 2, 2025

Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*/
/* End Header **************************************************************************/

#include "Factory.h"

/*
    /brief
    Creation of the Gameobjects. it the single argument constructor create and
    pushes the gameobject id in to the gameobject system.
*/
GameObject* GameObjectFactory::CreateGameObject(const std::string& name)
{
    EntityID id = nextEntityID++;
    auto gameObject = std::make_unique<GameObject>(id, name);
    GameObject* ptr = gameObject.get();

    entityToIndex[id] = gameObjects.size();
    gameObjects.push_back(std::move(gameObject));

    return ptr;
}
/*
    /brief
     This is the blueprint for the player creation where
     it has 2 overloads with the sprite and the color,
     color overload is for the creating the shape and the sprite is for the texture.
     in the future will create a hidden variable for player as the camera and inputs shld be on the player gameobject only.

*/

GameObject* GameObjectFactory::CreatePlayer(const std::string& name, RenderComponent::ShapeType shape, float x, float y, float w, float h, const vec4 color, CollisionLayer)
{
    GameObject* player = CreateGameObject(name);

    auto* transform = player->AddComponent<TransformComponent>(x, y);
    auto* physics = player->AddComponent<PhysicsComponent>(1.0f, false);
    player->AddComponent<RenderComponent>(shape, color);
    transform->SetScale(w, h);
    physics->SetVelocity(0, 0);

    // No gravity for this player variant

    std::cout << "Created Player: " << name << " at (" << x << ", " << y << ")" << std::endl;

    return player;
}

GameObject* GameObjectFactory::CreatePlayer(const std::string& name, RenderComponent::ShapeType shape, float x, float y, float w, float h, const char* filename, CollisionLayer layer)
{
    GameObject* player = CreateGameObject(name);
    auto* transform = player->AddComponent<TransformComponent>(x, y);
    auto* physics = player->AddComponent<PhysicsComponent>(1.0f, false);
    GLuint texID = GraphicsSystem::LoadTexture(filename);
    ResourceManager::Instance().PrintLoadedResources();
    auto render = player->AddComponent<RenderComponent>(texID);
    physics->SetStatic(false);
    transform->SetScale(w, h);
    physics->SetVelocity(0, 0);

    // Enable gravity
    physics->SetGravity(true);

    if (shape == RenderComponent::ShapeType::Circle)
    {
        player->AddComponent<ColliderComponent>(ColliderComponent::Type::Circle, w, 0.0f, layer);
        std::cout << "Added Circle collider with radius: " << (w * 0.5f) << std::endl;
    }
    else if (shape == RenderComponent::ShapeType::Square)
    {
        player->AddComponent<ColliderComponent>(ColliderComponent::Type::Square, w, h, layer);
        std::cout << "Added Square collider with size: " << w << "x" << h << std::endl;
    }

    std::cout << "Created Player: " << name << " at (" << x << ", " << y << ")" << std::endl;

    return player;
}

/*
    /brief
     This is the blueprint for the normal gameobject creation where
     it has 3 overloads with the sprite and the color and one more for the collsions,
     color overload is for the creating the shape and the sprite is for the texture and the last one is the collision layer.

*/

GameObject* GameObjectFactory::CreateShape(const std::string& name, RenderComponent::ShapeType shape, float x, float y, float w, float h, const vec4& color, CollisionLayer layer)
{
    GameObject* obj = CreateGameObject(name);

    auto* transform = obj->AddComponent<TransformComponent>(x, y);

    auto render = obj->AddComponent<RenderComponent>(shape, color);

    if (shape == RenderComponent::ShapeType::Line)
    {
        render->SetLine(vec2(x, y), vec2(w, h));
    }
    else if (shape == RenderComponent::ShapeType::Point)
    {
        render->SetPoint(vec2(x, y), w);
    }
    else
    {
        transform->SetScale(w, h);

        // Attach collider automatically
        if (shape == RenderComponent::ShapeType::Circle)
        {
            obj->AddComponent<ColliderComponent>(ColliderComponent::Type::Circle, w, 0.0f, layer);
        }
        else if (shape == RenderComponent::ShapeType::Square)
        {
            obj->AddComponent<ColliderComponent>(ColliderComponent::Type::Square, w, h, layer);
        }
    }
    // testing TO BE REMOVED
    std::cout << "Created "
        << (shape == RenderComponent::ShapeType::Circle ? "Circle" : shape == RenderComponent::ShapeType::Square ? "Square"
            : shape == RenderComponent::ShapeType::Line ? "Line"
            : "Point")
        << ": " << name << " at (" << x << "," << y << ")\n";

    return obj;
}

GameObject* GameObjectFactory::CreateShape(const std::string& name, RenderComponent::ShapeType shape, float x, float y, float w, float h, const vec4& color)
{
    GameObject* obj = CreateGameObject(name);

    auto* transform = obj->AddComponent<TransformComponent>(x, y);

    auto render = obj->AddComponent<RenderComponent>(shape, color);

    if (shape == RenderComponent::ShapeType::Line)
    {
        render->SetLine(vec2(x, y), vec2(w, h));
    }
    else if (shape == RenderComponent::ShapeType::Point)
    {
        render->SetPoint(vec2(x, y), w);
    }
    else
    {
        transform->SetScale(w, h);
    }
    // testing TO BE REMOVED
    std::cout << "Created "
        << (shape == RenderComponent::ShapeType::Circle ? "Circle" : shape == RenderComponent::ShapeType::Square ? "Square"
            : shape == RenderComponent::ShapeType::Line ? "Line"
            : "Point")
        << ": " << name << " at (" << x << "," << y << ")\n";

    return obj;
}

GameObject* GameObjectFactory::CreateShape(const std::string& name, RenderComponent::ShapeType shape, float x, float y, float w, float h, const char* filename)
{
    GameObject* obj = CreateGameObject(name);

    auto* transform = obj->AddComponent<TransformComponent>(x, y);
    GLuint texID = GraphicsSystem::LoadTexture(filename);
    auto render = obj->AddComponent<RenderComponent>(texID);

    if (shape == RenderComponent::ShapeType::Line)
    {
        render->SetLine(vec2(x, y), vec2(w, h));
    }
    else if (shape == RenderComponent::ShapeType::Point)
    {
        render->SetPoint(vec2(x, y), w);
    }
    else
    {
        transform->SetScale(w, h);
    }

    return obj;
}

/*
    /brief
     This is the blueprint for the normal image gameobject, wer would this function to be images that we wanna like the background
*/

GameObject* GameObjectFactory::CreateImage(const std::string& name, RenderComponent::ShapeType, float x, float y, float w, float h, char const* filename)
{
    GameObject* obj = CreateGameObject(name);

    auto* transform = obj->AddComponent<TransformComponent>(x, y);
    //GLint texID = GraphicsSystem::LoadTexture(filename);
    TextureResource* texResource = ResourceManager::Instance().GetTexture(name);
    GLuint texID = 0;

    if (texResource && texResource->IsLoaded())
    {
        // Use existing texture from ResourceManager
        texID = texResource->GetTextureID();
        std::cout << "Using texture from ResourceManager: " << name << std::endl;
    }
    else
    {
        // Fallback: load directly if not in ResourceManager
        texID = GraphicsSystem::LoadTexture(filename);
        std::cout << "Warning: Texture not in ResourceManager, loading directly: " << filename << std::endl;
    }

    obj->AddComponent<RenderComponent>(texID);
    transform->SetScale(w, h);
    return obj;
}

GameObject* GameObjectFactory::CreateAnimatedImage(const std::string& name, float x, float y, float w, float h,
    char const* filename, int numRows, int numCols,
    int startFrame, int endFrame, float fps, CollisionLayer layer)
{
    GameObject* obj = CreateGameObject(name);

    auto* transform = obj->AddComponent<TransformComponent>(x, y);
    transform->SetScale(-w, h);

    auto spritesheetComp = std::make_unique<Spritesheet>(
        obj,
        filename,
        numRows,
        numCols);

    GLuint textureID = spritesheetComp->GetTextureID();

    obj->AddComponent<RenderComponent>(textureID);

    Spritesheet* anim = spritesheetComp.get();
    obj->AddComponent(std::move(spritesheetComp));

    anim->SetAnimation(startFrame, endFrame, fps);

    return obj;
}

/*
    /brief
     This is the blueprint for the Cloning of the gameobjects where we take in the id and newname for the game Object the we clone if we need to.
*/

GameObject* GameObjectFactory::CloneGameObject(EntityID id, const std::string& newName)
{
    GameObject* original = GetGameObject(id);

    if (original)
    {
        std::string finalName;

        if (newName.empty())
        {
            std::stringstream ss;

            size_t length = original->GetName().find_first_of('(');
            if (length == std::string::npos)
            {
                length = original->GetName().length();
            }

            ++cloneCounter;
            ss << original->GetName().substr(0, length) << "(" << cloneCounter << ")";
            finalName = ss.str();
        }
        else
        {
            finalName = newName;
        }

        return original->Clone(finalName);
    }
    return nullptr;
}

/*
    /brief
     This is the blueprint for the Cloning of the gameobjects where we take in the id and newname for the game Object the we clone if we need to.
*/

GameObject* GameObjectFactory::ClonePlayer(EntityID id, const std::string& newName)
{
    GameObject* original = GetGameObject(id);

    if (original)
    {
        std::string finalName;

        if (newName.empty())
        {
            std::stringstream ss;
            size_t length = original->GetName().find_first_of('(');
            if (length == std::string::npos)
            {
                length = original->GetName().length();
            }

            ++cloneCounter;
            ss << original->GetName().substr(0, length) << "(" << cloneCounter << ")";
            finalName = ss.str();
        }
        else
        {
            finalName = newName;
        }

        GameObject* clone = original->Clone(finalName);

        std::cout << "Cloned Player: " << finalName << " from " << original->GetName() << std::endl;

        return clone;
    }
    return nullptr;
}

GameObject* GameObjectFactory::CloneShape(EntityID id, const std::string& newName)
{
    GameObject* original = GetGameObject(id);

    if (original)
    {
        std::string finalName;

        if (newName.empty())
        {
            std::stringstream ss;
            size_t length = original->GetName().find_first_of('(');
            if (length == std::string::npos)
            {
                length = original->GetName().length();
            }

            ++cloneCounter;
            ss << original->GetName().substr(0, length) << "(" << cloneCounter << ")";
            finalName = ss.str();
        }
        else
        {
            finalName = newName;
        }

        GameObject* clone = original->Clone(finalName);

        std::cout << "Cloned Shape: " << finalName << " from " << original->GetName() << std::endl;

        return clone;
    }
    return nullptr;
}

GameObject* GameObjectFactory::CloneImage(EntityID id, const std::string& newName)
{
    GameObject* original = GetGameObject(id);

    if (original)
    {
        std::string finalName;

        if (newName.empty())
        {
            std::stringstream ss;
            size_t length = original->GetName().find_first_of('(');
            if (length == std::string::npos)
            {
                length = original->GetName().length();
            }

            ++cloneCounter;
            ss << original->GetName().substr(0, length) << "(" << cloneCounter << ")";
            finalName = ss.str();
        }
        else
        {
            finalName = newName;
        }

        GameObject* clone = original->Clone(finalName);

        std::cout << "Cloned Image: " << finalName << " from " << original->GetName() << std::endl;

        return clone;
    }
    return nullptr;
}

GameObject* GameObjectFactory::CloneAnimatedImage(EntityID id, const std::string& newName)
{
    GameObject* original = GetGameObject(id);

    if (original)
    {
        std::string finalName;

        if (newName.empty())
        {
            std::stringstream ss;
            size_t length = original->GetName().find_first_of('(');
            if (length == std::string::npos)
            {
                length = original->GetName().length();
            }

            ++cloneCounter;
            ss << original->GetName().substr(0, length) << "(" << cloneCounter << ")";
            finalName = ss.str();
        }
        else
        {
            finalName = newName;
        }

        GameObject* clone = original->Clone(finalName);

        std::cout << "Cloned Animated Image: " << finalName << " from " << original->GetName() << std::endl;

        return clone;
    }
    return nullptr;
}

/*
    /brief
     This is the function that calls the getGameObject to return the gameobject.

*/

GameObject* GameObjectFactory::GetGameObject(EntityID id)
{
    auto it = entityToIndex.find(id);
    if (it != entityToIndex.end() && it->second < gameObjects.size())
    {
        return gameObjects[it->second].get();
    }
    return nullptr;
}

/*
/brief
 This is the function that calls the findGameObject to find the gameobject.

*/
GameObject* GameObjectFactory::FindGameObject(const std::string& name)
{
    for (auto& obj : gameObjects)
    {
        if (obj->GetName() == name)
        {
            return obj.get();
        }
    }
    return nullptr;
}

const std::vector<std::unique_ptr<GameObject>>& GameObjectFactory::GetAllGameObjects() const
{
    return gameObjects;
}
/*
/brief
This is the function that calls the destroyGameObject to destroy/ delete/ set unactive the gameobject.

*/

void GameObjectFactory::DestroyGameObject(EntityID id)
{
    auto it = entityToIndex.find(id);
    if (it != entityToIndex.end())
    {
        size_t index = it->second;

        if (index < gameObjects.size() - 1)
        {
            std::swap(gameObjects[index], gameObjects.back());
            entityToIndex[gameObjects[index]->GetID()] = index;
        }

        gameObjects.pop_back();
        entityToIndex.erase(it);
    }
}

void GameObjectFactory::Clear()
{
    gameObjects.clear();
    entityToIndex.clear();
    nextEntityID = 1;
}

#pragma region M1 Showcase - 2500 GOs Test

// Vector to store all GOs made by test function
std::vector<GameObject*> vecGOs;

/**
* @brief Spawns 2500 Circle Game Objects on screen randomly within the 1920x1080 resolution window.
            (Can spawn more - at your own risk!)
*/
void DemoM1_SpawnGameObjects()
{
    auto& factory = GameObjectFactory::Instance();

    std::random_device randomDevice;
    std::mt19937 gen(randomDevice());
    std::uniform_int_distribution<int> distributionX(-960, 960);
    std::uniform_int_distribution<int> distributionY(-540, 540);

    // spawn 2.5k small circles
    for (size_t i = 0; i < 2500; i++)
    {
        int intX{ distributionX(gen) }, intY{ distributionY(gen) };
        GameObject* newGO = factory.CreateShape("Demo Circles", RenderComponent::ShapeType::Circle, static_cast<float>(intX), static_cast<float>(intY), 40.f, 40.f, vec4(1.f, 0.f, 0.f, 1.f));
        vecGOs.push_back(newGO);
    }
}

/**
* @brief Function that removes all Circle Game Objects spawned from the function above.
            (Yes even if you chose to spawn more than 2500)
*/
void DemoM1_DestroyGameObjects()
{
    auto& factory = GameObjectFactory::Instance();

    for (GameObject*& GO : vecGOs)
    {
        factory.DestroyGameObject(GO->GetID());
    }
    vecGOs.clear();
}

#pragma endregion