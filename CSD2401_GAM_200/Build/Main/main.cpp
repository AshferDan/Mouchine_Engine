/*!
@file    	main.cpp
@author  	pghali@digipen.edu
@co-author  parminder.singh@digipen.edu
@date    	17/04/2024

This file uses functionality defined in type GLApp to initialize an OpenGL
context and implement a game loop.

*//*__________________________________________________________________________*/

/*                                                                   includes
----------------------------------------------------------------------------- */
// Extension loader library's header must be included before GLFW's header!!!
//#include "./graphics.h"
//#include "./core.h"
//#include "./factory.h"
//#include "./main.h"
//#include <iostream>
//
//void Main_init() {
//    // start with a 16:9 aspect ratio
//    GLApp::init(2400, 1350, "Tutorial 0 | Setting up OpenGL 4.5");
//}
//
// void Main_Update() {
//    GLApp::update();
//     
//}
//
// void Main_Unload() {
//     GLApp::cleanup();
// }
//
// 
// int main() {
//
//     Main_init();
//
//     while (!glfwWindowShouldClose(GLApp::ptr_window)) {
//         Main_Update();
//
//     }
//     Main_Unload();
// }

#include "core.h"
#include "factory.h"
#include "GameObjectSystem.h"
#include "transform.h"
#include "physics.h"
#include <iostream>
#include <GLFW/glfw3.h>

// Global variables for input handling
bool keyA = false;
bool keyD = false;
GLFWwindow* window = nullptr;

// Key callback function
void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_A) {
        keyA = (action == GLFW_PRESS || action == GLFW_REPEAT);
    }
    if (key == GLFW_KEY_D) {
        keyD = (action == GLFW_PRESS || action == GLFW_REPEAT);
    }

    // ESC to close window
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, GLFW_TRUE);
    }
}

// Function to draw a square at the given position
void DrawSquare(float x, float y, float size) {
    float halfSize = size / 2.0f;

    glBegin(GL_QUADS);
    glColor3f(1.0f, 0.0f, 0.0f); // Red color
    glVertex2f(x - halfSize, y - halfSize);
    glVertex2f(x + halfSize, y - halfSize);
    glVertex2f(x + halfSize, y + halfSize);
    glVertex2f(x - halfSize, y + halfSize);
    glEnd();
}

int main() {
    std::cout << "=== Component System Test with GLFW Input ===" << std::endl;

    // Initialize GLFW
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return -1;
    }

    // Create window
    window = glfwCreateWindow(800, 600, "Player Movement Demo", NULL, NULL);
    if (!window) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSetKeyCallback(window, keyCallback);

    CoreEngine engine;

    // Add the GameObject system
    engine.AddSystem<GameObjectSystem>();

    // Initialize
    std::cout << "\n--- Initializing Engine ---" << std::endl;
    engine.Init();

    // Create only the player
    std::cout << "\n--- Creating Player ---" << std::endl;
    auto& factory = GameObjectFactory::Instance();
    GameObject* player = factory.CreatePlayer("Hero", 0, 0, 0);

    std::cout << "\n--- Controls ---" << std::endl;
    std::cout << "A: Move left (2.0 units/sec)" << std::endl;
    std::cout << "D: Move right (2.0 units/sec)" << std::endl;
    std::cout << "ESC: Exit" << std::endl;

    // Game loop variables
    double lastTime = glfwGetTime();
    const float targetFPS = 60.0f;
    const float targetDeltaTime = 1.0f / targetFPS;

    // Set up orthographic projection
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(-10.0, 10.0, -10.0, 10.0, -1.0, 1.0); // 20x20 unit view
    glMatrixMode(GL_MODELVIEW);

    // Main game loop
    while (!glfwWindowShouldClose(window)) {
        // Calculate delta time
        double currentTime = glfwGetTime();
        float deltaTime = static_cast<float>(currentTime - lastTime);
        lastTime = currentTime;

        // Cap delta time to avoid large jumps
        if (deltaTime > 0.1f) {
            deltaTime = targetDeltaTime;
        }

        // Process input
        glfwPollEvents();

        // Handle player movement based on input
        if (auto* transform = player->GetComponent<TransformComponent>()) {
            float moveSpeed = 2.0f * deltaTime; // 2.0 units per second

            if (keyA) {
                transform->Move(-moveSpeed, 0.0f, 0.0f);
            }
            if (keyD) {
                transform->Move(moveSpeed, 0.0f, 0.0f);
            }
        }

        // Update all systems
        auto& gameObjects = factory.GetAllGameObjects();
        for (auto& obj : gameObjects) {
            if (obj->IsActive()) {
                obj->Update(deltaTime);
            }
        }

        // Rendering
        glClear(GL_COLOR_BUFFER_BIT);
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);

        // Draw the player as a square
        if (auto* transform = player->GetComponent<TransformComponent>()) {
            float x, y, z;
            transform->GetPosition(x, y, z);
            DrawSquare(x, y, 1.0f); // Draw 1x1 square at player position
        }

        // Swap buffers
        glfwSwapBuffers(window);

        // Sleep to maintain target FPS
        double endTime = glfwGetTime();
        double frameTime = endTime - currentTime;
        if (frameTime < targetDeltaTime) {
            double sleepTime = targetDeltaTime - frameTime;
            glfwWaitEventsTimeout(sleepTime);
        }
    }

    // Cleanup
    glfwDestroyWindow(window);
    glfwTerminate();

    std::cout << "\n=== Game Loop Complete ===" << std::endl;

    return 0;
}