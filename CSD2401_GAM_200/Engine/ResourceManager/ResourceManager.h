/* Start Header ************************************************************************/
/*!
\file       ResourceManager.h
\author     Leck Xin Yee [100%]
\par        xinyee.leck@digipen.edu
\date       October 02 2025

Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*/
/* End Header **************************************************************************/

#pragma once
#include <string>
#include <map>
#include <memory>
#include <iostream>
#include "System.h"

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
class AudioResource : public Resource {
public:
    AudioResource() : audioEngine(nullptr), loop(false) {}
    ~AudioResource() override;

    bool Load(const std::string& filepath) override;
    void Unload() override;
    bool IsLoaded() const override { return loaded; }

    void SetLoop(bool shouldLoop) { loop = shouldLoop; }
    bool IsLoop() const { return loop; }

    void SetAudioEngine(AudioEngine* engine) { audioEngine = engine; }

private:
    AudioEngine* audioEngine;
    bool loop;
};

// resource manager class
class ResourceManager{
public:
    static ResourceManager& Instance() {
        static ResourceManager instance;
        return instance;
    }

    //void Initialize();
    //void Update(float dt) override;
    //std::string GetName() override;

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

    // Audio engine reference for loading audio
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
    ResourceManager() {}
    ~ResourceManager() {
        UnloadAll();
        std::cout << "Resource manager destroyed" << std::endl;
    }

    std::map<std::string, std::unique_ptr<TextureResource>> textures;
    std::map<std::string, std::unique_ptr<AudioResource>> audioResources;

    AudioEngine* audioEngine = nullptr;

    template<typename Resource>
    Resource* GetResource(const std::string& name, std::map<std::string, std::unique_ptr<Resource>>& resourceMap);
};
