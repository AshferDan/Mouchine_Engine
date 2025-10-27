/* Start Header ************************************************************************/
/*!
\file       ResourceManager.cpp
\author     Leck Xin Yee [100% - 156 lines]
\par        xinyee.leck@digipen.edu
\date       October, 2, 2025

Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*/
/* End Header **************************************************************************/

#include "ResourceManager.h"
#include "GraphicsSystem.h"
#include "audio.h"

// Texture Resource Implementation
TextureResource::~TextureResource()
{
    Unload();
}

/**
 * @brief Loads a texture from file using GraphicsSystem
 */
bool TextureResource::Load(const std::string &filepath)
{
    if (loaded)
    {
        std::cout << "Texture already loaded: " << filepath << std::endl;
        return true;
    }


    this->filepath = filepath;

    textureID = GraphicsSystem::LoadTexture(filepath.c_str());

    if (textureID == 0)
    {
        std::cerr << "failed to load texture: " << filepath << std::endl;
        return false;
    }

    loaded = true;
    std::cout << "Texture loaded successfully: " << filepath << " (id: " << textureID << ")" << std::endl;

    return true;
}

/**
 * @brief Unloads the texture and deletes OpenGL texture object
 */
void TextureResource::Unload()
{
    if (loaded && textureID != 0)
    {
        glDeleteTextures(1, &textureID);
        textureID = 0;
        loaded = false;
        std::cout << "Texture unloaded: " << filepath << std::endl;
    }
}

// Audio Resource Implementation
AudioResource::~AudioResource()
{
    Unload();
}

/**
 * @brief Loads an audio file through the AudioEngine
 */
bool AudioResource::Load(const std::string &filepath)
{
    if (loaded)
    {
        std::cout << "Audio already loaded: " << filepath << std::endl;
        return true;
    }

    if (!audioEngine)
    {
        std::cerr << "Cannot load audio: AudioEngine not set" << std::endl;
        return false;
    }

    this->filepath = filepath;

    // Use audio engine load sound
    bool success = audioEngine->loadSound(name, filepath, loop);

    if (!success)
    {
        std::cerr << "Failed to load audio through audio engine: " << filepath << std::endl;
        return false;
    }

    loaded = true;
    std::cout << "Audio loaded successfully through audio engine: " << filepath << std::endl;
    return true;
}

/**
 * @brief Unloads the audio resource
 */
void AudioResource::Unload()
{
    if (loaded && audioEngine)
    {
        // Use audio engine unload sound
        loaded = false;
        std::cout << "Audio unloaded: " << filepath << std::endl;
    }
}

// Resource Manager Implementation
/**
 * @brief Loads a texture resource or returns existing one if already loaded
 */
TextureResource *ResourceManager::LoadTexture(const std::string &name, const std::string &filepath)
{
    // Check if already loaded
    auto it = textures.find(name);
    if (it != textures.end())
    {
        std::cout << "Texture '" << name << "' already loaded, returning existing resource" << std::endl;
        return it->second.get();
    }

    // Create new texture resource
    auto texture = std::make_unique<TextureResource>();
    texture->SetName(name);

    if (!texture->Load(filepath))
    {
        std::cerr << "faailed to load texture: " << name << " from " << filepath << std::endl;
        return nullptr;
    }

    TextureResource *ptr = texture.get();
    textures[name] = std::move(texture);

    return ptr;
}

/**
 * @brief Retrieves a loaded texture resource by name
 */
TextureResource *ResourceManager::GetTexture(const std::string &name)
{
    return GetResource<TextureResource>(name, textures);
}

/**
 * @brief Unloads a specific texture resource from the manager
 */
void ResourceManager::UnloadTexture(const std::string &name)
{
    auto it = textures.find(name);
    if (it != textures.end())
    {
        textures.erase(it);
        std::cout << "Texture '" << name << "' unloaded from resource manager" << std::endl;
    }
}

/**
 * @brief Loads an audio resource or returns existing one if already loaded
 */
AudioResource *ResourceManager::LoadAudio(const std::string &name, const std::string &filepath, bool loop)
{
    auto it = audioResources.find(name);
    if (it != audioResources.end())
    {
        std::cout << "Audio '" << name << "' already loaded, returning existing resource" << std::endl;
        return it->second.get();
    }

    if (!audioEngine)
    {
        std::cerr << "Audio engine not set in resource manager" << std::endl;
        return nullptr;
    }

    auto audio = std::make_unique<AudioResource>();
    audio->SetName(name);
    audio->SetLoop(loop);
    audio->SetAudioEngine(audioEngine);

    if (!audio->Load(filepath))
    {
        std::cerr << "Failed to load audio: " << name << " from " << filepath << std::endl;
        return nullptr;
    }

    AudioResource *ptr = audio.get();
    audioResources[name] = std::move(audio);
    return ptr;
}

/**
 * @brief Retrieves a loaded audio resource by name
 */
AudioResource *ResourceManager::GetAudio(const std::string &name)
{
    return GetResource<AudioResource>(name, audioResources);
}

/**
 * @brief Unloads a specific audio resource from the manager
 */
void ResourceManager::UnloadAudio(const std::string &name)
{
    auto it = audioResources.find(name);
    if (it != audioResources.end())
    {
        audioResources.erase(it);
        std::cout << "Audio '" << name << "' unloaded from resource manager" << std::endl;
    }
}

/**
 * @brief Unloads all textures and audio resources
 */
void ResourceManager::UnloadAll()
{
    std::cout << "Unloading all resources..." << std::endl;
    UnloadAllTextures();
    UnloadAllAudio();
}

/**
 * @brief Unloads all texture resources and clears the texture map
 */
void ResourceManager::UnloadAllTextures()
{
    std::cout << "Unloading " << textures.size() << " textures" << std::endl;
    textures.clear();
}

/**
 * @brief Unloads all audio resources and clears the audio map
 */
void ResourceManager::UnloadAllAudio()
{
    std::cout << "Unloading " << audioResources.size() << " audio resources" << std::endl;
    audioResources.clear();
}

/**
 * @brief Prints detailed information about all loaded resources
 */
void ResourceManager::PrintLoadedResources() const
{
    std::cout << "\n=== Loaded Resources ===" << std::endl;

    std::cout << "Textures (" << textures.size() << "):" << std::endl;
    for (const auto &pair : textures)
    {
        std::cout << "  - " << pair.first
                  << " (ID: " << pair.second->GetTextureID() << ")"
                  << " -> " << pair.second->GetFilePath() << std::endl;
    }

    std::cout << "Audio (" << audioResources.size() << "):" << std::endl;
    for (const auto &pair : audioResources)
    {
        std::cout << "  - " << pair.first << " -> " << pair.second->GetFilePath() << std::endl;
    }
}

/**
 * @brief Sets the AudioEngine reference for loading audio resources
 */
void ResourceManager::SetAudioEngine(AudioEngine *engine)
{
    audioEngine = engine;
    if (audioEngine)
    {
        std::cout << "Audio engine set in resource manager" << std::endl;
    }
}

// Template Helper
/**
 * @brief Template helper to retrieve resources from resource maps
 */
template <typename Resource>
Resource *ResourceManager::GetResource(const std::string &name, std::map<std::string, std::unique_ptr<Resource>> &resourceMap)
{
    auto it = resourceMap.find(name);
    if (it != resourceMap.end())
    {
        return it->second.get();
    }

    std::cerr << "Resource not found: " << name << std::endl;
    return nullptr;
}
