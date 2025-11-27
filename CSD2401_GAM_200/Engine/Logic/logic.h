#pragma once
#include "Component.h"
#include "gameObject.h"
#include "MathLib.h"

#include <string>
#include <vector>
#include <memory>

class GameObject;

enum class EventType {
    None = 0,
    OnStart, OnEnable, OnDisable, OnDestroy,                // Lifecycle Events
    OnUpdate,                                               // Frame Events
    //OnKeyPressed, OnKeyReleased,                            // Input Events
    //OnMousePressed, OnMouseReleased, OnMouseOver,           // Mouse Events
    OnCollisionEnter, OnCollisionStay, OnCollisionExit,     // Physics Events
    OnTriggerEnter, OnTriggerStay, OnTriggerExit,           //
    //OnGoalReached, OnFellOutOfMap                           // Gameplay Events
};

enum class ActionType {
    None = 0,
    //Shrink, Grow, ScaleTo, MoveTo, TeleportTo,              // Transform Action
    EnableObject, DisableObject, DestroyObject,             // Object State Action
    PlaySound, StopSound,                                   // Audio Action
    //LoadNextLevel, 
    Respawn, AddScore, TakeDamage,           // Gameplay Action
    FlashColor                                              // Visual Action
};

// String Conversion
const char* EventTypeToString(EventType e);
EventType StringToEventType(const std::string& s);

const char* ActionTypeToString(ActionType a);;
ActionType StringToActionType(const std::string& s);

// Event Rule
struct EventRule {
    EventType eventType = EventType::None;

    // The ID of the OTHER object required to trigger this rule.
    int targetObjectId = -1; // -1 = Any
    ActionType action = ActionType::None;

    // Action Parameters
    float floatValue = 0.f;
    float floatValue2 = 0.f;
    float floatValue3 = 0.f;

    std::string stringValue = "";
    
    vec2 vec2Value = { 0,0 };
    vec4 colorValue = { 1, 1, 1, 1 };
};

class EventComponent : public IComponent {
public:
    std::vector<EventRule> rules;

    EventComponent(GameObject* owner);
    EventComponent();

    std::unique_ptr<IComponent> Clone(GameObject* newClone) const override;
    
    std::string GetName() const override { return "EventComponent"; }
};

// Event Payload
struct Event {
    EventType type;
    GameObject* target;
    GameObject* other;
};

class EventSystem {
public:
    static std::vector<Event> events;

    static void Raise(EventType type, GameObject* target, GameObject* other = nullptr);
    static void ProcessEvents();

};
