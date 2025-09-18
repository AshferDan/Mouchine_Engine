#include "InputManager.h"

// Constructor
InputManager::InputManager()
    : window(nullptr)
{
}

// Initialize with GLFW window and set callback
void InputManager::initialize(GLFWwindow* win) {
    window = win;
    glfwSetWindowUserPointer(window, this);
    glfwSetKeyCallback(window, keyCallback);
}

// Bind a key code to a game action
void InputManager::bindKey(int keyCode, InputAction const& action) {
    bindings[keyCode] = action;
}

// Update key states (Pressed -> Held, Released -> Idle)
void InputManager::update() {
    for (std::unordered_map<int, KeyState>::iterator it = keyStates.begin(); it != keyStates.end(); ++it) {
        KeyState& state = it->second;
        if (state == KeyState::Pressed)
            state = KeyState::Held;
        else if (state == KeyState::Released)
            state = KeyState::Idle;
    }
}

// Query helpers
bool InputManager::isPressed(InputAction const& action) const {
    return check(action, KeyState::Pressed);
}

bool InputManager::isHeld(InputAction const& action) const {
    return check(action, KeyState::Held);
}

bool InputManager::isReleased(InputAction const& action) const {
    return check(action, KeyState::Released);
}

// Internal check function
bool InputManager::check(InputAction const& action, KeyState const& desired) const {
    for (std::unordered_map<int, InputAction>::const_iterator it = bindings.begin(); it != bindings.end(); ++it) {
        int const& key = it->first;
        InputAction const& boundAction = it->second;

        if (boundAction == action) {
            std::unordered_map<int, KeyState>::const_iterator sit = keyStates.find(key);
            if (sit != keyStates.end() && sit->second == desired)
                return true;
        }
    }
    return false;
}

// GLFW callback
void InputManager::keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    InputManager* self = static_cast<InputManager*>(glfwGetWindowUserPointer(window));
    if (!self) return;

    KeyState& state = self->keyStates[key]; // create if it doesn't exist
    if (action == GLFW_PRESS) {
        if (state == KeyState::Idle || state == KeyState::Released)
            state = KeyState::Pressed;
        else
            state = KeyState::Held;
    }
    else if (action == GLFW_RELEASE) {
        state = KeyState::Released;
    }
}