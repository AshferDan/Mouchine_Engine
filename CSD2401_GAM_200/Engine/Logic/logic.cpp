/* Start Header ************************************************************************/
/*!
\file       logic.cpp
\author     Lee Hwee Min [100% - 135 lines]
\par        l.hweemin@digipen.edu
\date       November, 29, 2025

Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*/
/* End Header **************************************************************************/

#include "logic.h"
#include "gameObject.h"
#include "factory.h"
#include "AudioManager.h"
#include <iostream>

// ========== STRING CONVERSIONS ==========

const char *EventTypeToString(EventType e)
{
    switch (e)
    {
    case EventType::None:
        return "None";
    case EventType::OnEnable:
        return "OnEnable";
    case EventType::OnDisable:
        return "OnDisable";
    case EventType::OnDestroy:
        return "OnDestroy";
    case EventType::OnCollisionEnter:
        return "OnCollisionEnter";
    case EventType::OnCollisionStay:
        return "OnCollisionStay";
    case EventType::OnCollisionExit:
        return "OnCollisionExit";
    }
    return "Unknown";
}

EventType StringToEventType(const std::string &s)
{
    if (s == "None")
        return EventType::None;
    if (s == "OnEnable")
        return EventType::OnEnable;
    if (s == "OnDisable")
        return EventType::OnDisable;
    if (s == "OnDestroy")
        return EventType::OnDestroy;
    if (s == "OnCollisionEnter")
        return EventType::OnCollisionEnter;
    if (s == "OnCollisionStay")
        return EventType::OnCollisionStay;
    if (s == "OnCollisionExit")
        return EventType::OnCollisionExit;

    return EventType::None;
}

const char *ActionTypeToString(ActionType a)
{
    switch (a)
    {
    case ActionType::None:
        return "None";
    case ActionType::EnableObject:
        return "EnableObject";
    case ActionType::DisableObject:
        return "DisableObject";
    case ActionType::DestroyObject:
        return "DestroyObject";
    case ActionType::PlaySound:
        return "PlaySound";
    case ActionType::StopSound:
        return "StopSound";
    case ActionType::Respawn:
        return "Respawn";
    }
    return "Unknown";
}

ActionType StringToActionType(const std::string &s)
{
    if (s == "None")
        return ActionType::None;
    if (s == "EnableObject")
        return ActionType::EnableObject;
    if (s == "DisableObject")
        return ActionType::DisableObject;
    if (s == "DestroyObject")
        return ActionType::DestroyObject;
    if (s == "PlaySound")
        return ActionType::PlaySound;
    if (s == "StopSound")
        return ActionType::StopSound;
    if (s == "Respawn")
        return ActionType::Respawn;

    return ActionType::None;
}

// ========== DEEP COPY ==========
LogicChain::LogicChain(const LogicChain &other)
{
    triggerType = other.triggerType;
    targetFilterID = other.targetFilterID;
    for (const auto &action : other.actions)
    {
        actions.push_back(action->Clone());
    }
}

LogicChain &LogicChain::operator=(const LogicChain &other)
{
    if (this == &other)
        return *this;
    triggerType = other.triggerType;
    targetFilterID = other.targetFilterID;
    actions.clear();
    for (const auto &action : other.actions)
    {
        actions.push_back(action->Clone());
    }
    return *this;
}

// ========== ACTIONS IMPLEMENTATION ===========
void ActionPlaySound::Execute(GameObject *owner, [[maybe_unused]] GameObject *other)
{
    if (soundName.empty())
        return;
    if (auto *audio = CORE->GetAudioManager())
    {
        audio->PlaySound(soundName, "sfx");
    }
    std::cout << "[Action] Playing Sound: " << soundName << " on " << owner->GetName() << std::endl;
}

void ActionDestroy::Execute(GameObject *owner, [[maybe_unused]] GameObject *other)
{
    GameObjectFactory::Instance().DestroyGameObject(owner->GetID());
    std::cout << "[Action] Destroying Object: " << owner->GetName() << std::endl;
}

void ActionToggleActive::Execute(GameObject *owner, [[maybe_unused]] GameObject *other)
{
    owner->SetActive(setState);
}

void ActionRespawn::Execute(GameObject *owner, [[maybe_unused]] GameObject *other)
{
    auto *transform = owner->GetComponent<TransformComponent>();
    if (transform)
    {
        transform->SetPosition(spawnX, spawnY);
    }

    std::cout << "[Action] Respawning Object: " << owner->GetName() << std::endl;

}

// ========== EvENT COMPONENT ==========

EventComponent::EventComponent(GameObject *owner) : IComponent(owner) {}
EventComponent::EventComponent() : IComponent(nullptr) {}

void EventComponent::OnEvent(EventType type, GameObject *other)
{
    for (const auto &chain : chains)
    {
        // Check event type subscription
        if (chain.triggerType != type)
            continue;

        // Debug: Event matched, checking filter
        std::cout << "[Logic] Object '" << owner->GetName()
                  << "' received event: " << EventTypeToString(type) << std::endl;

        // Check filter
        if (chain.targetFilterID != -1)
        {
            if (!other || static_cast<int>(other->GetID()) != chain.targetFilterID)
            {
                std::cout << "  -> Filter mismatch (Expected ID: " << chain.targetFilterID << ")" << std::endl;
                continue;
            }
        }

        std::cout << "  -> Executing " << chain.actions.size() << " actions..." << std::endl;

        // Execute all actions in the logic chain
        for (const auto &action : chain.actions)
        {
            action->Execute(owner, other);
        }
    }
}

/**
 * @brief Clone this component when duplicating a prefab or GameObject.
 *        All rules are copied to the new instance.
 */
std::unique_ptr<IComponent> EventComponent::Clone(GameObject *newClone) const
{
    auto clone = std::make_unique<EventComponent>(newClone);
    clone->chains = this->chains;
    return clone;
}

// ========== EVENT SYSTEM - DISPATCHER ==========

std::vector<QueuedEvent> EventSystem::events;

/**
 * @brief Queue a new event to be processed next frame.
 */
void EventSystem::Raise(EventType type, GameObject *target, GameObject *other)
{
    events.push_back({type, target, other});
}

/**
 * @brief Clear all pending events.
 *        Used when switching scenes or resetting the editor.
 */
void EventSystem::Clear()
{
    events.clear();
}

/**
 * @brief Safely process all queued gameplay events.
 */
void EventSystem::ProcessEvents()
{
    auto queue = events;
    events.clear();

    for (auto &e : queue)
    {
        if (!e.target)
            continue;

        if (e.type == EventType::OnDestroy)
        {
            continue;
        }

        auto *ec = e.target->GetComponent<EventComponent>();
        if (ec)
        {
            ec->OnEvent(e.type, e.other);
        }
    }
}