#ifndef INPUTMANAGER_H
#define INPUTMANAGER_H

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <string>
#include "System.h"
#include "factory.h"
#include "transform.h"

class InputManager : public ISystem
{
public:
    InputManager(GLFWwindow* window);
    ~InputManager();

    void Initialize();
    void Update(float dt) override;
    std::string GetName() override;

    //State getters
    double GetMouseX() const { return mouseX; }
    double GetMouseY() const { return mouseY; }
    double GetScrollX() const { return scrollX; }
    double GetScrollY() const { return scrollY; }

private:

    // Callbacks
    static void key_cb(GLFWwindow* pwin, int key, int scancode, int action, int mods);
    static void mousebutton_cb(GLFWwindow* pwin, int button, int action, int mods);
    static void mousepos_cb(GLFWwindow* pwin, double xpos, double ypos);
    static void mousescroll_cb(GLFWwindow* pwin, double xoffset, double yoffset);

    void HandleKey(int key, int action);
    void HandleMouseButton(int button, int action);
    void HandleMouseMove(double xpos, double ypos);
    void HandleScroll(double xoffset, double yoffset);

private:
    // Window info
    GLFWwindow* window;
    int width, height;
    std::string title; //dont think we need this

    // Mouse position / scroll
    double mouseX, mouseY;
    double scrollX, scrollY;
};

#endif