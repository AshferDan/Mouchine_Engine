#include "Factory.h"
#include "GraphicsSystem.h"

#include "transform.h"
#include "collision.h"
#include "physics.h"
#include <iostream>
#include <cmath>
#include <sstream>
#include <random> // For M1 demo, spawning 2.5k GOs

GameObject *GameObjectFactory::CreateGameObject(const std::string &name)
{
    EntityID id = nextEntityID++;
    auto gameObject = std::make_unique<GameObject>(id, name);
    GameObject *ptr = gameObject.get();

    entityToIndex[id] = gameObjects.size();
    gameObjects.push_back(std::move(gameObject));

    return ptr;
}

GameObject *GameObjectFactory::CreatePlayer(const std::string &name, RenderComponent::ShapeType shape, float x, float y, float w, float h, const glm::vec4 color, CollisionLayer)
{
    GameObject *player = CreateGameObject(name);

    auto *transform = player->AddComponent<TransformComponent>(x, y);
    auto *physics = player->AddComponent<PhysicsComponent>(1.0f, false);
    player->AddComponent<RenderComponent>(shape, color);
    transform->SetScale(w, h);
    physics->SetVelocity(0, 0);
    physics->SetGravity(false);

    std::cout << "Created Player: " << name << " at (" << x << ", " << y << ")" << std::endl;

    return player;
}

GameObject *GameObjectFactory::CreatePlayer(const std::string &name, RenderComponent::ShapeType shape, float x, float y, float w, float h, const char *filename, CollisionLayer layer)
{
    GameObject *player = CreateGameObject(name);
    auto *transform = player->AddComponent<TransformComponent>(x, y);
    auto *physics = player->AddComponent<PhysicsComponent>(0, 0);
    GLuint texID = GraphicsSystem::LoadTexture(filename);
    // resource manager - check it out and return texture id
    player->AddComponent<RenderComponent>(texID);
    physics->SetGravity(true); // Enable/Disable gravity
    physics->SetStatic(false);
    transform->SetScale(w, h);
    physics->SetVelocity(0, 0);
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

GameObject *GameObjectFactory::CreateShape(const std::string &name, RenderComponent::ShapeType shape, float x, float y, float w, float h, const glm::vec4 &color, CollisionLayer layer)
{
    GameObject *obj = CreateGameObject(name);

    auto *transform = obj->AddComponent<TransformComponent>(x, y);

    auto render = obj->AddComponent<RenderComponent>(shape, color);

    if (shape == RenderComponent::ShapeType::Line)
    {
        render->SetLine(glm::vec2(x, y), glm::vec2(w, h));
    }
    else if (shape == RenderComponent::ShapeType::Point)
    {
        render->SetPoint(glm::vec2(x, y), w);
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
                                                                       : shape == RenderComponent::ShapeType::Line     ? "Line"
                                                                                                                       : "Point")
              << ": " << name << " at (" << x << "," << y << ")\n";

    return obj;
}



GameObject* GameObjectFactory::CreateShape(const std::string& name, RenderComponent::ShapeType shape, float x, float y, float w, float h, const glm::vec4& color) {
    GameObject* obj = CreateGameObject(name);

    auto* transform = obj->AddComponent<TransformComponent>(x, y);

    auto render = obj->AddComponent<RenderComponent>(shape, color);

    if (shape == RenderComponent::ShapeType::Line) {
        render->SetLine(glm::vec2(x, y), glm::vec2(w, h));
    }
    else if (shape == RenderComponent::ShapeType::Point) {
        render->SetPoint(glm::vec2(x, y), w);
    }
    else {
        transform->SetScale(w, h);
    }
    //testing TO BE REMOVED
    std::cout << "Created "
        << (shape == RenderComponent::ShapeType::Circle ? "Circle" :
            shape == RenderComponent::ShapeType::Square ? "Square" :
            shape == RenderComponent::ShapeType::Line ? "Line" : "Point")
        << ": " << name << " at (" << x << "," << y << ")\n";

    return obj;
}

GameObject *GameObjectFactory::CreateShape(const std::string &name, RenderComponent::ShapeType shape, float x, float y, float w, float h, const char *filename)
{
    GameObject *obj = CreateGameObject(name);

    auto *transform = obj->AddComponent<TransformComponent>(x, y);
    GLuint texID = GraphicsSystem::LoadTexture(filename);
    auto render = obj->AddComponent<RenderComponent>(texID);


    if (shape == RenderComponent::ShapeType::Line)
    {
        render->SetLine(glm::vec2(x, y), glm::vec2(w, h));
    }
    else if (shape == RenderComponent::ShapeType::Point)
    {
        render->SetPoint(glm::vec2(x, y), w);
    }
    else
    {
        transform->SetScale(w, h);
    }
    // testing TO BE REMOVED
    //std::cout << "Created "
    //          << (shape == RenderComponent::ShapeType::Circle ? "Circle" : shape == RenderComponent::ShapeType::Square ? "Square"
    //                                                                   : shape == RenderComponent::ShapeType::Line     ? "Line"
    //                                                                                                                   : "Point")
    //          << ": " << name << " at (" << x << "," << y << ")\n";

    return obj;
}

GameObject *GameObjectFactory::CloneGameObject(EntityID id, const std::string &newName)
{
    GameObject *original = GetGameObject(id);

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

GameObject *GameObjectFactory::CreateImage(const std::string &name, RenderComponent::ShapeType, float x, float y, float w, float h, char const *filename)
{
    GameObject *obj = CreateGameObject(name);

    auto *transform = obj->AddComponent<TransformComponent>(x, y);
    GLint texID = GraphicsSystem::LoadTexture(filename);
    obj->AddComponent<RenderComponent>(texID);
    transform->SetScale(w, h);
    return obj;
}
GameObject *GameObjectFactory::CreateAnimatedImage(const std::string &name, float x, float y, float w, float h,
                                                   char const *filename, int numRows, int numCols,
                                                   int startFrame, int endFrame, float fps)
{
    GameObject *obj = CreateGameObject(name);

    auto *transform = obj->AddComponent<TransformComponent>(x, y);
    transform->SetScale(-w, h);

    auto spritesheetComp = std::make_unique<Spritesheet>(
        obj,
        filename,
        numRows,
        numCols);

    GLuint textureID = spritesheetComp->GetTextureID();

    obj->AddComponent<RenderComponent>(textureID);

    Spritesheet *anim = spritesheetComp.get();
    obj->AddComponent(std::move(spritesheetComp));

    anim->SetAnimation(startFrame, endFrame, fps);

    return obj;
}

GameObject* GameObjectFactory::CreateAnimatedImage(const std::string& name, float x, float y, float w, float h, char const* filename, int numRows, int numCols, int startFrame, int endFrame, float fps, CollisionLayer layer)
{
    GameObject* obj = CreateGameObject(name);

    auto* transform = obj->AddComponent<TransformComponent>(x, y);
    auto* physics = obj->AddComponent<PhysicsComponent>(0, 0);
    GLuint texID = GraphicsSystem::LoadTexture(filename);
    obj->AddComponent<RenderComponent>(texID);
    physics->SetGravity(true); // Enable/Disable gravity
    physics->SetStatic(false);
    transform->SetScale(w, h);
    physics->SetVelocity(0, 0);
    physics->SetGravity(true);
    obj->AddComponent<ColliderComponent>(ColliderComponent::Type::Square, w, h, layer);
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

GameObject *GameObjectFactory::ClonePlayer(EntityID id, const std::string &newName)
{
    GameObject *original = GetGameObject(id);

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

        GameObject *clone = original->Clone(finalName);

        std::cout << "Cloned Player: " << finalName << " from " << original->GetName() << std::endl;

        return clone;
    }
    return nullptr;
}

GameObject *GameObjectFactory::CloneShape(EntityID id, const std::string &newName)
{
    GameObject *original = GetGameObject(id);

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

        GameObject *clone = original->Clone(finalName);

        std::cout << "Cloned Shape: " << finalName << " from " << original->GetName() << std::endl;

        return clone;
    }
    return nullptr;
}

GameObject *GameObjectFactory::CloneImage(EntityID id, const std::string &newName)
{
    GameObject *original = GetGameObject(id);

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

        GameObject *clone = original->Clone(finalName);

        std::cout << "Cloned Image: " << finalName << " from " << original->GetName() << std::endl;

        return clone;
    }
    return nullptr;
}

GameObject *GameObjectFactory::CloneAnimatedImage(EntityID id, const std::string &newName)
{
    GameObject *original = GetGameObject(id);

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

        GameObject *clone = original->Clone(finalName);

        std::cout << "Cloned Animated Image: " << finalName << " from " << original->GetName() << std::endl;

        return clone;
    }
    return nullptr;
}

    GameObject *GameObjectFactory::GetGameObject(EntityID id)
    {
        auto it = entityToIndex.find(id);
        if (it != entityToIndex.end() && it->second < gameObjects.size())
        {
            return gameObjects[it->second].get();
        }
        return nullptr;
    }

    GameObject *GameObjectFactory::FindGameObject(const std::string &name)
    {
        for (auto &obj : gameObjects)
        {
            if (obj->GetName() == name)
            {
                return obj.get();
            }
        }
        return nullptr;
    }

    const std::vector<std::unique_ptr<GameObject>> &GameObjectFactory::GetAllGameObjects() const
    {
        return gameObjects;
    }

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

    // FUNCTIONS TO SPAWN 2.5K OBJECTS FOR M1 DEMO!!

    std::vector<GameObject *> vecGOs;
    void DemoM1_SpawnGameObjects()
    {
        auto &factory = GameObjectFactory::Instance();

        std::random_device randomDevice;
        std::mt19937 gen(randomDevice());
        std::uniform_int_distribution<int> distributionX(-960, 960);
        std::uniform_int_distribution<int> distributionY(-540, 540);

        // spawn 2.5k small circles
        for (size_t i = 0; i < 2500; i++)
        {
            int intX{distributionX(gen)}, intY{distributionY(gen)};
            GameObject *newGO = factory.CreateShape("Demo Circles", RenderComponent::ShapeType::Circle, static_cast<float>(intX), static_cast<float>(intY), 40.f, 40.f, glm::vec4(1.f, 0.f, 0.f, 1.f));
            // GameObject* newGO = factory.CreateAnimatedImage("MouchRunner", static_cast<float>(intX), static_cast<float>(intY), 100.0f, 100.0f, "../Assets/Mouch_Running.png", 1, 2, 0, 7,9.0f);
            vecGOs.push_back(newGO);
        }
    }

    void DemoM1_DestroyGameObjects()
    {
        auto &factory = GameObjectFactory::Instance();

        for (GameObject *&GO : vecGOs)
        {
            factory.DestroyGameObject(GO->GetID());
        }
        vecGOs.clear();
    }
