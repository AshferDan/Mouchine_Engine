#include "inputmanager.h"
#include <iostream>
#include <unordered_map>

//GLint InputManager::width = 800;
//GLint InputManager::height = 600;
//std::string InputManager::title = "InputManager Window";
//GLFWwindow* InputManager::ptr_window = nullptr;
//
//double InputManager::mouseX = 0.0;
//double InputManager::mouseY = 0.0;
//double InputManager::scrollX = 0.0;
//double InputManager::scrollY = 0.0;

InputManager::InputManager(GLFWwindow* window) : window(window), width(0), height(0), mouseX(0.0), mouseY(0.0), scrollX(0.0), scrollY(0.0){
    glfwSetWindowUserPointer(window, this);
}

InputManager::~InputManager() {}

void InputManager::Initialize() {
    glfwGetWindowSize(window, &width, &height);

    glfwSetKeyCallback(window, InputManager::key_cb);
    glfwSetMouseButtonCallback(window, InputManager::mousebutton_cb);
    glfwSetCursorPosCallback(window, InputManager::mousepos_cb);
    glfwSetScrollCallback(window, InputManager::mousescroll_cb);
}

void InputManager::Update(float dt) {
    //empty
}

void InputManager::key_cb(GLFWwindow* pwin, int key, int, int action, int) {
    auto* self = static_cast<InputManager*>(glfwGetWindowUserPointer(pwin));

    if (self) {
        self->HandleKey(key, action);
    }
}

void InputManager::mousebutton_cb(GLFWwindow* pwin, int button, int action, int) {
    auto* self = static_cast<InputManager*>(glfwGetWindowUserPointer(pwin));

    if (self) {
        self->HandleMouseButton(button, action);
    }
}

void InputManager::mousepos_cb(GLFWwindow* pwin, double xpos, double ypos) {
    auto* self = static_cast<InputManager*>(glfwGetWindowUserPointer(pwin));

    if (self) {
        self->HandleMouseButton(xpos, ypos);
    }
}

void InputManager::mousescroll_cb(GLFWwindow* pwin, double xoffset, double yoffset) {
    auto* self = static_cast<InputManager*>(glfwGetWindowUserPointer(pwin));

    if (self) {
        self->HandleScroll(xoffset, yoffset);
    }
}

void InputManager::HandleKey(int key, int action) {
    static std::unordered_map<int, bool> singleTriggerKeys = {
        {GLFW_KEY_SPACE, false},
        {GLFW_KEY_ESCAPE, false}
    };

    auto it = singleTriggerKeys.find(key);
    if (it != singleTriggerKeys.end()) {
        if (action == GLFW_PRESS && !it->second) {
            if (key == GLFW_KEY_SPACE) {
                std::cout << "Jump Triggered" << std::endl;
                //add set position of player object
            }
            if (key == GLFW_KEY_P) {
                std::cout << "Pause toggled." << std::endl;
            }
        }
        else if(action == GLFW_RELEASE) {
            it->second = false;
        }
        return;
    }

    if (action == GLFW_PRESS || action == GLFW_REPEAT) {
        switch (key) {
        case GLFW_KEY_W:
            std::cout << "UP" << std::endl;
            break;
        case GLFW_KEY_A:
            std::cout << "LEFT" << std::endl;
            break;
        case GLFW_KEY_D:
            std::cout << "RIGHT" << std::endl;
            break;
        case GLFW_KEY_ESCAPE:
            glfwSetWindowShouldClose(window, GLFW_TRUE);
            break;
        }
    }
}

void InputManager::HandleMouseButton(int button, int action) {
    if (action == GLFW_PRESS || action == GLFW_REPEAT) {
        if (button == GLFW_MOUSE_BUTTON_LEFT) {
            std::cout << "Left mouse action" << std::endl;
        }
        if (button == GLFW_MOUSE_BUTTON_RIGHT) {
            std::cout << "Right mouse button" << std::endl;
        }
    }
}

void InputManager::HandleMouseMove(double xpos, double ypos) {
    mouseX = xpos;
    mouseY = ypos;
}


void InputManager::HandleScroll(double xoffset, double yoffset) {
    scrollX = xoffset;
    scrollY = yoffset;
}


// Key callback: handles held keys(long press) and single-trigger actions
//void InputManager::key_cb(GLFWwindow* pwin, int key, int, int action, int) {
//    // ----- Map for single-trigger keys (key -> triggered flag) -----
//    static std::unordered_map<int, bool> singleTriggerKeys = {
//        {GLFW_KEY_SPACE, false}, // jump
//        {GLFW_KEY_P, false}      // pause
//    };
//
//    // Check if this key is a single-trigger key
//    std::unordered_map<int, bool>::iterator it = singleTriggerKeys.find(key);
//    if (it != singleTriggerKeys.end()) {
//        if (action == GLFW_PRESS && !(it->second)) {
//            switch (key) {
//            case GLFW_KEY_SPACE:
//                std::cout << "Jump triggered!" << std::endl;
//                break;
//            case GLFW_KEY_P:
//                std::cout << "Pause toggled!" << std::endl;
//                break;
//            }
//            it->second = true; // mark as triggered
//        }
//        else if (action == GLFW_RELEASE) {
//            it->second = false; // reset flag
//        }
//        return; // exit early
//    }
//
//    // ----- Continuous actions (held) -----
//    if (action == GLFW_PRESS || action == GLFW_REPEAT) {
//        switch (key) {
//        case GLFW_KEY_W:
//            std::cout << "Move forward (held or pressed)" << std::endl;
//            break;
//        case GLFW_KEY_A:
//            std::cout << "Move left (held or pressed)" << std::endl;
//            break;
//        case GLFW_KEY_D:
//            std::cout << "Move right (held or pressed)" << std::endl;
//            break;
//        case GLFW_KEY_ESCAPE:
//            glfwSetWindowShouldClose(pwin, GLFW_TRUE);
//            break;
//        }
//    }
//}
//
//// Mouse buttons
//void InputManager::mousebutton_cb(GLFWwindow*, int button, int action, int) {
//    if (action == GLFW_PRESS || action == GLFW_REPEAT) {
//        switch (button) {
//        case GLFW_MOUSE_BUTTON_LEFT:
//            std::cout << "Left mouse action" << std::endl;
//            break;
//        case GLFW_MOUSE_BUTTON_RIGHT:
//            std::cout << "Right mouse action" << std::endl;
//            break;
//        }
//    }
//}
//
//// Mouse position
//void InputManager::mousepos_cb(GLFWwindow*, double xpos, double ypos) {
//    mouseX = xpos;
//    mouseY = ypos;
//}
//
//// Scroll
//void InputManager::mousescroll_cb(GLFWwindow*, double xoffset, double yoffset) {
//    scrollX = xoffset;
//    scrollY = yoffset;
//}
