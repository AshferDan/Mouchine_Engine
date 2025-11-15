#include "AudioManager.h"
#include "Message.h"
#include <iostream>

AudioManager::AudioManager(AudioEngine* engine)
    : audioEngine(engine) {

    // default category volumes
    categoryVolumes["music"] = 1.0f;
    categoryVolumes["sfx"] = 1.0f;
    categoryVolumes["voice"] = 1.0f;
    categoryVolumes["ambient"] = 1.0f;
}

AudioManager::~AudioManager() {
    StopAllSounds();
}

bool AudioManager::LoadAllSounds() {
    if (!audioEngine) {
        std::cerr << "AudioEngine not set in AudioManager" << std::endl;
        return false;
    }

    std::cout << "\n=== Loading Audio Files ===" << std::endl;

    bool allSuccessful = true;
    for (const auto& pair : soundDatabase) {
        const SoundData& sound = pair.second;

        std::cout << "Loading: " << sound.name << " from " << sound.filepath << std::endl;

        bool success = audioEngine->loadSound(sound.name, sound.filepath, sound.loop);
        if (!success) {
            std::cerr << "Failed to load sound: " << sound.name << std::endl;
            allSuccessful = false;
        }
    }

    std::cout << "Audio loading complete" << std::endl;
    return allSuccessful;
}

void AudioManager::TriggerEvent(const std::string& eventName) {
    auto it = eventToSoundMap.find(eventName);
    if (it != eventToSoundMap.end()) {
        PlaySound(it->second);
    }
    else {
        std::cout << "No sound mapped to event: " << eventName << std::endl;
    }
}

void AudioManager::PlaySound(const std::string& soundName, const std::string& channelName) {
    if (!audioEngine) {
        std::cerr << "AudioEngine not available" << std::endl;
        return;
    }

    auto it = soundDatabase.find(soundName);
    if (it == soundDatabase.end()) {
        std::cerr << "Sound not found in database: " << soundName << std::endl;
        return;
    }

    const SoundData& sound = it->second;

    // Play the sound
    std::string channel = channelName.empty() ? soundName + "_channel" : channelName;
    audioEngine->playSound(soundName, channel);

    // Apply volume (base volume * category volume)
    float finalVolume = sound.volume;
    auto categoryIt = categoryVolumes.find(sound.category);
    if (categoryIt != categoryVolumes.end()) {
        finalVolume *= categoryIt->second;
    }

    audioEngine->setVolume(channel, finalVolume);
}

void AudioManager::SetCategoryVolume(const std::string& category, float volume) {
    categoryVolumes[category] = std::max(0.0f, std::min(1.0f, volume));
    std::cout << "Set " << category << " volume to " << volume << std::endl;
}

void AudioManager::StopSound(const std::string& channelName) {
    if (audioEngine) {
        audioEngine->stopSound(channelName);
    }
}

void AudioManager::StopAllSounds() {
    if (audioEngine) {
        audioEngine->stopAllSound();
    }
}

void AudioManager::HandleMessage(const PlaySoundMessage& msg) {
    PlaySound(msg.soundName);
}

void AudioManager::HandleMessage(const PauseSoundMessage& msg) {
    if (audioEngine) {
        audioEngine->pauseSound(msg.channelName, msg.pause);
    }
}

void AudioManager::PrintConfig() const {
    std::cout << "\n=== Audio Manager Configuration ===" << std::endl;

    std::cout << "\nSounds (" << soundDatabase.size() << "):" << std::endl;
    for (const auto& pair : soundDatabase) {
        const SoundData& sound = pair.second;
        std::cout << "  - " << sound.name
            << " [" << sound.category << "]"
            << " (loop: " << (sound.loop ? "yes" : "no")
            << ", vol: " << sound.volume << ")" << std::endl;
        std::cout << "    File: " << sound.filepath << std::endl;
    }

    std::cout << "\nEvent Mappings (" << eventToSoundMap.size() << "):" << std::endl;
    for (const auto& pair : eventToSoundMap) {
        std::cout << "  - " << pair.first << " -> " << pair.second << std::endl;
    }

    std::cout << "\nCategory Volumes:" << std::endl;
    for (const auto& pair : categoryVolumes) {
        std::cout << "  - " << pair.first << ": " << pair.second << std::endl;
    }
}

void AudioManager::AddSound(const SoundData& sound) {
    soundDatabase[sound.name] = sound;
}

void AudioManager::AddEventMapping(const std::string& eventName, const std::string& soundName) {
    eventToSoundMap[eventName] = soundName;
}

void AudioManager::Clear() {
    soundDatabase.clear();
    eventToSoundMap.clear();
}