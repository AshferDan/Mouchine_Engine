#include "ResourceManager.h"
#include "GraphicsSystem.h"
#include "audio.h"

/*  Texture Resource Implementation  */
TextureResource::~TextureResource() {
    Unload();
}

bool TextureResource::Load(const std::string& filepath) {
    if (loaded) {
        std::cout << "Texture already loaded: " << filepath << std::endl;
        return true;
    }

    this->filepath = filepath;

    textureID = GraphicsSystem::LoadTexture(filepath.c_str());

    if (textureID == 0) {
        std::cerr << "failed to load texture: " << filepath << std::endl;
        return false;
    }

    loaded = true;
    std::cout << "Texture loaded successfully: " << filepath << " (id: " << textureID << ")" << std::endl;

    return true;
}

void TextureResource::Unload() {
    if (loaded && textureID != 0) {
        glDeleteTextures(1, &textureID);
        textureID = 0;
        loaded = false;
        std::cout << "Texture unloaded: " << filepath << std::endl;
    }
}

/*  Audio Resource Implementation  */
AudioResource::~AudioResource() {
    Unload();
}

bool AudioResource::Load(const std::string& filepath) {
    if (loaded) {
        std::cout << "Audio already loaded: " << filepath << std::endl;
        return true;
    }

    if (!audioEngine) {
        std::cerr << "Cannot load audio: AudioEngine not set" << std::endl;
        return false;
    }

    this->filepath = filepath;

    // use audio engine load sound
    bool success = audioEngine->loadSound(name, filepath, loop);

    if (!success) {
        std::cerr << "Failed to load audio through audio engine: " << filepath << std::endl;
        return false;
    }

    loaded = true;
    std::cout << "Audio loaded successfully through audio engine: " << filepath << std::endl;
    return true;
}

void AudioResource::Unload() {
    if (loaded && audioEngine) {
        // use audio engine unload sound
        loaded = false;
        std::cout << "Audio unloaded: " << filepath << std::endl;
    }
}


/*  Resource Manager Implementation  */
//void ResourceManager::Initialize() {
//    std::cout << "Resource Manager System initialized" << std::endl;
//}
//
//void ResourceManager::Update(float dt) {
//    // Empty for now - could add async loading, resource streaming, etc. later
//    // You can leave this empty or add: (void)dt; to suppress unused parameter warning
//}
//
//std::string ResourceManager::GetName() {
//    return "Resource Manager System";
//}

TextureResource* ResourceManager::LoadTexture(const std::string& name, const std::string& filepath) {
    // Check if already loaded
    auto it = textures.find(name);
    if (it != textures.end()) {
        std::cout << "Texture '" << name << "' already loaded, returning existing resource" << std::endl;
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
        std::cout << "Texture '" << name << "' unloaded from resource manager" << std::endl;
    }
}

AudioResource* ResourceManager::LoadAudio(const std::string& name, const std::string& filepath, bool loop) {
    auto it = audioResources.find(name);
    if (it != audioResources.end()) {
        std::cout << "Audio '" << name << "' already loaded, returning existing resource" << std::endl;
        return it->second.get();
    }

    if (!audioEngine) {
        std::cerr << "Audio engine not set in resource manager" << std::endl;
        return nullptr;
    }

    auto audio = std::make_unique<AudioResource>();
    audio->SetName(name);
    audio->SetLoop(loop);
    audio->SetAudioEngine(audioEngine);

    if (!audio->Load(filepath)) {
        std::cerr << "Failed to load audio: " << name << " from " << filepath << std::endl;
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
        std::cout << "Audio '" << name << "' unloaded from resource manager" << std::endl;
    }
}

void ResourceManager::UnloadAll() {
    std::cout << "Unloading all resources..." << std::endl;
    UnloadAllTextures();
    UnloadAllAudio();
}

void ResourceManager::UnloadAllTextures() {
    std::cout << "Unloading " << textures.size() << " textures" << std::endl;
    textures.clear();
}

void ResourceManager::UnloadAllAudio() {
    std::cout << "Unloading " << audioResources.size() << " audio resources" << std::endl;
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
        std::cout << "Audio engine set in resource manager" << std::endl;
    }
}

/*  Template Helper  */
template<typename Resource>
Resource* ResourceManager::GetResource(const std::string& name, std::map<std::string, std::unique_ptr<Resource>>& resourceMap) {
    auto it = resourceMap.find(name);
    if (it != resourceMap.end()) {
        return it->second.get();
    }

    std::cerr << "Resource not found: " << name << std::endl;
    return nullptr;
}
