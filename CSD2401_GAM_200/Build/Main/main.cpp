#include <iostream>
#include <GLFW/glfw3.h>
#include "core.h"
#include "factory.h"
#include "GameObjectSystem.h"
#include "transform.h"
#include "physics.h"
#include "InputManager.h"


GLFWwindow* window = nullptr;


// Key callback function

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
    std::cout << "=== 2D Component System Test ===" << std::endl;

    // Initialize GLFW
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return -1;
    }

    // Create window
    window = glfwCreateWindow(800, 600, "2D Player Movement", NULL, NULL);
    if (!window) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSetKeyCallback(window, InputManager::keyCallback);

    CoreEngine engine;
    engine.AddSystem<GameObjectSystem>();
    engine.Init();

    // Create player
    auto& factory = GameObjectFactory::Instance();
    GameObject* player = factory.CreatePlayer("Hero", 0, 0, 0);

    std::cout << "\n--- Controls ---" << std::endl;
    std::cout << "A: Move left (2.0 units/sec)" << std::endl;
    std::cout << "D: Move right (2.0 units/sec)" << std::endl;
    std::cout << "ESC: Exit" << std::endl;

    // Game loop variables
    double lastTime = glfwGetTime();
    const float targetDeltaTime = 1.0f / 60.0f;

    // Set up 2D orthographic projection (no Z-axis needed)
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(-10.0, 10.0, -10.0, 10.0, 0.0, 1.0); // Near=0, Far=1 (2D only)
    glMatrixMode(GL_MODELVIEW);

    // Main game loop
    while (!glfwWindowShouldClose(window)) {
        float deltaTime = static_cast<float>(glfwGetTime() - lastTime);
        lastTime = glfwGetTime();

        if (deltaTime > 0.1f) deltaTime = targetDeltaTime;

        glfwPollEvents();

        // Handle player movement (2D only - X axis)
        if (auto* transform = player->GetComponent<TransformComponent>()) {
            float moveSpeed = 5.0f * deltaTime;

            if (InputManager::keyA) transform->Move(-moveSpeed, 0.0f, 0.0f);
            if (InputManager::keyD) transform->Move(moveSpeed, 0.0f, 0.0f);
            if (InputManager::keyW) transform->Move(0.0f, moveSpeed, 0.0f);
            if (InputManager::keyS) transform->Move(0.0f, -moveSpeed, 0.0f);
        }

        // Update game objects
        for (auto& obj : factory.GetAllGameObjects()) {
            if (obj->IsActive()) obj->Update(deltaTime);
        }

        // Rendering
        glClear(GL_COLOR_BUFFER_BIT);
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);

        // Draw player (2D only)
        if (auto* transform = player->GetComponent<TransformComponent>()) {
            float x, y, z;
            transform->GetPosition(x, y, z);
            DrawSquare(x, y, 1.0f);
        }

        glfwSwapBuffers(window);

        // Maintain frame rate
        double frameTime = glfwGetTime() - lastTime;
        if (frameTime < targetDeltaTime) {
            glfwWaitEventsTimeout(targetDeltaTime - frameTime);
        }
    }

    // Cleanup
    glfwDestroyWindow(window);
    glfwTerminate();
    std::cout << "\n=== Game Complete ===" << std::endl;
    return 0;
}