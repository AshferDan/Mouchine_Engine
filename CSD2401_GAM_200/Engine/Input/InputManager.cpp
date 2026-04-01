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
#include "Serialization.h"
#include "IMessageHandler.h"
#include "LevelEditor.h"
#include "CameraManager.h"

bool InputManager::KeyStateImgui = true;
double InputManager::MouseWorldX = 0.0;
double InputManager::MouseWorldY = 0.0;
bool InputManager::MouseLeftClicked = false;
bool InputManager::ShowFPS = false;
double InputManager::ControllerCursorX = 0.0;
double InputManager::ControllerCursorY = 0.0;

/**
 * @brief Constructs an InputManager with specified parameters.
 * @param window Pointer to the GLFW window.
 * @param audio Optional pointer to the AudioEngine.
 */
InputManager::InputManager(GLFWwindow *window, AudioEngine *audio) : window(window), audioEngine(audio), width(0), height(0), mouseX(0.0), mouseY(0.0), scrollX(0.0), scrollY(0.0),cachedPlayerID(0)
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
	for (int i = 0; i < GLFW_KEY_LAST; ++i) {
		m_keyStates[i] = KEY_STATE_RELEASE;
	}
	for (int i = 0; i < GLFW_MOUSE_BUTTON_LAST; ++i) {
		m_mouseButtonStates[i] = KEY_STATE_RELEASE;
	}
    for (int i = 0; i <= GLFW_GAMEPAD_BUTTON_LAST; ++i) {
        m_gamepadButtonStates[i] = KEY_STATE_RELEASE;
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
 * @brief Gets the system name.
 * @return The string "Input System".
 */

std::string InputManager::GetName()
{
    return "InputSystem";
}

bool InputManager::IsKeyPressed(int key)
{
	return m_keyStates[key] == KEY_STATE_PRESS;
}

bool InputManager::IsKeyReleased(int key)
{
	return m_keyStates[key] == KEY_STATE_RELEASE;
}

bool InputManager::IsKeyHeld(int key)
{
	return m_keyStates[key] == KEY_STATE_HOLD;
}

bool InputManager::IsMouseButtonPressed(int button)
{
	return m_mouseButtonStates[button] == KEY_STATE_PRESS;
}

bool InputManager::IsMouseButtonReleased(int button)
{
	return m_mouseButtonStates[button] == KEY_STATE_RELEASE;
}

bool InputManager::IsMouseButtonHeld(int button)
{
	return m_mouseButtonStates[button] == KEY_STATE_HOLD;
}

bool InputManager::IsAnyKeyPress() {
    for (int key = GLFW_KEY_SPACE; key <= GLFW_KEY_LAST; ++key)
    {
        if (glfwGetKey(window, key) == GLFW_PRESS)
            return true;
    }
    return false;
}

bool InputManager::IsGamepadButtonPressed(int button)
{
    auto it = m_gamepadButtonStates.find(button);
    return (it != m_gamepadButtonStates.end()) && (it->second == KEY_STATE_PRESS);
}

bool InputManager::IsGamepadButtonHeld(int button)
{
    auto it = m_gamepadButtonStates.find(button);
    return (it != m_gamepadButtonStates.end()) && (it->second == KEY_STATE_HOLD);
}

bool InputManager::IsGamepadButtonReleased(int button)
{
    auto it = m_gamepadButtonStates.find(button);
    return (it != m_gamepadButtonStates.end()) && (it->second == KEY_STATE_RELEASE);
}

float InputManager::GetGamepadAxis(int axis)
{
    if (!m_gamepadConnected || axis < 0 || axis > GLFW_GAMEPAD_AXIS_LAST) return 0.0f;
    return m_gamepadState.axes[axis];
}

/**
 * @brief Updates input handling, including key presses, mouse events,
 *        player movement, jumping, animations, and quitting logic.
 * @param dt Delta time since the last frame.
 */
void InputManager::Update(float)
{
	static bool firstUpdate = true;
	if (firstUpdate) {
		std::cout << "[InputManager] Update function is running!" << std::endl;
		firstUpdate = false;
	}

	for (auto& it : m_keyStates) {
		if (glfwGetKey(window, it.first) == GLFW_PRESS) {
			if (it.second == KEY_STATE_RELEASE) {
				it.second = KEY_STATE_PRESS;
			}
			else {
				it.second = KEY_STATE_HOLD;
			}
		}
		else {
			it.second = KEY_STATE_RELEASE;
		}
	}
	for (auto& it : m_mouseButtonStates) {
		if (glfwGetMouseButton(window, it.first) == GLFW_PRESS) {
			if (it.second == KEY_STATE_RELEASE) {
				it.second = KEY_STATE_PRESS;
			}
			else {
				it.second = KEY_STATE_HOLD;
			}
		}
		else {
			it.second = KEY_STATE_RELEASE;
		}
	}

    // ---- Gamepad button state polling ----
    {
        GLFWgamepadstate gpState{};
        m_gamepadConnected = (glfwGetGamepadState(GLFW_JOYSTICK_1, &gpState) == GLFW_TRUE);
        if (m_gamepadConnected)
        {
            for (int i = 0; i <= GLFW_GAMEPAD_BUTTON_LAST; ++i)
            {
                if (gpState.buttons[i] == GLFW_PRESS)
                {
                    m_gamepadButtonStates[i] = (m_gamepadButtonStates[i] == KEY_STATE_RELEASE)
                        ? KEY_STATE_PRESS : KEY_STATE_HOLD;
                }
                else
                {
                    m_gamepadButtonStates[i] = KEY_STATE_RELEASE;
                }
            }
            m_gamepadState = gpState;
        }
        else
        {
            for (auto& kv : m_gamepadButtonStates)
                kv.second = KEY_STATE_RELEASE;
        }
    }

    glfwGetWindowSize(window, &width, &height);
    static bool wasLeftMousePressed = false;
    bool isLeftMousePressed = (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS);

    MouseLeftClicked = (!wasLeftMousePressed && isLeftMousePressed);
    wasLeftMousePressed = isLeftMousePressed;

    // === UPDATE MOUSE POSITION FIRST - BEFORE ANY EARLY RETURNS ===
    double xpos, ypos;
    glfwGetCursorPos(window, &xpos, &ypos);
    mouseX = xpos;
    mouseY = ypos;

    // Convert screen to world coordinates
    vec2 camPos = CameraManager::Instance().GetCameraPosition();

    // Get camera zoom to properly scale mouse coordinates
    float zoom = 1.0f;
    CameraComponent* cam = CameraManager::Instance().GetActiveCamera();
    if (cam) {
        zoom = cam->getZoom();
    }

    MouseWorldX = static_cast<double>((xpos - width / 2.0) / zoom) + camPos.x;
    MouseWorldY = static_cast<double>((height / 2.0 - ypos) / zoom) + camPos.y; // Flip Y axis

    // ---- Virtual cursor shared by controller and arrow-key menu navigation ----
    // Mouse movement always wins and resets to real mouse mode (runs regardless of controller).
    {
        static double prevMouseXVirt = 0.0, prevMouseYVirt = 0.0;
        if (std::abs(xpos - prevMouseXVirt) > 2.0 || std::abs(ypos - prevMouseYVirt) > 2.0)
            m_usingController = false;
        prevMouseXVirt = xpos;
        prevMouseYVirt = ypos;
    }

    // Controller stick / D-pad moves virtual cursor. Buttons alone do NOT activate it
    // (avoids teleporting the cursor when the user presses A/Start on the real mouse).
    if (m_gamepadConnected)
    {
        float lx = m_gamepadState.axes[GLFW_GAMEPAD_AXIS_LEFT_X];
        float ly = m_gamepadState.axes[GLFW_GAMEPAD_AXIS_LEFT_Y];
        bool dpadActive = m_gamepadButtonStates[GLFW_GAMEPAD_BUTTON_DPAD_LEFT]  != KEY_STATE_RELEASE
                       || m_gamepadButtonStates[GLFW_GAMEPAD_BUTTON_DPAD_RIGHT] != KEY_STATE_RELEASE
                       || m_gamepadButtonStates[GLFW_GAMEPAD_BUTTON_DPAD_UP]    != KEY_STATE_RELEASE
                       || m_gamepadButtonStates[GLFW_GAMEPAD_BUTTON_DPAD_DOWN]  != KEY_STATE_RELEASE;
        bool stickActive = std::abs(lx) > 0.2f || std::abs(ly) > 0.2f;

        if ((stickActive || dpadActive) && !m_usingController)
        {
            // Inherit current mouse position so the cursor doesn't jump
            ControllerCursorX = MouseWorldX;
            ControllerCursorY = MouseWorldY;
            m_usingController = true;
        }

        if (m_usingController)
        {
            const float cursorSpeed = 15.0f / zoom;
            if (std::abs(lx) > 0.2f) ControllerCursorX += lx * cursorSpeed;
            if (std::abs(ly) > 0.2f) ControllerCursorY -= ly * cursorSpeed;
            if (m_gamepadButtonStates[GLFW_GAMEPAD_BUTTON_DPAD_RIGHT] != KEY_STATE_RELEASE) ControllerCursorX += cursorSpeed;
            if (m_gamepadButtonStates[GLFW_GAMEPAD_BUTTON_DPAD_LEFT]  != KEY_STATE_RELEASE) ControllerCursorX -= cursorSpeed;
            if (m_gamepadButtonStates[GLFW_GAMEPAD_BUTTON_DPAD_UP]    != KEY_STATE_RELEASE) ControllerCursorY += cursorSpeed;
            if (m_gamepadButtonStates[GLFW_GAMEPAD_BUTTON_DPAD_DOWN]  != KEY_STATE_RELEASE) ControllerCursorY -= cursorSpeed;

            MouseWorldX = ControllerCursorX;
            MouseWorldY = ControllerCursorY;
        }

        // A button = confirm/click — works in any cursor mode
        if (m_gamepadButtonStates[GLFW_GAMEPAD_BUTTON_A] == KEY_STATE_PRESS)
            MouseLeftClicked = true;
    }

    static bool keyPressed = false;
    LevelEditor* levelEditor = CORE ? CORE->GetLevelEditor() : nullptr;

    // ------------------- UNDO & REDO: Ctrl+Z / Ctrl+Y -------------------//
    // Evaluated BEFORE the early return so Pause mode can still undo/redo.
    // Play mode is explicitly excluded.
    // Uses IsKeyPressed (KEY_STATE_PRESS) so each physical keypress fires exactly
    // once — no undoRedoConsumed workaround needed.
    if (levelEditor && levelEditor->GetEditorMode() == EditorMode::Stop)
    {
        bool ctrl = IsKeyHeld(GLFW_KEY_LEFT_CONTROL) || IsKeyHeld(GLFW_KEY_RIGHT_CONTROL);
        bool shift = IsKeyHeld(GLFW_KEY_LEFT_SHIFT) || IsKeyHeld(GLFW_KEY_RIGHT_SHIFT);
        bool zPressed = IsKeyPressed(GLFW_KEY_Z);
        bool yPressed = IsKeyPressed(GLFW_KEY_Y);

        if (ctrl)
        {
            // REDO: Ctrl+Y  or  Ctrl+Shift+Z
            if (yPressed || (zPressed && shift))
            {
                if (!levelEditor->redoStack.empty())
                    levelEditor->PerformRedo();
            }
            // UNDO: Ctrl+Z (no shift)
            else if (zPressed && !shift)
            {
                if (!levelEditor->undoStack.empty())
                    levelEditor->PerformUndo();
            }
        }
    }

    if (levelEditor && levelEditor->GetEditorMode() != EditorMode::Play && levelEditor->GetEditorMode() != EditorMode::Stop)
    {
        static bool editorModeWarning = true;
        if (editorModeWarning) {
            std::cout << "[InputManager] Not in Play/Stop mode - input disabled" << std::endl;
            editorModeWarning = false;
        }
        // still allow escape key, but ignore player movement inputs
        double xposition, yposition;
        glfwGetCursorPos(window, &xposition, &yposition);
        mouseX = xposition;
        mouseY = yposition;
        return;
    }

    static bool playModeConfirmed = false;
    if (!playModeConfirmed) {
        std::cout << "[InputManager] In Play mode - input enabled" << std::endl;
        playModeConfirmed = true;
    }


    static std::unordered_map<int, bool> singleTriggerKeys = {
        {GLFW_KEY_SPACE, false},
        {GLFW_KEY_F1, false},
        {GLFW_KEY_F2, false},
        {GLFW_KEY_F3, false},
        {GLFW_KEY_F4, false},
        {GLFW_KEY_F6, false}, // replay best run (ghost alongside player)
        {GLFW_KEY_F10, false}, // enter replay watch mode
        {GLFW_KEY_F7, false}, // rewind 10s
        {GLFW_KEY_F8, false}, // pause/resume replay
        {GLFW_KEY_F9, false}, // forward 10s
        {GLFW_KEY_M, false}, // toggle pause (for now)
        {GLFW_KEY_ENTER, false},
        {GLFW_KEY_ESCAPE,false},
        {GLFW_KEY_P, false}
    };

    static bool altEnterPressed = false;
    bool altPressed = (glfwGetKey(window, GLFW_KEY_LEFT_ALT) == GLFW_PRESS) ||
        (glfwGetKey(window, GLFW_KEY_RIGHT_ALT) == GLFW_PRESS);
    bool enterPressed = (glfwGetKey(window, GLFW_KEY_ENTER) == GLFW_PRESS) ||
        (glfwGetKey(window, GLFW_KEY_KP_ENTER) == GLFW_PRESS);

    if (altPressed && enterPressed) {
        if (!altEnterPressed) {
            altEnterPressed = true;
            // Toggle fullscreen
            if (CORE && CORE->GetGraphicsSystem()) {
                CORE->GetGraphicsSystem()->ToggleFullScreen();
            }
        }
    }
    else {
        altEnterPressed = false;
    }


    // F / G / H — replay watch mode controls (F=rewind, G=pause/resume, H=forward)
    {
        GhostSystem* ghost = CORE ? CORE->GetGhostSystem() : nullptr;
        bool inReplay = ghost && ghost->IsReplayMode();

        if (glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS)
        {
            if (!singleTriggerKeys[GLFW_KEY_F] && inReplay)
            {
                ghost->SeekReplay(-10.0f);
                std::cout << "[Replay] Rewind 10s" << std::endl;
            }
            singleTriggerKeys[GLFW_KEY_F] = true;
        }
        else { singleTriggerKeys[GLFW_KEY_F] = false; }

        if (glfwGetKey(window, GLFW_KEY_G) == GLFW_PRESS)
        {
            if (!singleTriggerKeys[GLFW_KEY_G] && inReplay)
            {
                ghost->ToggleReplayPause();
                std::cout << "[Replay] " << (ghost->IsReplayPaused() ? "Paused" : "Resumed") << std::endl;
            }
            singleTriggerKeys[GLFW_KEY_G] = true;
        }
        else { singleTriggerKeys[GLFW_KEY_G] = false; }

        if (glfwGetKey(window, GLFW_KEY_H) == GLFW_PRESS)
        {
            if (!singleTriggerKeys[GLFW_KEY_H] && inReplay)
            {
                ghost->SeekReplay(10.0f);
                std::cout << "[Replay] Forward 10s" << std::endl;
            }
            singleTriggerKeys[GLFW_KEY_H] = true;
        }
        else { singleTriggerKeys[GLFW_KEY_H] = false; }
    }

    // handle M key for pausing and resuming background music
    if (glfwGetKey(window, GLFW_KEY_M) == GLFW_PRESS)
    {
        if (!singleTriggerKeys[GLFW_KEY_M])
        {
            AudioManager* audioManager = CORE ? CORE->GetAudioManager() : nullptr;
            if (audioManager) audioManager->ToggleMute();
            singleTriggerKeys[GLFW_KEY_M] = true;
        }
    }

    // Check if in play mode for pause menu
    LevelEditor* editor = CORE ? CORE->GetLevelEditor() : nullptr;
    bool isPlayMode = (editor && editor->GetEditorMode() == EditorMode::Play);

    // Check if we're in the main menu
    IGameCallbacksSystem* gameCb = CORE ? CORE->GetGameCallbacks() : nullptr;
    bool isMainMenu = gameCb && gameCb->IsOnMainMenu();

    // Handle ESC key
    if (isPlayMode)
    {
        // Check if win or lose screen is active - if so, disable ESC key
        bool winScreenActive  = gameCb && gameCb->IsWinScreenActive();
        bool loseScreenActive = gameCb && gameCb->IsLoseScreenActive();

        // In play mode: ESC exits replay mode if active, otherwise toggles pause menu
        auto* textInput = CORE ? CORE->GetTextInputSystem() : nullptr;
        bool textInputActive = textInput && (textInput->IsActive() || textInput->ConsumedEscapeThisFrame());

        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        {
            if (!singleTriggerKeys[GLFW_KEY_ESCAPE] && !winScreenActive && !loseScreenActive && !isMainMenu && !textInputActive)
            {
                GhostSystem* ghostSys = CORE ? CORE->GetGhostSystem() : nullptr;
                bool isPauseActive = gameCb && gameCb->IsPauseMenuActive();

                // Priority order (highest first) — only one action fires per press
                if (ghostSys && ghostSys->IsReplayMode())
                {
                    std::cout << "[InputManager] ESC - Exiting Replay Mode" << std::endl;
                    ghostSys->ExitReplayMode();
                    singleTriggerKeys[GLFW_KEY_ESCAPE] = true;
                }
                else if (isPauseActive && gameCb->IsHowToPlayOpen())
                {
                    // Delegate to PauseMenuLogic's own ESC handler in its HowToPlay loop
                    singleTriggerKeys[GLFW_KEY_ESCAPE] = true;
                }
                else if (isPauseActive && gameCb->IsGhostMenuOpen())
                {
                    std::cout << "[InputManager] ESC - Closing Ghost Menu" << std::endl;
                    gameCb->CloseGhostMenu();
                    singleTriggerKeys[GLFW_KEY_ESCAPE] = true;
                }
                else if (isPauseActive && gameCb->IsPauseSettingsOpen())
                {
                    std::cout << "[InputManager] ESC - Closing Pause Settings" << std::endl;
                    gameCb->ClosePauseSettings();
                    singleTriggerKeys[GLFW_KEY_ESCAPE] = true;
                }
                else
                {
                    // Otherwise toggle pause menu normally
                    if (gameCb) gameCb->TogglePauseMenu();
                    singleTriggerKeys[GLFW_KEY_ESCAPE] = true;
                }
            }
        }
        else
        {
            singleTriggerKeys[GLFW_KEY_ESCAPE] = false;
        }

        // Gamepad Start button: toggle pause (same as ESC in play mode)
        if (m_gamepadConnected && m_gamepadButtonStates[GLFW_GAMEPAD_BUTTON_START] == KEY_STATE_PRESS
            && !winScreenActive && !loseScreenActive && !isMainMenu)
        {
            GhostSystem* ghostSys = CORE ? CORE->GetGhostSystem() : nullptr;
            if (ghostSys && ghostSys->IsReplayMode())
                ghostSys->ExitReplayMode();
            else if (gameCb)
                gameCb->TogglePauseMenu();
        }

    }
    else
    {
        // Not in play mode: ESC quits the game
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        {
            KeyStateQuit = true;
            glfwSetWindowShouldClose(window, GLFW_TRUE);
        }
    }

    // Handle P key
    if (isPlayMode)
    {
        // In play mode: P toggles pause menu (unless in main menu)
        /*if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS)
        {
            if (!singleTriggerKeys[GLFW_KEY_P] && !isMainMenu)
            {
                if (gameCb) gameCb->TogglePauseMenu();
                singleTriggerKeys[GLFW_KEY_P] = true;
            }
        }
        else
        {
            singleTriggerKeys[GLFW_KEY_P] = false;
        }*/

        // Handle TAB key to toggle How To Play screen
        static bool tabWasPressed = false;
        bool tabPressed = (glfwGetKey(window, GLFW_KEY_TAB) == GLFW_PRESS);

        if (tabPressed && !tabWasPressed && !isMainMenu)
        {
            if (gameCb)
            {
                if (gameCb->IsHowToPlayOpen())
                {
                    std::cout << "[InputManager] TAB - Closing How To Play" << std::endl;
                    gameCb->CloseHowToPlay();
                }
                else
                {
                    std::cout << "[InputManager] TAB - Opening How To Play" << std::endl;
                    gameCb->OpenHowToPlay();
                }
            }
            tabWasPressed = true;
        }
        else if (!tabPressed)
        {
            tabWasPressed = false;
        }

        {
            static bool shiftRWasPressed = false;
            bool shiftRPressed = (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS && glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS);
            if (shiftRPressed && !shiftRWasPressed)
            {
                std::string currentScenePath = gameCb ? gameCb->GetCurrentScenePath() : "";
                if (!currentScenePath.empty())
                    CORE->RequestSceneLoad(currentScenePath, true);
            }
            shiftRWasPressed = shiftRPressed;
        }
    }
    else
    {
        // Not in play mode: P toggles ImGui
        if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS)
        {
            InputManager::KeyStateImgui = true;
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
#ifndef _DEBUG
#else
                if (key == GLFW_KEY_F1)
                {
                    CORE->GetGraphicsSystem()->toggleDebugMode();
                    std::cout << "Dubug mode toggled" << CORE->GetGraphicsSystem()->isDebugModeOn() << std::endl;
                }
                else if (key == GLFW_KEY_F2)
                {
                    CORE->stepMode = !CORE->stepMode;

                    if (!CORE->stepMode)
                    {
                        CORE->accumulator = 0.0;
                    }

                    std::cout << "\n[STEP MODE] " << (CORE->stepMode ? "ENABLED - Press F3 to advance frames" : "DISABLED - Normal play") << std::endl;
                }
                else if (key == GLFW_KEY_F3 && CORE->stepMode)
                {
                    CORE->stepOnce = true;
                    std::cout << "[STEP MODE] Advancing 1 frame..." << std::endl;
                }
#endif
                if (key == GLFW_KEY_F4)
                {
                    ShowFPS = !ShowFPS;
                    std::cout << "FPS display " << (ShowFPS ? "ENABLED" : "DISABLED") << std::endl;
                }
                else if (key == GLFW_KEY_P) {

                }
            }
            wasPressed = true;
        }
        else
        {
            wasPressed = false;
        }
    }


    // In replay watch mode, block all player input below this point
    {
        GhostSystem* ghostSys = CORE ? CORE->GetGhostSystem() : nullptr;
        if (ghostSys && ghostSys->IsReplayMode())
            return;
    }

    GameObject* player = nullptr;

    // Try to get cached player first
    if (cachedPlayerID != 0)
    {
        player = GameObjectFactory::Instance().GetGameObject(cachedPlayerID);
        if (!player)
        {
            // Cached player no longer exists (scene changed), clear cache
            std::cout << "[InputManager] Cached player no longer exists, clearing cache" << std::endl;
            cachedPlayerID = 0;
        }
    }

    // If no cached player, try to find Hero
    if (!player)
    {
        player = GameObjectFactory::Instance().FindGameObject("Hero");
        if (player)
        {
            cachedPlayerID = player->GetID();
            std::cout << "[InputManager] Found Hero with ID: " << cachedPlayerID << std::endl;
        }
        else
        {
            // Only print warning occasionally to avoid spam
            static int warningCounter = 0;
            if (warningCounter++ % 60 == 0)
            {
                std::cout << "[InputManager] WARNING: Hero not found in scene!" << std::endl;
            }
        }
    }

    if (!player)
    {
        return; // No player to control
    }


    auto* physics = player->GetComponent<PhysicsComponent>();
    if (!physics) return;

    TransformComponent* transform = player->GetComponent<TransformComponent>();
    if (!transform) return;

    // ===== PARTICLE TESTING SETUP
    
    //auto* dashTrail = player->GetComponent<ParticleEmitterComponent>();

    //if (!dashTrail) {
    //    // If it doesn't exist, add it and configure it once
    //    dashTrail = player->AddComponent<ParticleEmitterComponent>();
    //    if (dashTrail) {
    //        dashTrail->ConfigureDashTrail();
    //        dashTrail->SetTexture(0);
    //        std::cout << "Particle Emitter initialized successfully\n";
    //    }
    //    else {
    //        std::cerr << "particle not initialized properly\n";
    //    }
    //}

    /**
     * @brief Destroy 2500 Game Objects on screen, if ENTER key is pressed
     */
    //if (glfwGetKey(window, GLFW_KEY_ENTER) == GLFW_PRESS)
    //{
    //    DemoM1_DestroyGameObjects();
    //}
    //else if (glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS && isPlayMode) 
    //{
    //    CameraManager::Instance().AddCameraTrauma(1.f);
    //    CameraManager::Instance().FlashActiveCamera(0.3f, vec4(1.f, 0.f, 0.f, 0.7f));
    //    //CORE->SetGamePaused(!CORE->IsGamePaused());
    //}
}

    void InputManager::ResetState()
    {

        MouseLeftClicked = false;

        mouseX = 0.0;
        mouseY = 0.0;
        scrollX = 0.0;
        scrollY = 0.0;

        std::cout << "[InputManager] State reset (including all abilities)." << std::endl;
    }
