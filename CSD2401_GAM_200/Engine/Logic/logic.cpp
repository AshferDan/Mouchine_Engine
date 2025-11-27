#include "logic.h"
#include <iostream>

std::vector<Event> EventSystem::events;

const char* EventTypeToString(EventType e)
{
    switch (e)
    {
    case EventType::None: return "None";
    case EventType::OnStart: return "OnStart";
    case EventType::OnEnable: return "OnEnable";
    case EventType::OnDisable: return "OnDisable";
    case EventType::OnDestroy: return "OnDestroy";
    case EventType::OnUpdate: return "OnUpdate";
    //case EventType::OnKeyPressed: return "OnKeyPressed";
    //case EventType::OnKeyReleased: return "OnKeyReleased";
    //case EventType::OnMousePressed: return "OnMousePressed";
    //case EventType::OnMouseReleased: return "OnMouseReleased";
    //case EventType::OnMouseOver: return "OnMouseOver";
    case EventType::OnCollisionEnter: return "OnCollisionEnter";
    case EventType::OnCollisionStay: return "OnCollisionStay";
    case EventType::OnCollisionExit: return "OnCollisionExit";
    case EventType::OnTriggerEnter: return "OnTriggerEnter";
    case EventType::OnTriggerStay: return "OnTriggerStay";
    case EventType::OnTriggerExit: return "OnTriggerExit";
    //case EventType::OnGoalReached: return "OnGoalReached";
    //case EventType::OnFellOutOfMap: return "OnFellOutOfMap";
    }
    return "Unknown";
}

EventType StringToEventType(const std::string& s)
{
    if (s == "None") return EventType::None;
    if (s == "OnStart") return EventType::OnStart;
    if (s == "OnEnable") return EventType::OnEnable;
    if (s == "OnDisable") return EventType::OnDisable;
    if (s == "OnDestroy") return EventType::OnDestroy;
    if (s == "OnUpdate") return EventType::OnUpdate;
    //if (s == "OnKeyPressed") return EventType::OnKeyPressed;
    //if (s == "OnKeyReleased") return EventType::OnKeyReleased;
    //if (s == "OnMousePressed") return EventType::OnMousePressed;
    //if (s == "OnMouseReleased") return EventType::OnMouseReleased;
    //if (s == "OnMouseOver") return EventType::OnMouseOver;
    if (s == "OnCollisionEnter") return EventType::OnCollisionEnter;
    if (s == "OnCollisionStay") return EventType::OnCollisionStay;
    if (s == "OnCollisionExit") return EventType::OnCollisionExit;
    if (s == "OnTriggerEnter") return EventType::OnTriggerEnter;
    if (s == "OnTriggerStay") return EventType::OnTriggerStay;
    if (s == "OnTriggerExit") return EventType::OnTriggerExit;
    //if (s == "OnGoalReached") return EventType::OnGoalReached;
    //if (s == "OnFellOutOfMap") return EventType::OnFellOutOfMap;

    return EventType::None;
}

const char* ActionTypeToString(ActionType a)
{
    switch (a)
    {
    case ActionType::None: return "None";
    //case ActionType::Shrink: return "Shrink";
    //case ActionType::Grow: return "Grow";
    //case ActionType::ScaleTo: return "ScaleTo";
    //case ActionType::MoveTo: return "MoveTo";
    //case ActionType::TeleportTo: return "TeleportTo";
    case ActionType::EnableObject: return "EnableObject";
    case ActionType::DisableObject: return "DisableObject";
    case ActionType::DestroyObject: return "DestroyObject";
    case ActionType::PlaySound: return "PlaySound";
    case ActionType::StopSound: return "StopSound";
    //case ActionType::LoadNextLevel: return "LoadNextLevel";
    case ActionType::Respawn: return "Respawn";
    case ActionType::AddScore: return "AddScore";
    case ActionType::TakeDamage: return "TakeDamage";
    case ActionType::FlashColor: return "FlashColor";
    }
    return "Unknown";
}

ActionType StringToActionType(const std::string& s)
{
    if (s == "None") return ActionType::None;
    //if (s == "Shrink") return ActionType::Shrink;
    //if (s == "Grow") return ActionType::Grow;
    //if (s == "ScaleTo") return ActionType::ScaleTo;
    //if (s == "MoveTo") return ActionType::MoveTo;
    //if (s == "TeleportTo") return ActionType::TeleportTo;
    if (s == "EnableObject") return ActionType::EnableObject;
    if (s == "DisableObject") return ActionType::DisableObject;
    if (s == "DestroyObject") return ActionType::DestroyObject;
    if (s == "PlaySound") return ActionType::PlaySound;
    if (s == "StopSound") return ActionType::StopSound;
    //if (s == "LoadNextLevel") return ActionType::LoadNextLevel;
    if (s == "Respawn") return ActionType::Respawn;
    if (s == "AddScore") return ActionType::AddScore;
    if (s == "TakeDamage") return ActionType::TakeDamage;
    if (s == "FlashColor") return ActionType::FlashColor;

    return ActionType::None;
}

EventComponent::EventComponent(GameObject* owner) : IComponent(owner) {}

EventComponent::EventComponent() : IComponent(nullptr) {}

std::unique_ptr<IComponent> EventComponent::Clone(GameObject* newClone) const {
    auto clone = std::make_unique<EventComponent>(newClone);
    clone->rules = rules;
    return clone;
}

void EventSystem::Raise(EventType type, GameObject* target, GameObject* other) {
    std::cout << "[RAISE] " << EventTypeToString(type)
        << " | target: " << (target ? target->GetName() : "null")
        << " | other: " << (other ? other->GetName() : "null")
        << std::endl;
    
    events.push_back({ type, target, other });
}

void EventSystem::ProcessEvents() {
    auto queue = events;
    events.clear();

    for (auto& e : queue) {
        if (e.target) {
            e.target->OnEventTriggered(e.type, e.other);
        }
    }
}