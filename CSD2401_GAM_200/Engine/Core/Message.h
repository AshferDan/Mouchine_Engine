#pragma once
#include <string>

enum class MessageType {
    PLAY_SOUND,
    STOP_SOUND,
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

// audio message
class soundMessage : public Message {
public:
    soundMessage(const std::string& soundName, const std::string& channel = "")
        : Message(MessageType::PLAY_SOUND), soundName(soundName), channel(channel) {
    }

    std::string soundName;
    std::string channel;
};