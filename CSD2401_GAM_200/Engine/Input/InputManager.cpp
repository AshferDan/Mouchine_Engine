/* Start Header ************************************************************************/
/*!
\file       InputManager.cpp
\author     Lee Hwee Min [100% - 181 lines]
\par        l.hweemin@digipen.edu
\date       October, 2, 2025
Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*/
/* End Header **************************************************************************/

#include "inputmanager.h"
#include <iostream>
#include <unordered_map>
#include "Animator.hpp"
#include "core.h"

bool InputManager::KeyStateImgui = true;
/**
 * @brief Constructs an InputManager with specified parameters.
 * @param window Pointer to the GLFW window.
 * @param audio Optional pointer to the AudioEngine.
 */
InputManager::InputManager(GLFWwindow *window, AudioEngine *audio) : window(window), audioEngine(audio), width(0), height(0), mouseX(0.0), mouseY(0.0), scrollX(0.0), scrollY(0.0)
{
    glfwSetWindowUserPointer(window, this);

    if (audioEngine)
    {
        std::cout << "InputManager: AudioEngine successfully linked" << std::endl;
    }
    else
    {
        std::cout << "InputManager: WARNING - AudioEngine is NULL!" << std::endl;
    }
}

/**
 * @brief Destructor for InputManager.
 */
InputManager::~InputManager() {}

/**
 * @brief Initializes the InputManager by fetching window size.
 */
void InputManager::Initialize()
{
    glfwGetWindowSize(window, &width, &height);
}

/**
 * @brief Updates input handling, including key presses, mouse events,
 *        player movement, jumping, animations, and quitting logic.
 * @param dt Delta time since the last frame.
 */
void InputManager::Update(float dt)
{
    static std::unordered_map<int, bool> singleTriggerKeys = {
        {GLFW_KEY_SPACE, false},
        {GLFW_KEY_F1, false},
        {GLFW_KEY_M, false}, // toggle pause (for now)
    };

    // handle M key for pausing and resuming background music
    static bool bgmPaused = false;

    if (glfwGetKey(window, GLFW_KEY_M) == GLFW_PRESS)
    {
        if (!singleTriggerKeys[GLFW_KEY_M])
        {
            if (audioEngine)
            {
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

    /**
     * @brief Destroy 2500 Game Objects on screen, if ENTER key is pressed
     */
    if (glfwGetKey(window, GLFW_KEY_ENTER) == GLFW_PRESS)
    {
        DemoM1_DestroyGameObjects();
    }

    //---------------NEW SYSTEM USING PHYSICS FOR MOVEMENT-------------------//
    // Horizontal movement
    float moveSpeed = 200.0f;    // Movement speed
    float moveDirectionX = 0.0f; // Track intended direction (1=Right, -1=Left)
    bool is_jumping = false;

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

    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    {
        KeyStateQuit = true;
        glfwSetWindowShouldClose(window, GLFW_TRUE);
    }
    /**
        * @brief Toggle on and off imgui
        */
    if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS)
    {
        InputManager::KeyStateImgui= (!InputManager::KeyStateImgui) ? true : false;
        std::cout << InputManager::KeyStateImgui << std::endl;

    }
    // Jump
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
    {
        // Only jump if grounded
        is_jumping = true;
        if (physics->IsGrounded())
        {
            is_jumping = false;
            physics->Jump(400.0f);        // Jump force
            physics->GetVelocity(vx, vy); // Get updated VY after jump
            // Play jump sound effect
            if (audioEngine)
            {
                audioEngine->playSound("jump_sfx", "jump_channel");
            }
        }
    }

    auto *anim = player->GetComponent<Spritesheet>();

    if (transform && anim)
    {

        if (std::abs(moveDirectionX) > 0.0f)
        {
            float baseWidth = std::abs(transform->scaleX);
            float newScaleX = (moveDirectionX < 0) ? -baseWidth : baseWidth;
            transform->SetScale(newScaleX, transform->scaleY);
        }

        const char *IDLE_SS = "./Assets/Mouch_Idle.png";
        int _ROWS = 1;
        int _COLS = 2;
        const int FRAME_START = 0;
        const int FRAME_END = 1;
        const float IDLE_FPS = 2.0f;
        const float RUN_FPS = 7.0f;
        const char *RUN_SS = "./Assets/Mouch_Running.png";
        const char *JUMP_SS = "./Assets/Mouch_jump.png";
        // --------------------------------------------------------

        float newFPS;
        const char *newSpritesheet;

        if (std::abs(vx) > 0.0f) // Player is running/moving
        {
            newFPS = RUN_FPS;
            newSpritesheet = RUN_SS;
        }
        else if (is_jumping)
        {
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

            anim->LoadNewSpritesheet(newSpritesheet, _ROWS, _COLS);

            anim->SetAnimation(FRAME_START, FRAME_END, newFPS);
        }
    }

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
}

/**
 * @brief Gets the system name.
 * @return The string "Input System".
 */
std::string InputManager::GetName()
{
    return "Input System";
}
