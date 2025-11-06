/* Start Header ************************************************************************/
/*!
\file       ResourceManager.h
\author     Leck Xin Yee [100% - 83 lines]
\par        xinyee.leck@digipen.edu
\date       October, 2, 2025

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
#include <ft2build.h>
#include FT_FREETYPE_H
#include <glm/glm.hpp>
#include <GL/glew.h>

class AudioEngine;

namespace FMOD {
    class Sound;
}

struct Character {
    GLuint TextureID;
    glm::ivec2 size;
    glm::ivec2 bearing;
    GLuint advance;
};

/**
 * @class Resource
 * @brief Base class for all resource types (textures, audio, etc.)
 */
class Resource {
public:
    virtual ~Resource() = default;

    /**
     * @brief Loads a resource from a file
     * @param filepath Path to the resource file
     * @return True if loading was successful, false otherwise
     */
    virtual bool Load(const std::string& filepath) = 0;

    /**
     * @brief Unloads the resource and frees memory
     */
    virtual void Unload() = 0;

    /**
     * @brief Checks if the resource is currently loaded
     * @return True if loaded, false otherwise
     */
    virtual bool IsLoaded() const = 0;

    /**
     * @brief Gets the name identifier of the resource
     * @return Reference to the resource name string
     */
    const std::string& GetName() const { return name; }

    /**
     * @brief Gets the filepath of the resource
     * @return Reference to the filepath string
     */
    const std::string& GetFilePath() const { return filepath; }

    /**
     * @brief Sets the name identifier for the resource
     * @param resourceName Name to assign to the resource
     */
    void SetName(const std::string& resourceName) { name = resourceName; }

protected:
    std::string name;
    std::string filepath;
    bool loaded = false;
};

/**
 * @class TextureResource
 * @brief Manages texture resource loading and unloading
 */
class TextureResource : public Resource {
public:
    /**
     * @brief Constructs a TextureResource
     */
    TextureResource() : textureID(0) {}

    /**
     * @brief Destructs the TextureResource and unloads texture
     */
    ~TextureResource() override;

    /**
     * @brief Loads a texture from a file using OpenGL
     * @param filepath Path to the texture file
     * @return True if loading was successful, false otherwise
     */
    bool Load(const std::string& filepath) override;

    /**
     * @brief Unloads the texture and deletes OpenGL texture object
     */
    void Unload() override;

    /**
     * @brief Checks if the texture is currently loaded
     * @return True if loaded, false otherwise
     */
    bool IsLoaded() const override { return loaded; }

    /**
     * @brief Gets the OpenGL texture ID
     * @return The texture ID
     */
    unsigned int GetTextureID() const { return textureID; }

private:
    unsigned int textureID;
};
    
/**
 * @class AudioResource
 * @brief Manages audio resource loading and unloading through AudioEngine
 */
class AudioResource : public Resource {
public:
    /**
     * @brief Constructs an AudioResource
     */
    AudioResource() : audioEngine(nullptr), loop(false) {}

    /**
     * @brief Destructs the AudioResource and unloads audio
     */
    ~AudioResource() override;

    /**
     * @brief Loads an audio file through the AudioEngine
     * @param filepath Path to the audio file
     * @return True if loading was successful, false otherwise
     */
    bool Load(const std::string& filepath) override;

    /**
     * @brief Unloads the audio resource
     */
    void Unload() override;

    /**
     * @brief Checks if the audio is currently loaded
     * @return True if loaded, false otherwise
     */
    bool IsLoaded() const override { return loaded; }

    /**
     * @brief Sets whether the audio should loop
     * @param shouldLoop True to enable looping, false otherwise
     */
    void SetLoop(bool shouldLoop) { loop = shouldLoop; }

    /**
     * @brief Checks if the audio is set to loop
     * @return True if looping is enabled, false otherwise
     */
    bool IsLoop() const { return loop; }

    /**
     * @brief Sets the AudioEngine to use for loading
     * @param engine Pointer to the AudioEngine
     */
    void SetAudioEngine(AudioEngine* engine) { audioEngine = engine; }

private:
    AudioEngine* audioEngine;
    bool loop;
};

/**
 * @class FontResource
 * @brief Manages font resource loading and character glyph caching
 */
class FontResource : public Resource {
public:
    // cnstructs font resource 
    FontResource() : fontSize(0) {}

    // destructs the font resource and unloads font data
    ~FontResource() override;

    // loads a font from a file using FreeType
    bool Load(const std::string& filepath) override;

    // loads a font with a specific size
    bool Load(const std::string& filepath, GLuint size);

    // unloads the font and deletes all character textures
    void Unload() override;

    // checks if the font is currently loaded
    bool IsLoaded() const override { return loaded; }

    // gets the map of loaded character glyphs
    const std::map<char, Character>& GetCharacters() const { return characters; }

    // gets the font size
    GLuint GetFontSize() const { return fontSize; }

    // sets the font size (requires reload to take effect)
    void SetFontSize(GLuint size) { fontSize = size; }

private:
    std::map<char, Character> characters;
    GLuint fontSize;

    // loads all ASCII characters (0-127) from the font
    void LoadCharacters(FT_Face face);
};


/**
 * @class ResourceManager
 * @brief Singleton manager for all game resources (textures, audio, etc.)
 */
class ResourceManager{
public:
    /**
     * @brief Gets the singleton instance of ResourceManager
     * @return Reference to the ResourceManager instance
     */
    static ResourceManager& Instance() {
        static ResourceManager instance;
        return instance;
    }

    ResourceManager(const ResourceManager&) = delete;
    ResourceManager& operator=(const ResourceManager&) = delete;

    /**
     * @brief Loads a texture resource
     * @param name Identifier for the texture
     * @param filepath Path to the texture file
     * @return Pointer to the loaded TextureResource, or nullptr if failed
     */
    TextureResource* LoadTexture(const std::string& name, const std::string& filepath);

    /**
     * @brief Retrieves a loaded texture resource
     * @param name Identifier of the texture
     * @return Pointer to the TextureResource, or nullptr if not found
     */
    TextureResource* GetTexture(const std::string& name);

    /**
     * @brief Gets the filepath of a texture by its OpenGL texture ID
     * @param textureID The OpenGL texture ID
     * @return The filepath if found, empty string otherwise
     */
    std::string GetTextureFilePath(unsigned int textureID);

    /**
     * @brief Unloads a specific texture resource
     * @param name Identifier of the texture to unload
     */
    void UnloadTexture(const std::string& name);

    /**
     * @brief Loads an audio resource
     * @param name Identifier for the audio
     * @param filepath Path to the audio file
     * @param loop Whether the audio should loop
     * @return Pointer to the loaded AudioResource, or nullptr if failed
     */
    AudioResource* LoadAudio(const std::string& name, const std::string& filepath, bool loop = false);

    /**
     * @brief Retrieves a loaded audio resource
     * @param name Identifier of the audio
     * @return Pointer to the AudioResource, or nullptr if not found
     */
    AudioResource* GetAudio(const std::string& name);

    /**
     * @brief Unloads a specific audio resource
     * @param name Identifier of the audio to unload
     */
    void UnloadAudio(const std::string& name);

    /**
     * @brief Sets the AudioEngine reference for loading audio
     * @param engine Pointer to the AudioEngine
     */
    void SetAudioEngine(AudioEngine* engine);

    /**
     * @brief Gets the current AudioEngine pointer
     * @return Pointer to the AudioEngine
     */
    AudioEngine* GetAudioEngine() const { return audioEngine; }

    // loads a font resource
    FontResource* LoadFont(const std::string& name, const std::string& filepath, GLuint fontSize);

    // retrieves a loaded font resource
    FontResource* GetFont(const std::string& name);

    // unloads a specific font resource
    void UnloadFont(const std::string& name);

    /**
     * @brief Unloads all resources (textures and audio)
     */
    void UnloadAll();

    /**
     * @brief Unloads all texture resources
     */
    void UnloadAllTextures();

    /**
     * @brief Unloads all texture resources
     */
    void UnloadAllAudio();

    // unloads all font resources
    void UnloadAllFonts();

    /**
     * @brief Gets the count of loaded textures
     * @return Number of loaded textures
     */
    size_t GetTextureCount() const { return textures.size(); }

    /**
     * @brief Gets the count of loaded audio resources
     * @return Number of loaded audio resources
     */
    size_t GetAudioCount() const { return audioResources.size(); }

    // gets the count of loaded fonts
    size_t GetFontCount() const { return fonts.size(); }

    /**
     * @brief Prints information about all loaded resources
     */
    void PrintLoadedResources() const;

private:
    ResourceManager();
    ~ResourceManager() {
        UnloadAll();
        std::cout << "Resource manager destroyed" << std::endl;
    }

    std::map<std::string, std::unique_ptr<TextureResource>> textures;
    std::map<std::string, std::unique_ptr<AudioResource>> audioResources;
    std::map<std::string, std::unique_ptr<FontResource>> fonts;

    AudioEngine* audioEngine = nullptr;
    FT_Library ftLibrary = nullptr;
    int ftRefCount = 0;

    // initialises FreeType library if not already initialized
    bool InitialiseFreeType();

    // shuts down FreeType library if no fonts are using it
    void ShutdownFreeType();

    // gets the FreeType library instance
    FT_Library GetFreeTypeLibrary() const { return ftLibrary; }

    /**
     * @brief Template helper to retrieve resources from maps
     * @param name Identifier of the resource
     * @param resourceMap Map containing the resources
     * @return Pointer to the resource, or nullptr if not found
     */
    template<typename Resource>
    Resource* GetResource(const std::string& name, std::map<std::string, std::unique_ptr<Resource>>& resourceMap);

    friend class FontResource;  // allow font resource to access FreeType library
};
