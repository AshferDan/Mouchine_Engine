#include "inputmanager.h"
#include <iostream>
#include <unordered_map>
#include "Animator.hpp"
#include "core.h"
InputManager::InputManager(GLFWwindow *window, AudioEngine* audio) : window(window), audioEngine(audio), width(0), height(0), mouseX(0.0), mouseY(0.0), scrollX(0.0), scrollY(0.0)
{
    glfwSetWindowUserPointer(window, this);

    if (audioEngine) {
        std::cout << "InputManager: AudioEngine successfully linked" << std::endl;
    }
    else {
        std::cout << "InputManager: WARNING - AudioEngine is NULL!" << std::endl;
    }
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

void InputManager::Update(float)
{
    static std::unordered_map<int, bool> singleTriggerKeys = {
        {GLFW_KEY_SPACE, false},
        {GLFW_KEY_F1, false},
        {GLFW_KEY_M, false}, // toggle pause (for now)
    };

    ImGuiIO &io = ImGui::GetIO();

    if (io.WantCaptureKeyboard)
    {
        return;
    }

    // handle M key for pausing and resuming background music
    static bool bgmPaused = false;

    if (glfwGetKey(window, GLFW_KEY_M) == GLFW_PRESS) {
        if (!singleTriggerKeys[GLFW_KEY_M]) {
            if (audioEngine) {
                bgmPaused = !bgmPaused;
                audioEngine->pauseSound("bgm_channel", bgmPaused);
                std::cout << "bgm paused" << std::endl;
            }
            singleTriggerKeys[GLFW_KEY_M] = true;
        }
    }

    for (auto &kv : singleTriggerKeys)
    {
        int key = kv.first;
        bool &wasPressed = kv.second;

        if (glfwGetKey(window, key) == GLFW_PRESS)
        {
            if (!wasPressed)
            {
                if (key == GLFW_KEY_F1)
                {
                    CORE->GetGraphicsSystem()->toggleDebugMode();
                    std::cout << "Dubug mode toggled" << CORE->GetGraphicsSystem()->isDebugModeOn() << std::endl;
                }
            }
            wasPressed = true;
        }
        else
        {
            wasPressed = false;
        }
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

    if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS)
    {
        DemoM1_SpawnGameObjects();
    }
    if (glfwGetKey(window, GLFW_KEY_ENTER) == GLFW_PRESS)
    {
        DemoM1_DestroyGameObjects();
    }

    //---------------NEW SYSTEM USING PHYSICS FOR MOVEMENT-------------------//
    // Horizontal movement
    float moveSpeed = 200.0f; // Movement speed
    float moveDirectionX = 0.0f; // Track intended direction (1=Right, -1=Left)
    bool is_jumping= false;
   
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
    {
        vx = -moveSpeed;
     
        moveDirectionX = -1.0f;
    }
    else if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
    {
        vx = moveSpeed;
       
        moveDirectionX = 1.0f;
    }
    else
    {
   
        vx = 0.0f;
    }

    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        KeyStateQuit = true;
        glfwSetWindowShouldClose(window, GLFW_TRUE);
    }
    // Jump
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
    {
        // Only jump if grounded
        is_jumping = true;
        if (physics->IsGrounded())
        {
            is_jumping = false;
            physics->Jump(400.0f); // Jump force
            physics->GetVelocity(vx, vy); // Get updated VY after jump
            // Play jump sound effect
            if (audioEngine) {
                audioEngine->playSound("jump_sfx", "jump_channel");
            }
        }
    }

    auto* anim = player->GetComponent<Spritesheet>();

    if (transform && anim) {

        // --- A. HORIZONTAL FLIP LOGIC ---
        // ... (Your flip logic is fine here) ...
        if (std::abs(moveDirectionX) > 0.0f) {
            float baseWidth = std::abs(transform->scaleX);
            float newScaleX = (moveDirectionX < 0) ? -baseWidth : baseWidth;
            transform->SetScale(newScaleX, transform->scaleY);
        }

        // --- B. ANIMATION SWITCHING LOGIC (Idling vs Running) ---

        // **Animation Constants - CORRECTED END FRAME INDEX (0 and 1 for 2 columns)**
        const char* IDLE_SS = "./Assets/Mouch_Idle.png";
        int _ROWS = 1;
        int _COLS = 2;
        const int FRAME_START = 0;
        const int FRAME_END = 1; 
        const float IDLE_FPS = 2.0f;
        const float RUN_FPS = 7.0f;
        const char* RUN_SS = "./Assets/Mouch_Running.png";
        const char* JUMP_SS = "./Assets/Mouch_jump.png";
        // --------------------------------------------------------

        float newFPS;
        const char* newSpritesheet;

        if (std::abs(vx) > 0.0f) // Player is running/moving
        {
            newFPS = RUN_FPS;
            newSpritesheet = RUN_SS;
        }
        else if (is_jumping) {
            _COLS = 1;
            newFPS = 0;
            newSpritesheet = JUMP_SS;
        }
        else // Player is stopped (vx is 0)
        {
            newFPS = IDLE_FPS;
            newSpritesheet = IDLE_SS;
        }

        float desiredTimePerFrame = 1.0f / newFPS;

        
        if (std::abs(anim->GetTimePerFrame() - desiredTimePerFrame) > 0.0001f)
        {
            // 1. Load the new texture (This is the crucial step that was being skipped)
            anim->LoadNewSpritesheet(newSpritesheet, _ROWS, _COLS);

            // 2. Set the new animation speed and range
            anim->SetAnimation(FRAME_START, FRAME_END, newFPS);
        }
    }

    // Apply the horizontal velocity (vertical is handled by gravity/jump)
    physics->SetVelocity(vx, vy);

    // Mouse
    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS)
    {
        std::cout << "Left Mouse Press, X:" << mouseX << " Y:" << mouseY << std::endl;
    }
    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS)
    {
        std::cout << "Right Mouse Press" << mouseX << " Y:" << mouseY << std::endl;
    }

    double xpos, ypos;
    glfwGetCursorPos(window, &xpos, &ypos);
    mouseX = xpos;
    mouseY = ypos;

    // scroll not added
}

std::string InputManager::GetName()
{
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
