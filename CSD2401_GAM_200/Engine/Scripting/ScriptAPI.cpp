
#include "ScriptAPI.h"


#pragma region Engine
void API::Init_Engine(CoreEngine& engine, AudioEngine& audio) {
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

    // Initialise ResourceManager
    std::cout << "\n--- Initialising Resource Manager ---" << std::endl;
    auto& resourceMgr = ResourceManager::Instance();
    resourceMgr.SetAudioEngine(&audio);

    // load textures
    resourceMgr.LoadTexture("player_sprite", "./Assets/protosprite.png");

    // load audio files
    resourceMgr.LoadAudio("bgm", "./Assets/audio/level_1.wav", true); // looping
    resourceMgr.LoadAudio("jump_sfx", "./Assets/audio/jump.wav", false); // when jumping only

    // print loaded resources
    resourceMgr.PrintLoadedResources();

    // register the sound with audio engine
    std::cout << "\n--- Starting Background Music ---" << std::endl;
    audio.playSound("bgm", "bgm_channel");
    audio.setVolume("bgm_channel", 0.5f);
}
#pragma endregion

#pragma region Audio

#pragma endregion

#pragma region Graphics
void API::CreateShape(const std::string& name,
    RenderComponent::ShapeType type,
    float x, float y,
    float width, float height,
    vec4 color,
    CollisionLayer layer,
    bool hasPhysics,
    bool isStatic,
    bool hasGravity)
{
    auto& factory = GameObjectFactory::Instance();
    GameObject* obj = factory.CreateShape(name, type, x, y, width, height, color, layer);

    if (hasPhysics && obj)
    {
        auto* physics = obj->AddComponent<PhysicsComponent>(1.0f, isStatic);
        physics->SetGravity(hasGravity);
        physics->SetStatic(isStatic);
    }
}

void API::CreateImage(const std::string& name,
    RenderComponent::ShapeType type,
    float x, float y,
    float width, float height,
    const std::string& texturePath)
{
    auto& factory = GameObjectFactory::Instance();
    factory.CreateImage(name, type, x, y, width, height, texturePath.c_str());
}

void API::CreateAnimatedImage(const std::string& name,
    float x, float y,
    float width, float height,
    const std::string& texturePath,
    int rows, int columns,
    int startFrame, int endFrame,
    float fps,
    CollisionLayer layer,
    bool hasPhysics,
    bool isStatic,
    bool hasGravity)
{
    auto& factory = GameObjectFactory::Instance();
    GameObject* obj = factory.CreateAnimatedImage(name, x, y, width, height, texturePath.c_str(),
        rows, columns, startFrame, endFrame, fps, layer);

    if (hasPhysics && obj)
    {
        auto* physics = obj->AddComponent<PhysicsComponent>(1.0f, isStatic);
        physics->SetGravity(hasGravity);
        physics->SetStatic(isStatic);
    }
}


#pragma endregion


void API::SetVelocity(const std::string& objectName, float vx, float vy)
{   
   
    auto& factory = GameObjectFactory::Instance();

    GameObject* obj = factory.FindGameObject(objectName);

    if (obj)
    {
        auto* physics = obj->GetComponent<PhysicsComponent>();
        if (physics)
        {
            physics->SetVelocity(vx, vy);
        }
    }
}

void API::StartGameLoop(CoreEngine& engine)
{
    std::cout << "\n--- Starting Engine Gameloop ---" << std::endl;
    engine.GameLoop();
}


