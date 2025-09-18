#pragma once
#include <unordered_map>
#include <GLFW/glfw3.h>

// Represents the current state of a key
enum class KeyState {
    Idle,
    Pressed,
    Held,
    Released
};

// Represents game actions
enum class InputAction {
    None = 0,
    MoveLeft,
    MoveRight,
    Jump,
    Dash,
    Pause
};

class InputManager {
public:
    InputManager();
    ~InputManager() = default;

    void initialize(GLFWwindow* window);

    void bindKey(int keyCode, InputAction const& action);

    void update();

    bool isPressed(InputAction const& action) const;
    bool isHeld(InputAction const& action) const;
    bool isReleased(InputAction const& action) const;

private:
    GLFWwindow* window;
    std::unordered_map<int, InputAction> bindings; // keyCode -> action
    std::unordered_map<int, KeyState> keyStates;   // keyCode -> state

    // Helper to check if an action is in a desired state
    bool check(InputAction const& action, KeyState const& desired) const;

    static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
};
