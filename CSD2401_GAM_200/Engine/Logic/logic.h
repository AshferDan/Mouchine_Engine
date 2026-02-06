/* Start Header ************************************************************************/
/*!
\file       logic.h
\author     Lee Hwee Min [100% - 54 lines]
\par        l.hweemin@digipen.edu
\date       November, 29, 2025

Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*/
/* End Header **************************************************************************/

#pragma once
#include "Component.h"
#include "MathLib.h"

#include <string>
#include <vector>
#include <memory>
#include <iostream>
#include <cstdlib>

class GameObject;
class GameObjectFactory;

/**
 * @brief All event types that a GameObject may react to.
 */
enum class EventType {
    None = 0,
    OnEnable, OnDisable, OnDestroy,                          // Lifecycle Events
    OnCollisionEnter, OnCollisionStay, OnCollisionExit,      // Physics Events
    //OnGoalReached                                          // Gameplay Events
};  

/**
 * @brief Actions executed when an EventRule triggers.
 */
enum class ActionType {
    None = 0,
    EnableObject, DisableObject, DestroyObject,             // Object State Action
    PlaySound, StopSound,                                   // Audio Action                                 
    Respawn,
};

/**
 * @brief Convert EventType to string for JSON serialization.
 */
const char* EventTypeToString(EventType e);
EventType StringToEventType(const std::string& s);

/**
 * @brief Convert ActionType to string for JSON serialization.
 */
const char* ActionTypeToString(ActionType a);;
ActionType StringToActionType(const std::string& s);


// ========== ACTION BASE CLASS ==========

/**
 * @brief Abstract Base Class for all Logic Actions.
 * This removes the need for giant switch statements in the game loop.
 */
class IAction {
public:
    virtual ~IAction() = default;

    virtual void Execute(GameObject* owner, GameObject* triggerInstigator) = 0;

    // For serialization/Editor
    virtual ActionType GetType() const = 0;

    virtual std::unique_ptr<IAction> Clone() const = 0;
};


// ========== ACTION CLASSES ==========

class ActionPlaySound : public IAction {
public:
    std::string soundName;

    ActionPlaySound(const std::string& name = "") : soundName(name) {}

    void Execute(GameObject* owner, GameObject* other) override;
    ActionType GetType() const override { return ActionType::PlaySound; }
    std::unique_ptr<IAction> Clone() const override { return std::make_unique<ActionPlaySound>(*this); }
};

class ActionDestroy : public IAction {
public:
    void Execute(GameObject* owner, GameObject* other) override;
    ActionType GetType() const override { return ActionType::DestroyObject; }
    std::unique_ptr<IAction> Clone() const override { return std::make_unique<ActionDestroy>(*this); }
};

class ActionToggleActive : public IAction {
public:
    bool setState; // true = enable, false = disable
    ActionToggleActive(bool state) : setState(state) {}

    void Execute(GameObject* owner, GameObject* other) override;
    ActionType GetType() const override { return setState ? ActionType::EnableObject : ActionType::DisableObject; }
    std::unique_ptr<IAction> Clone() const override { return std::make_unique<ActionToggleActive>(*this); }

};

class ActionRespawn : public IAction {
    float spawnX, spawnY;
public:
    ActionRespawn(float x = 0.0f, float y = 0.0f) : spawnX(x), spawnY(y) {}

    void Execute(GameObject* owner, GameObject* other) override;
    ActionType GetType() const override { return ActionType::Respawn; }
    std::unique_ptr<IAction> Clone() const override { return std::make_unique<ActionRespawn>(*this); }
};


// ========== lOGIC CONTAINER ==========

/**
 * @brief Represents a subscription to a specific event.
 * Contains a list of Actions to execute when that event fires.
 */
struct LogicChain {
    EventType triggerType = EventType::None;
    int targetFilterID = -1; // -1 = target any object

    // container of actions
    std::vector<std::unique_ptr<IAction>> actions;

    // helper - deep copy
    LogicChain() = default;
    LogicChain(const LogicChain& other);
    LogicChain& operator=(const LogicChain& other);
};

/**
 * @brief This component stores a list of controlled rules.
 * Whenever a GameObject receives an event, it checks these rules
 * to determine which actions to perform.
 */
class EventComponent : public IComponent {
public:
    //std::vector<EventRule> rules;
    // container of logic
    std::vector<LogicChain> chains;

    EventComponent(GameObject* owner);
    EventComponent();

    // process incoming event
    void OnEvent(EventType type, GameObject* other);

    std::unique_ptr<IComponent> Clone(GameObject* newClone) const override;
    std::string GetName() const override { return "EventComponent"; }
};
/**
 * @brief A queued gameplay event waiting to be processed.
 */
struct QueuedEvent {
    EventType type;
    GameObject* target;
    GameObject* other;
};

/**
 * @brief The EventSystem acts as a dispatcher
 */
class EventSystem {
public:
    static std::vector<QueuedEvent> events;

    static void Raise(EventType type, GameObject* target, GameObject* other = nullptr);
    static void ProcessEvents();
    static void Clear();
};
