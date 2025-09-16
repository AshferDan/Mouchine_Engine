#include "InputManager.h"


// Global variables for input handling

void InputManager::keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (GLFW_PRESS == action) {
        if (key == GLFW_KEY_A) {
            keyA = (action == GLFW_PRESS || action == GLFW_REPEAT);
        }
        if (key == GLFW_KEY_D) {
            keyD = (action == GLFW_PRESS || action == GLFW_REPEAT);
        }
        if (key == GLFW_KEY_W) {
            keyW = (action == GLFW_PRESS || action == GLFW_REPEAT);
        }
        if (key == GLFW_KEY_S) {
            keyS = (action == GLFW_PRESS || action == GLFW_REPEAT);
        }
    }

    // ESC to close window
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, GLFW_TRUE);
    }
}
