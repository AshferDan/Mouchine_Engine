/* Start Header ************************************************************************/
/*!
\file       ControllerNotificationSystem.cpp
\date       2026
Copyright (C) 2025 DigiPen Institute of Technology.
*/
/* End Header **************************************************************************/

#include "ControllerNotificationSystem.h"
#include "core.h"
#include "factory.h"
#include "transform.h"
#include "RenderComponent.h"
#include "TextComponent.h"
#include "GraphicsSystem.h"
#include "CameraManager.h"
#include "InputManager.h"
#include "MathLib.h"
#include <algorithm>
#include <GLFW/glfw3.h>

void ControllerNotificationSystem::Initialize()
{
    // Called after every scene load — objects are already destroyed by then,
    // so just clear our stale pointers.
    ClearPointers();
}

void ControllerNotificationSystem::Update(float dt)
{
    InputManager* inp = CORE->GetInputManager();
    if (!inp) return;

    // Detect connection state transitions
    if (inp->IsGamepadJustConnected())
        ShowPopup("Controller Connected");
    else if (inp->IsGamepadJustDisconnected())
        ShowPopup("Controller Disconnected");

    if (!m_active) return;

    m_timer += dt;

    // Safety check — if the GOs were destroyed externally (scene reload), clean up
    auto& factory = GameObjectFactory::Instance();
    if ((m_background && !factory.IsAlive(m_background)) ||
        (m_text       && !factory.IsAlive(m_text)))
    {
        ClearPointers();
        return;
    }

    // Allow dismissal only after the grace period
    if (m_timer >= kDismissGrace)
    {
        bool anyKey = inp->IsAnyKeyPress();
        if (!anyKey)
        {
            for (int i = 0; i <= GLFW_GAMEPAD_BUTTON_LAST; ++i)
            {
                if (inp->IsGamepadButtonPressed(i)) { anyKey = true; break; }
            }
        }
        if (anyKey || m_timer >= kDuration)
        {
            DestroyPopup();
            return;
        }
    }

    // Fade-out alpha in the final kFadeDuration seconds
    float alpha = 1.0f;
    if (m_timer > kDuration - kFadeDuration)
        alpha = std::max(0.0f, (kDuration - m_timer) / kFadeDuration);

    if (m_background)
    {
        auto* rc = m_background->GetComponent<RenderComponent>();
        if (rc) rc->SetColor(vec4(1.0f, 1.0f, 1.0f, alpha));
    }
    if (m_text)
    {
        auto* tc = m_text->GetComponent<TextComponent>();
        if (tc) tc->SetColor(vec4(0.08f, 0.04f, 0.02f, alpha));
    }
}

void ControllerNotificationSystem::ShowPopup(const std::string& message)
{
    DestroyPopup(); // Clear any existing popup first

    auto& factory = GameObjectFactory::Instance();
    vec2 camPos = CameraManager::Instance().GetCameraPosition();

    // Background panel
    m_background = factory.CreateGameObject("CtrlNotif_BG");
    auto* bt = m_background->AddComponent<TransformComponent>();
    bt->posX   = camPos.x;
    bt->posY   = camPos.y + 280.0f; // upper region of the screen
    bt->scaleX = 975.0f;
    bt->scaleY = 240.0f;
    GLuint bgTex = GraphicsSystem::LoadTexture("../Assets/Sprites/Extras_menu.png");
    auto* br = m_background->AddComponent<RenderComponent>(bgTex);
    br->SetColor(vec4(1.0f, 1.0f, 1.0f, 1.0f));
    br->SetLayer(200);

    // Text label
    m_text = factory.CreateGameObject("CtrlNotif_Text");
    auto* tt = m_text->AddComponent<TransformComponent>();
    tt->posX   = camPos.x;
    tt->posY   = camPos.y + 280.0f;
    tt->scaleX = 600.0f;
    tt->scaleY = 80.0f;
    m_text->AddComponent<TextComponent>(
        "../Assets/FrederickatheGreat-Regular.ttf", 38,
        message, vec4(0.08f, 0.04f, 0.02f, 1.0f), false);
    m_text->GetComponent<TextComponent>()->SetLayer(201);

    m_active = true;
    m_timer  = 0.0f;
}

void ControllerNotificationSystem::DestroyPopup()
{
    if (!m_active) return;
    auto& factory = GameObjectFactory::Instance();
    if (m_background && factory.IsAlive(m_background))
        factory.QueueDestroyGameObject(m_background->GetID());
    if (m_text && factory.IsAlive(m_text))
        factory.QueueDestroyGameObject(m_text->GetID());
    ClearPointers();
}

void ControllerNotificationSystem::ClearPointers()
{
    m_background = nullptr;
    m_text       = nullptr;
    m_active     = false;
    m_timer      = 0.0f;
}
