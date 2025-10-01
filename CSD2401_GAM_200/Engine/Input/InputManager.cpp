#include "inputmanager.h"
#include <iostream>
#include <unordered_map>

InputManager::InputManager(GLFWwindow *window) : window(window), width(0), height(0), mouseX(0.0), mouseY(0.0), scrollX(0.0), scrollY(0.0)
{
    glfwSetWindowUserPointer(window, this);
}

InputManager::~InputManager() {}

void InputManager::Initialize()
{
    glfwGetWindowSize(window, &width, &height);

    // ImGui_ImplGlfw_InitForOpenGL(window, true);
    // glfwSetKeyCallback(window, InputManager::key_cb);
    // glfwSetMouseButtonCallback(window, InputManager::mousebutton_cb);
    // glfwSetCursorPosCallback(window, InputManager::mousepos_cb);
    // glfwSetScrollCallback(window, InputManager::mousescroll_cb);
}

void InputManager::Update(float dt)
{
    static std::unordered_map<int, bool> singleTriggerKeys = {
        {GLFW_KEY_SPACE, false},
        {GLFW_KEY_F1, false}};

    ImGuiIO &io = ImGui::GetIO();

    if (io.WantCaptureKeyboard)
    {
        return;
    }

    auto *player = GameObjectFactory::Instance().FindGameObject("Hero");
    if (!player)
    {
        return;
    }

    auto *physics = player->GetComponent<PhysicsComponent>();
    if (!physics)
    {
        return;
    }

    TransformComponent *transform = player->GetComponent<TransformComponent>();
    if (!transform)
    {
        return;
    }

    float vx, vy;
    physics->GetVelocity(vx, vy);



    //---------------NEW SYSTEM USING PHYSICS FOR MOVEMENT-------------------//
    // Horizontal movement
    float moveSpeed = 5.0f;  // Movement speed

    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
        vx = -moveSpeed;
    }
    else if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
        vx = moveSpeed;
    }
    else {
        // No horizontal input - slow down
        vx = 0.0f;
    }

    // Jump
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
        // Only jump if grounded
        if (physics->IsGrounded()) {
            physics->Jump(10.0f);  // Jump force
            physics->GetVelocity(vx, vy);
        }
    }

    // Apply the horizontal velocity (vertical is handled by gravity/jump)
    physics->SetVelocity(vx, vy);

    //Mouse
    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
        std::cout << "Left Mouse Press, X:" << mouseX << " Y:" << mouseY << std::endl;
    }
    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS) {
        std::cout << "Right Mouse Press" << mouseX << " Y:" << mouseY << std::endl;
    }

    double xpos, ypos;
    glfwGetCursorPos(window, &xpos, &ypos);
    mouseX = xpos;
    mouseY = ypos;

    //scroll not added
}

std::string InputManager::GetName() {
    return "Input System";
}


    //-----------------OLD SYSTEM USING TRANSFORM FOR MOVEMENT -----------------//
//
// if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
//    std::cout << "UP" << std::endl;
//    y += movementspeed;
//}
// if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
//    std::cout << "LEFT" << std::endl;
//    x -= movementspeed;
//}
// if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
//    std::cout << "RIGHT" << std::endl;
//    x += movementspeed;
//}
// if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
//    std::cout << "DOWN" << std::endl;
//    y -= movementspeed;
//}

// if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
//     if (!isJumping) {
//         isJumping = true;
//         jumpStartY = y;
//     }

//    if (y < jumpStartY + jumpMaxHeight) {
//        y += jumpSpeed * dt;
//        if (y > jumpStartY + jumpMaxHeight) {
//            y = jumpStartY + jumpMaxHeight;
//        }
//    }
//}
// else {
//    isJumping = false;
//}

// transform->SetPosition(x, y);
    // Key callback: handles held keys(long press) and single-trigger actions
    // void InputManager::key_cb(GLFWwindow* pwin, int key, int, int action, int) {
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
    // void InputManager::mousebutton_cb(GLFWwindow*, int button, int action, int) {
    //     if (action == GLFW_PRESS || action == GLFW_REPEAT) {
    //         switch (button) {
    //         case GLFW_MOUSE_BUTTON_LEFT:
    //             std::cout << "Left mouse action" << std::endl;
    //             break;
    //         case GLFW_MOUSE_BUTTON_RIGHT:
    //             std::cout << "Right mouse action" << std::endl;
    //             break;
    //         }
    //     }
    // }
    //
    //// Mouse position
    // void InputManager::mousepos_cb(GLFWwindow*, double xpos, double ypos) {
    //     mouseX = xpos;
    //     mouseY = ypos;
    // }
    //
    //// Scroll
    // void InputManager::mousescroll_cb(GLFWwindow*, double xoffset, double yoffset) {
    //     scrollX = xoffset;
    //     scrollY = yoffset;
    // }
