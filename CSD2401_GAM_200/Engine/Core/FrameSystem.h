#pragma once 
#include "System.h"
#include "GraphicsSystem.h"
#include "RenderComponent.h"
class GraphicsSystem;
class FrameSystem :public ISystem {
    ~FrameSystem();

    void Initialize() override;
   
   //All systems are updated every game frame.
    void Update(float dt) override;

   //All systems provide a string name for debugging.
    std::string GetName() override;


private:
    static GLFWwindow* window;
    static GLdouble FPS;
    static GLdouble delta_time;
};