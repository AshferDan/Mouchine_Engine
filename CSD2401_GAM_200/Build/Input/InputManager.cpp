#include "inputmanager.h"
#include <iostream>
#include <unordered_map>

GLint InputManager::width = 800;
GLint InputManager::height = 600;
std::string InputManager::title = "InputManager Window";
GLFWwindow* InputManager::ptr_window = nullptr;

double InputManager::mouseX = 0.0;
double InputManager::mouseY = 0.0;
double InputManager::scrollX = 0.0;
double InputManager::scrollY = 0.0;

bool InputManager::init(GLint w, GLint h, std::string t) {
    width = w; height = h; title = t;

    if (!glfwInit()) {
        std::cerr << "GLFW init failed!" << std::endl;
        return false;
    }

    ptr_window = glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr);
    if (!ptr_window) {
        std::cerr << "GLFW unable to create window!" << std::endl;
        glfwTerminate();
        return false;
    }

    glfwMakeContextCurrent(ptr_window);
    setup_event_callbacks();

    return true;
}

void InputManager::cleanup() {
    glfwTerminate();
}

void InputManager::setup_event_callbacks() {
    glfwSetKeyCallback(ptr_window, InputManager::key_cb);
    glfwSetMouseButtonCallback(ptr_window, InputManager::mousebutton_cb);
    glfwSetCursorPosCallback(ptr_window, InputManager::mousepos_cb);
    glfwSetScrollCallback(ptr_window, InputManager::mousescroll_cb);
}

// Key callback: handles held keys(long press) and single-trigger actions
void InputManager::key_cb(GLFWwindow* pwin, int key, int, int action, int) {
    // ----- Map for single-trigger keys (key -> triggered flag) -----
    static std::unordered_map<int, bool> singleTriggerKeys = {
        {GLFW_KEY_SPACE, false}, // jump
        {GLFW_KEY_P, false}      // pause
    };

    // Check if this key is a single-trigger key
    std::unordered_map<int, bool>::iterator it = singleTriggerKeys.find(key);
    if (it != singleTriggerKeys.end()) {
        if (action == GLFW_PRESS && !(it->second)) {
            switch (key) {
            case GLFW_KEY_SPACE:
                std::cout << "Jump triggered!" << std::endl;
                break;
            case GLFW_KEY_P:
                std::cout << "Pause toggled!" << std::endl;
                break;
            }
            it->second = true; // mark as triggered
        }
        else if (action == GLFW_RELEASE) {
            it->second = false; // reset flag
        }
        return; // exit early
    }

    // ----- Continuous actions (held) -----
    if (action == GLFW_PRESS || action == GLFW_REPEAT) {
        switch (key) {
        case GLFW_KEY_W:
            std::cout << "Move forward (held or pressed)" << std::endl;
            break;
        case GLFW_KEY_A:
            std::cout << "Move left (held or pressed)" << std::endl;
            break;
        case GLFW_KEY_D:
            std::cout << "Move right (held or pressed)" << std::endl;
            break;
        case GLFW_KEY_ESCAPE:
            glfwSetWindowShouldClose(pwin, GLFW_TRUE);
            break;
        }
    }
}

// Mouse buttons
void InputManager::mousebutton_cb(GLFWwindow*, int button, int action, int) {
    if (action == GLFW_PRESS || action == GLFW_REPEAT) {
        switch (button) {
        case GLFW_MOUSE_BUTTON_LEFT:
            std::cout << "Left mouse action" << std::endl;
            break;
        case GLFW_MOUSE_BUTTON_RIGHT:
            std::cout << "Right mouse action" << std::endl;
            break;
        }
    }
}

// Mouse position
void InputManager::mousepos_cb(GLFWwindow*, double xpos, double ypos) {
    mouseX = xpos;
    mouseY = ypos;
}

// Scroll
void InputManager::mousescroll_cb(GLFWwindow*, double xoffset, double yoffset) {
    scrollX = xoffset;
    scrollY = yoffset;
}
