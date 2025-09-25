#ifndef INPUTMANAGER_H
#define INPUTMANAGER_H

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <string>

struct InputManager
{
    // Initialization
    static bool init(GLint width, GLint height, std::string title);
    static void cleanup();

    // Callbacks
    static void key_cb(GLFWwindow* pwin, int key, int scancode, int action, int mods);
    static void mousebutton_cb(GLFWwindow* pwin, int button, int action, int mods);
    static void mousepos_cb(GLFWwindow* pwin, double xpos, double ypos);
    static void mousescroll_cb(GLFWwindow* pwin, double xoffset, double yoffset);
    static void setup_event_callbacks();

    // Window info
    static GLint width, height;
    static std::string title;
    static GLFWwindow* ptr_window;

    // Mouse position / scroll
    static double mouseX, mouseY;
    static double scrollX, scrollY;
};

#endif