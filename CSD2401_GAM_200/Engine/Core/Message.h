/* Start Header ************************************************************************/
/*!
\file       Message.h
\author     Lee Hwee Min [100% - ?? lines]
\date       November, 3, 2025
Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*/
/* End Header **************************************************************************/

#pragma once
#include <string>
#include <functional>
#include <unordered_map>
#include <vector>
#include <iostream>

/**
 * @brief Represents a message sent between systems.
 *        The `type` identifies the kind of message,
 *        while `sender` and `data` can carry optional context.
 */
struct Message
{
    std::string type;   ///< Identifier, e.g., "Collision", "PlaySound", "SceneChange"
    void* sender = nullptr; ///< Optional pointer to sender (usually a system)
    void* data = nullptr;   ///< Optional payload (could point to a GameObject, string, etc.)
};

/**
 * @brief Simple publish–subscribe message bus.
 *        Systems can subscribe to message types and receive callbacks when those messages are broadcast.
 */
class MessageBus
{
public:
    using Callback = std::function<void(const Message&)>;

    /**
     * @brief Subscribes a callback to a specific message type.
     * @param messageType String identifier of the message to listen for.
     * @param callback Function that handles the message.
     */
    void Subscribe(const std::string& messageType, Callback callback)
    {
        subscribers[messageType].push_back(callback);
    }

    /**
     * @brief Broadcasts a message to all subscribers of that message type.
     * @param msg The message object containing type, sender, and optional data.
     */
    void Broadcast(const Message& msg)
    {
        auto it = subscribers.find(msg.type);
        if (it != subscribers.end())
        {
            for (auto& callback : it->second)
            {
                callback(msg);
            }
        }
#ifdef _DEBUG
        else
        {
            std::cout << "[MessageBus] No subscribers for message type: " << msg.type << std::endl;
        }
#endif
    }

    /**
     * @brief Removes all subscriptions (useful when resetting the engine or changing scenes).
     */
    void Clear()
    {
        subscribers.clear();
    }

private:
    std::unordered_map<std::string, std::vector<Callback>> subscribers;
};
