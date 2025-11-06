#pragma once
#include <string>
#include <iostream>
#include <unordered_map>
#include <vector>
#include <algorithm>
#include <MathLib.h>
#include <RenderComponent.h>
#include <typeindex>

// Forward declaration
class IMessageHandler;

class Message {
public:
    enum class Id {
        UNKNOWN,
        KEY_INPUT,
        PLAY_SOUND,
        PAUSE_SOUND,
        COLLISION,
        SCENE_CHANGE,
        CREATE_OBJECT,
        PLAY_ANIMATION
    };

    virtual ~Message() = default;

    Id GetType() const { return type; }

protected:
    Id type;
    explicit Message(Id type) : type(type) {}
};

class KeyMessage : public Message {
public:
    int keyCode;
    bool isPressed;

    KeyMessage(int key, bool pressed)
        : Message(Id::KEY_INPUT), keyCode(key), isPressed(pressed) {
    }
};

class PlaySoundMessage : public Message {
public:
    std::string soundName;
    explicit PlaySoundMessage(const std::string& sound)
        : Message(Id::PLAY_SOUND), soundName(sound) {
    }
};

class PauseSoundMessage : public Message {
public:
    std::string channelName;
    bool pause;

    PauseSoundMessage(const std::string& channel, bool pauseState)
        : Message(Id::PAUSE_SOUND), channelName(channel), pause(pauseState) {
    }
};

class CollisionMessage : public Message {
public:
    void* objectA;
    void* objectB;

    CollisionMessage(void* a, void* b)
        : Message(Id::COLLISION), objectA(a), objectB(b) {
    }
};

class CreateObjectMessage : public Message {
public:
    enum class ObjectType { Empty, Basic, Prefab };

    std::string name;
    ObjectType type;
    RenderComponent::ShapeType shapeType;
    float pos[2];
    float scale[2];
    float rotation;
    vec4 color;

    CreateObjectMessage(const std::string& objName,
        ObjectType objType,
        RenderComponent::ShapeType shape,
        const float position[2],
        const float scaleVal[2],
        float rot,
        const vec4& col)
        : Message(Id::CREATE_OBJECT),
        name(objName),
        type(objType),
        shapeType(shape),
        rotation(rot),
        color(col)
    {
        pos[0] = position[0]; pos[1] = position[1];
        scale[0] = scaleVal[0]; scale[1] = scaleVal[1];
    }

};

class MessageBus {
private:
    std::unordered_map<std::type_index, std::vector<IMessageHandler*>> subscribers;

public:
    // Subscribe to a specific message type
    template<typename T>
    void Subscribe(IMessageHandler* handler) {
        subscribers[typeid(T)].push_back(handler);
    }

    // Unsubscribe a handler from a specific message type
    template<typename T>
    void Unsubscribe(IMessageHandler* handler) {
        auto it = subscribers.find(typeid(T));
        if (it != subscribers.end()) {
            auto& vec = it->second;
            vec.erase(std::remove(vec.begin(), vec.end(), handler), vec.end());
        }
    }

    // Publish message to all handlers of this message type
    template<typename T>
    void Publish(const T& message) {
        auto it = subscribers.find(typeid(T));
        if (it == subscribers.end()) return;

        auto& handlers = it->second;
        for (IMessageHandler* handler : handlers) {
            if (handler)
                handler->HandleMessage(message);
        }
        std::cout << "[MessageBus] Finished publishing \n";
    }

    // Clear all subscribers
    void Clear() { subscribers.clear(); }
};
