#pragma once
#pragma once
#include <string>
#include <map>
#include <memory>
#include <iostream>

class AudioEngine;

namespace FMOD {
    class Sound;
}

// base 
class Resource {
public:
    virtual ~Resource() = default;
    virtual bool Load(const std::string& filepath) = 0;
    virtual void Unload() = 0;
    virtual bool IsLoaded() const = 0;

    const std::string& GetName() const { return name; }
    const std::string& GetFilePath() const { return filepath; }
    void SetName(const std::string& resourceName) { name = resourceName; }

protected:
    std::string name;
    std::string filepath;
    bool loaded = false;
};

// texture resource
class TextureResource : public Resource {
public:
    TextureResource() : textureID(0) {}
    ~TextureResource() override;

    bool Load(const std::string& filepath) override;
    void Unload() override;
    bool IsLoaded() const override { return loaded; }

    unsigned int GetTextureID() const { return textureID; }

private:
    unsigned int textureID;
};

// audio resource
namespace FMOD {
    class System;
}

class AudioResource : public Resource {
public:
    AudioResource() : sound(nullptr), loop(false), fmodSystem(nullptr) {}
    ~AudioResource() override;

    bool Load(const std::string& filepath) override;
    void Unload() override;
    bool IsLoaded() const override { return loaded; }

    void SetLoop(bool shouldLoop) { loop = shouldLoop; }
    bool IsLoop() const { return loop; }
    FMOD::Sound* GetSound() const { return sound; }
    void SetFMODSystem(FMOD::System* system) { fmodSystem = system; }

private:
    FMOD::Sound* sound;
    bool loop;
    FMOD::System* fmodSystem;
};

// resource manager class
class ResourceManager {
public:
    static ResourceManager& Instance() {
        static ResourceManager instance;
        return instance;
    }

    ResourceManager(const ResourceManager&) = delete;
    ResourceManager& operator=(const ResourceManager&) = delete;

    // texture management
    TextureResource* LoadTexture(const std::string& name, const std::string& filepath);
    TextureResource* GetTexture(const std::string& name);
    void UnloadTexture(const std::string& name);

    // audio management
    AudioResource* LoadAudio(const std::string& name, const std::string& filepath, bool loop = false);
    AudioResource* GetAudio(const std::string& name);
    void UnloadAudio(const std::string& name);

    // mfod for loading audio
    void SetAudioEngine(AudioEngine* engine);
    AudioEngine* GetAudioEngine() const { return audioEngine; }

    // general management
    void UnloadAll();
    void UnloadAllTextures();
    void UnloadAllAudio();

    // resource info
    size_t GetTextureCount() const { return textures.size(); }
    size_t GetAudioCount() const { return audioResources.size(); }

    void PrintLoadedResources() const;

private:
    ResourceManager() {
        std::cout << "Resource manager initialized" << std::endl;
    }

    ~ResourceManager() {
        UnloadAll();
        std::cout << "Resource manager destroyed" << std::endl;
    }

    // resource containers
    std::map<std::string, std::unique_ptr<TextureResource>> textures;
    std::map<std::string, std::unique_ptr<AudioResource>> audioResources;

    AudioEngine* audioEngine = nullptr;

    template<typename Resource>
    Resource* GetResource(const std::string& name, std::map<std::string, std::unique_ptr<Resource>>& resourceMap);
};
