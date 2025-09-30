#include "ResourceManager.h"
#include "GraphicsSystem.h"
#include "audio.h"

/*  Texture Resource Implementation  */
TextureResource::~TextureResource() {
    Unload();
}

bool TextureResource::Load(const std::string& filepath) {
    if (loaded) {
        std::cout << "texture already loaded: " << filepath << std::endl;
        return true;
    }

    this->filepath = filepath;

    textureID = GraphicsSystem::LoadTexture(filepath.c_str());

    if (textureID == 0) {
        std::cerr << "failed to load texture: " << filepath << std::endl;
        return false;
    }

    loaded = true;
    std::cout << "texture loaded successfully: " << filepath << " (id: " << textureID << ")" << std::endl;

    return true;
}

void TextureResource::Unload() {
    if (loaded && textureID != 0) {
        glDeleteTextures(1, &textureID);
        textureID = 0;
        loaded = false;
        std::cout << "texture unloaded: " << filepath << std::endl;
    }
}

/*  Audio Resource Implementation  */
AudioResource::~AudioResource() {
    Unload();
}

bool AudioResource::Load(const std::string& filepath) {
    if (loaded) {
        return true;
    }

    if (!fmodSystem) {
        std::cerr << "cannot load audio cause fmod system not set" << std::endl;
        return false;
    }

    this->filepath = filepath;

    // fmod mode flags
    FMOD_MODE mode = FMOD_DEFAULT;
    if (loop) {
        mode |= FMOD_LOOP_NORMAL;
    }
    else {
        mode |= FMOD_LOOP_OFF;
    }

    // load sound using fmod
    FMOD_RESULT result = fmodSystem->createSound(filepath.c_str(), mode, nullptr, &sound);
    if (result != FMOD_OK) {
        std::cerr << "failed to load audio: " << filepath << " error: " << FMOD_ErrorString(result) << std::endl;
        return false;
    }

    loaded = true;
    std::cout << "audio loaded successfully: " << filepath << std::endl;

    return true;
}

void AudioResource::Unload() {
    if (loaded && sound) {
        sound->release();
        sound = nullptr;
        loaded = false;
        std::cout << "audio unloaded: " << filepath << std::endl;
    }
}

/*  Resource Manager Implementation  */
TextureResource* ResourceManager::LoadTexture(const std::string& name, const std::string& filepath) {
    // Check if already loaded
    auto it = textures.find(name);
    if (it != textures.end()) {
        std::cout << "texture '" << name << "' already loaded, returning existing resource" << std::endl;
        return it->second.get();
    }

    // create new texture resource
    auto texture = std::make_unique<TextureResource>();
    texture->SetName(name);

    if (!texture->Load(filepath)) {
        std::cerr << "faailed to load texture: " << name << " from " << filepath << std::endl;
        return nullptr;
    }

    TextureResource* ptr = texture.get();
    textures[name] = std::move(texture);

    return ptr;
}

TextureResource* ResourceManager::GetTexture(const std::string& name) {
    return GetResource<TextureResource>(name, textures);
}

void ResourceManager::UnloadTexture(const std::string& name) {
    auto it = textures.find(name);
    if (it != textures.end()) {
        textures.erase(it);
        std::cout << "texture '" << name << "' unloaded from resource manager" << std::endl;
    }
}

AudioResource* ResourceManager::LoadAudio(const std::string& name, const std::string& filepath, bool loop) {
    // check if already loaded
    auto it = audioResources.find(name);
    if (it != audioResources.end()) {
        std::cout << "audio '" << name << "' already loaded, returning existing resource" << std::endl;
        return it->second.get();
    }

    if (!audioEngine) {
        std::cerr << "loading audio failed" << std::endl;
        return nullptr;
    }

    // create new audio resource
    auto audio = std::make_unique<AudioResource>();
    audio->SetName(name);
    audio->SetLoop(loop);
    audio->SetFMODSystem(audioEngine->GetFMODSystem());

    if (!audio->Load(filepath)) {
        std::cerr << "failed to load audio: " << name << " from " << filepath << std::endl;
        return nullptr;
    }

    AudioResource* ptr = audio.get();
    audioResources[name] = std::move(audio);

    return ptr;
}

AudioResource* ResourceManager::GetAudio(const std::string& name) {
    return GetResource<AudioResource>(name, audioResources);
}

void ResourceManager::UnloadAudio(const std::string& name) {
    auto it = audioResources.find(name);
    if (it != audioResources.end()) {
        audioResources.erase(it);
        std::cout << "audio '" << name << "' unloaded from resource manager" << std::endl;
    }
}

void ResourceManager::UnloadAll() {
    std::cout << "unloading all resources..." << std::endl;
    UnloadAllTextures();
    UnloadAllAudio();
}

void ResourceManager::UnloadAllTextures() {
    std::cout << "unloading " << textures.size() << " textures" << std::endl;
    textures.clear();
}

void ResourceManager::UnloadAllAudio() {
    std::cout << "unloading " << audioResources.size() << " audio resources" << std::endl;
    audioResources.clear();
}

void ResourceManager::PrintLoadedResources() const {
    std::cout << "\n=== Loaded Resources ===" << std::endl;

    std::cout << "Textures (" << textures.size() << "):" << std::endl;
    for (const auto& pair : textures) {
        std::cout << "  - " << pair.first << " -> " << pair.second->GetFilePath() << std::endl;
    }

    std::cout << "Audio (" << audioResources.size() << "):" << std::endl;
    for (const auto& pair : audioResources) {
        std::cout << "  - " << pair.first << " -> " << pair.second->GetFilePath() << std::endl;
    }
}

void ResourceManager::SetAudioEngine(AudioEngine* engine) {
    audioEngine = engine;
    if (audioEngine) {
        std::cout << "audio engine set in resource manager" << std::endl;
    }
}

/*  Template Helper  */
template<typename Resource>
Resource* ResourceManager::GetResource(const std::string& name, std::map<std::string, std::unique_ptr<Resource>>& resourceMap) {
    auto it = resourceMap.find(name);
    if (it != resourceMap.end()) {
        return it->second.get();
    }

    std::cerr << "resource not found: " << name << std::endl;
    return nullptr;
}
