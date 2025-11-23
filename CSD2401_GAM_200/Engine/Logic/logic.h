#pragma once
#include <string>
#include <vector>
#include <iostream>
#include <memory>

// Forward Declaration
class GameObject;
class IComponent;
class Isystem;

enum class EventType {
    None = 0,

    // Lifecycle Events
    OnStart,            // When object first created or first frame
    OnEnable,           // When object becomes active
    OnDisable,          // When object becomes inactive
    OnDestroy,

    // Frame Events
    OnUpdate,

    // Input Events
    OnKeyPressed,
    OnKeyReleased,
    OnMousePressed,
    OnMouseReleased,
    OnMouseOver, // Hover

    // Physics Events
    OnCollisionEnter,   // Collision start
    OnCollisionStay,    // Still colliding
    OnCollisionExit,    // Collision end

    OnTriggerEnter,     // Trigger start
    OnTriggerStay,      // Trigger stay
    OnTriggerExit,      // Trigger end

    // Gameplay Events
    //OnHealthZero,
    //OnCheckpointReached,
    OnGoalReached,      // Level Finish
    OnFellOutOfMap,     // Falling off map
};

enum class ActionType
{
    None = 0,

    // Transform Actions
    Shrink,
    Grow,
    ScaleTo,
    MoveTo,
    TeleportTo,

    // Object State Actions
    EnableObject,
    DisableObject,
    DestroyObject,

    // Audio Actions
    PlaySound,
    StopSound,

    // Gameplay Actions
    LoadNextLevel,
    Respawn,
    AddScore,
    TakeDamage,

    // Visual Actions
    FlashColor,
    //ShakeObject,
};


// Convert EventType to Readable strings
static inline const char* EventTypeToString(EventType e) {
    switch (e) {
    case EventType::None:               return "None";
    case EventType::OnStart:            return "OnStart";
    case EventType::OnEnable:           return "OnEnable";
    case EventType::OnDisable:          return "OnDisable";
    case EventType::OnDestroy:          return "OnDestroy";

    case EventType::OnUpdate:           return "OnUpdate";

    case EventType::OnKeyPressed:       return "OnKeyPressed";
    case EventType::OnKeyReleased:      return "OnKeyReleased";
    case EventType::OnMousePressed:     return "OnMousePressed";
    case EventType::OnMouseReleased:    return "OnMouseReleased";
    case EventType::OnMouseOver:        return "OnMouseOver";

    case EventType::OnCollisionEnter:   return "OnCollisionEnter";
    case EventType::OnCollisionStay:    return "OnCollisionStay";
    case EventType::OnCollisionExit:    return "OnCollisionExit";

    case EventType::OnTriggerEnter:     return "OnTriggerEnter";
    case EventType::OnTriggerStay:      return "OnTriggerStay";
    case EventType::OnTriggerExit:      return "OnTriggerExit";

        //case EventType::OnHealthZero: return "OnHealthZero";
        //case EventType::OnCheckpointReached: return "OnCheckpointReached";
    case EventType::OnGoalReached:      return "OnGoalReached";
    case EventType::OnFellOutOfMap:     return "OnFellOutOfMap";
    }
    return "Unknown";
}

// Convert string back to EventType 
static inline EventType StringToEventType(const std::string& s)
{
    if (s == "None") return EventType::None;

    if (s == "OnStart") return EventType::OnStart;
    if (s == "OnEnable") return EventType::OnEnable;
    if (s == "OnDisable") return EventType::OnDisable;
    if (s == "OnDestroy") return EventType::OnDestroy;

    if (s == "OnUpdate") return EventType::OnUpdate;

    if (s == "OnKeyPressed") return EventType::OnKeyPressed;
    if (s == "OnKeyReleased") return EventType::OnKeyReleased;
    if (s == "OnMousePressed") return EventType::OnMousePressed;
    if (s == "OnMouseReleased") return EventType::OnMouseReleased;
    if (s == "OnMouseOver") return EventType::OnMouseOver;

    if (s == "OnCollisionEnter") return EventType::OnCollisionEnter;
    if (s == "OnCollisionStay") return EventType::OnCollisionStay;
    if (s == "OnCollisionExit") return EventType::OnCollisionExit;

    if (s == "OnTriggerEnter") return EventType::OnTriggerEnter;
    if (s == "OnTriggerStay") return EventType::OnTriggerStay;
    if (s == "OnTriggerExit") return EventType::OnTriggerExit;

    //if (s == "OnHealthZero") return EventType::OnHealthZero;
    //if (s == "OnCheckpointReached") return EventType::OnCheckpointReached;
    if (s == "OnGoalReached") return EventType::OnGoalReached;
    if (s == "OnFellOutOfMap") return EventType::OnFellOutOfMap;

    return EventType::None;
}

static inline const char* ActionTypeToString(ActionType a)
{
    switch (a)
    {
    case ActionType::None:          return "None";
    case ActionType::Shrink:        return "Shrink";
    case ActionType::Grow:          return "Grow";
    case ActionType::ScaleTo:       return "ScaleTo";
    case ActionType::MoveTo:        return "MoveTo";
    case ActionType::TeleportTo:    return "TeleportTo";
    case ActionType::EnableObject:  return "EnableObject";
    case ActionType::DisableObject: return "DisableObject";
    case ActionType::DestroyObject: return "DestroyObject";
    case ActionType::PlaySound:     return "PlaySound";
    case ActionType::StopSound:     return "StopSound";
    case ActionType::LoadNextLevel: return "LoadNextLevel";
    case ActionType::Respawn:       return "Respawn";
    case ActionType::AddScore:      return "AddScore";
    case ActionType::TakeDamage:    return "TakeDamage";
    case ActionType::FlashColor:    return "FlashColor";
    }
    return "Unknown";
}

static inline ActionType StringToActionType(const std::string& s)
{
    if (s == " None")         return ActionType::None;
    if (s == "Shrink")        return ActionType::Shrink;
    if (s == "Grow")          return ActionType::Grow;
    if (s == "ScaleTo")       return ActionType::ScaleTo;
    if (s == "MoveTo")        return ActionType::MoveTo;
    if (s == "TeleportTo")    return ActionType::TeleportTo;
    if (s == "EnableObject")  return ActionType::EnableObject;
    if (s == "DisableObject") return ActionType::DisableObject;
    if (s == "DestroyObject") return ActionType::DestroyObject;
    if (s == "PlaySound")     return ActionType::PlaySound;
    if (s == "StopSound")     return ActionType::StopSound;
    if (s == "LoadNextLevel") return ActionType::LoadNextLevel;
    if (s == "Respawn")       return ActionType::Respawn;
    if (s == "AddScore")      return ActionType::AddScore;
    if (s == "TakeDamage")    return ActionType::TakeDamage;
    if (s == "FlashColor")    return ActionType::FlashColor;

    return ActionType::None;
}


struct EventRule
{
    EventType eventType = EventType::None;             // e.g., OnCollisionEnter
    std::string targetFilter = "";   // e.g., "Enemy", "Coin", "" leave empty to match ANY target
    ActionType action = ActionType::None;               // e.g., Shrink, DestroyObject
};

// EVENT COMPONENT -  Attached to object. Stores ONLY the selected event.
class EventComponent : public IComponent
{
public:
    std::vector<EventRule> rules;

    EventComponent(GameObject* owner) : IComponent(owner) {}
    EventComponent() : IComponent(nullptr) {}

    std::unique_ptr<IComponent> Clone(GameObject* NewClone) const override
    {
        auto clone = std::make_unique<EventComponent>(NewClone);
        clone->rules = rules;
        return clone;
    };

    std::string GetName() const override { return "EventComponent"; }
};

// EVENT PAYLOAD STRUCT - This is what gets queued when an event happens.
struct Event
{
    EventType type;
    GameObject* target;   // the receiver of event
    GameObject* other;    // object that caused event (for collision)
};

// GLOBAL EVENT SYSTEM (EVENT QUEUE) - Systems raise events by calling Raise().
class EventSystem {
public:
    static inline std::vector<Event> events;

    static void Raise(EventType type, GameObject* target, GameObject* other = nullptr) {
        std::cout << "[RAISE] " << EventTypeToString(type)
            << " | target: " << (target ? target->GetName() : "NULL")
            << " | other: " << (other ? other->GetName() : "NULL")
            << "\n";

        events.push_back({ type, target, other });
    }

    static void Clear() {
        events.clear();
    }
};

// EVENT DISPATCHER SYSTEM - This system processes all raised events each frame.
class EventSystemRunner : public ISystem {
public:
    void Initialize() override {}

    void Update(float /*dt*/) override {
        for (const Event& e : EventSystem::events)
        {
            std::cout << "[Event System Runner] e.target: "<< e.target <<"\n";
            if (!e.target) continue;

            std::cout << "[EventRunner] Calling OnEventTriggered("
                << EventTypeToString(e.type)
                << ") on " << e.target->GetName() << "\n";

            // Let GameObject handle rule matching and action execution
            e.target->OnEventTriggered(e.type, e.other);
        }

        EventSystem::Clear();
    }

    std::string GetName() override { return "EventSystemRunner"; };
};

// Can be used like
//class Coin : public GameObject {
//public:
//    void OnEventTriggered(EventType e) override {
//        if (e == EventType::OnCollisionEnter) {
//            PlaySound("coin_pickup");
//            SetActive(false);
//        }
//    }
//};
