#pragma once
#include <string>

enum class MessageType {
    PLAY_SOUND,
    STOP_SOUND,
    SET_VOLUME,
    LOAD_TEXTURE,
    SPAWN_ENTITY,
};

class Message {
public:
    Message(MessageType type) : type(type) {}
    virtual ~Message() = default;

    MessageType GetType() const { return type; }

protected:
    MessageType type;
};

// Play sound message
class soundMessage : public Message {
public:
    soundMessage(const std::string& soundName, const std::string& channelName = "", float volume = 1.0f)
        : Message(MessageType::PLAY_SOUND), soundName(soundName), channelName(channelName), volume(volume) {
    }

    std::string soundName;
    std::string channelName;
    float volume;
};

// Stop sound message
class StopSoundMessage : public Message {
public:
    StopSoundMessage(const std::string& channelName)
        : Message(MessageType::STOP_SOUND), channelName(channelName) {
    }

    std::string channelName;
};

// Set volume message
class SetVolumeMessage : public Message {
public:
    SetVolumeMessage(const std::string& channelName, float volume)
        : Message(MessageType::SET_VOLUME), channelName(channelName), volume(volume) {
    }

    std::string channelName;
    float volume;
};