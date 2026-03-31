/* Start Header ************************************************************************/
/*!
\file       MediaPlayerLogic.cpp
\author     Jonathan Fam Qian Wei
\par        Fam.q@digipen.edu
\date       2026

Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*/
/* End Header **************************************************************************/

#include "MediaPlayerLogic.h"
#include "GhostSystem.h"
#include "InputManager.h"
#include "GraphicsSystem.h"
#include "transform.h"
#include "RenderComponent.h"
#include <GLFW/glfw3.h>
#include <iostream>
#include <algorithm>

// ---------------------------------------------------------------------------
// Behaviour constants — layout is driven entirely by the scene JSON.
// ---------------------------------------------------------------------------
static constexpr float MP_SEEK_SECS  = 5.0f;   // Seconds to skip forward / rewind

// ---------------------------------------------------------------------------
// GameObject name constants — must match the scene hierarchy exactly.
// ---------------------------------------------------------------------------
static constexpr const char* OBJ_ROOT   = "Media_Player";
static constexpr const char* OBJ_BOX    = "Replay_box";
static constexpr const char* OBJ_PLAY   = "Replay_playing";
static constexpr const char* OBJ_BAR    = "Replay_bar_brown";
static constexpr const char* OBJ_BWD    = "Replay_backward";
static constexpr const char* OBJ_FWD    = "Replay_forward";
static constexpr const char* OBJ_VIDEO  = "ReplayVideo";    // Restart button
static constexpr const char* OBJ_SLIDER = "Replay_slider";

// ---------------------------------------------------------------------------
// Icon paths — add Replay_paused.png to Assets/Sprites/Icons/ for the swap.
// ---------------------------------------------------------------------------
static constexpr const char* ICON_PLAY  = "../Assets/Sprites/Icons/Replay_playing.png";
static constexpr const char* ICON_PAUSE = "../Assets/Sprites/Icons/paused_button.png";

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------

void MediaPlayerLogicSystem::SetObjectActive(const char* name, bool active)
{
    auto& factory = GameObjectFactory::Instance();
    GameObject* obj = factory.FindGameObject(name);
    if (!obj) return;

    obj->SetActive(active);

    // Components are serialised as disabled when the object is inactive,
    // so we explicitly enable/disable the ones we care about.
    if (auto* tf = obj->GetComponent<TransformComponent>())
        tf->SetEnabled(active);
    if (auto* rc = obj->GetComponent<RenderComponent>())
    {
        rc->SetEnabled(active);
        // Force screen space on every child — some may have been saved with
        // screenSpace=false if they were not individually updated in the editor.
        if (active)
            rc->SetScreenSpace(true);
    }
}

// ---------------------------------------------------------------------------
// ISystem interface
// ---------------------------------------------------------------------------

void MediaPlayerLogicSystem::Initialize()
{
    m_spawned        = false;
    m_mouseWasDown   = false;
    m_sliderDragging = false;
    m_wasPaused      = false;
}

void MediaPlayerLogicSystem::Update(float dt)
{
    (void)dt;

    GhostSystem* ghost = CORE ? CORE->GetGhostSystem() : nullptr;
    if (!ghost)
        return;

    bool inReplayMode = ghost->IsReplayMode();

    if (inReplayMode && !m_spawned)
        SpawnMediaPlayer();
    else if (!inReplayMode && m_spawned)
        DestroyMediaPlayer();

    if (m_spawned && inReplayMode)
        UpdateMediaPlayer();
}

// ---------------------------------------------------------------------------
// Private helpers
// ---------------------------------------------------------------------------

void MediaPlayerLogicSystem::SpawnMediaPlayer()
{
    SetObjectActive(OBJ_ROOT,   true);
    SetObjectActive(OBJ_BOX,    true);
    SetObjectActive(OBJ_PLAY,   true);
    SetObjectActive(OBJ_BAR,    true);
    SetObjectActive(OBJ_BWD,    true);
    SetObjectActive(OBJ_FWD,    true);
    SetObjectActive(OBJ_VIDEO,  true);
    SetObjectActive(OBJ_SLIDER, true);

    m_spawned      = true;
    m_wasPaused    = false;
    std::cout << "[MediaPlayerLogic] Media_Player activated.\n";
}

void MediaPlayerLogicSystem::DestroyMediaPlayer()
{
    SetObjectActive(OBJ_ROOT,   false);
    SetObjectActive(OBJ_BOX,    false);
    SetObjectActive(OBJ_PLAY,   false);
    SetObjectActive(OBJ_BAR,    false);
    SetObjectActive(OBJ_BWD,    false);
    SetObjectActive(OBJ_FWD,    false);
    SetObjectActive(OBJ_VIDEO,  false);
    SetObjectActive(OBJ_SLIDER, false);

    m_spawned        = false;
    m_sliderDragging = false;
    std::cout << "[MediaPlayerLogic] Media_Player deactivated.\n";
}

void MediaPlayerLogicSystem::UpdatePlayPauseIcon(bool paused)
{
    auto& factory = GameObjectFactory::Instance();
    GameObject* btn = factory.FindGameObject(OBJ_PLAY);
    if (!btn) return;

    auto* rc = btn->GetComponent<RenderComponent>();
    if (!rc)  return;

    GraphicsSystem* gfx = CORE ? CORE->GetGraphicsSystem() : nullptr;
    if (!gfx) return;

    // paused == true  → replay is paused, so show the PLAY icon (click to resume)
    // paused == false → replay is running, so show the PAUSE icon (click to pause)
    const char* iconPath = paused ? ICON_PLAY : ICON_PAUSE;

    GLuint newTex = gfx->LoadTexture(iconPath);
    if (newTex)
    {
        GLuint old = rc->GetTextureID();
        if (old) glDeleteTextures(1, &old);
        rc->SetTextureID(newTex);
        rc->SetTexturePath(iconPath);
    }
}

void MediaPlayerLogicSystem::UpdateMediaPlayer()
{
    GhostSystem*    ghost = CORE ? CORE->GetGhostSystem()    : nullptr;
    GraphicsSystem* gfx   = CORE ? CORE->GetGraphicsSystem() : nullptr;
    InputManager*   input = CORE ? CORE->GetInputManager()   : nullptr;
    if (!ghost || !gfx || !input) return;

    auto& factory = GameObjectFactory::Instance();

    // --- Slider thumb: position driven by playback progress along the bar ---
    // Read the bar's actual transform so we respect whatever is set in the scene.
    const float duration = ghost->GetReplayDuration();
    const float clock    = ghost->GetReplayClock();
    const float progress = (duration > 0.f)
        ? std::max(0.f, std::min(1.f, clock / duration))
        : 0.f;

    float barLeft = 0.f, barWidth = 0.f, barCY = 0.f;
    if (GameObject* barObj = factory.FindGameObject(OBJ_BAR))
    {
        if (auto* tf = barObj->GetComponent<TransformComponent>())
        {
            barLeft  = tf->posX - tf->scaleX * 0.5f;
            barWidth = tf->scaleX;
            barCY    = tf->posY;
        }
    }

    if (GameObject* sliderObj = factory.FindGameObject(OBJ_SLIDER))
    {
        if (auto* tf = sliderObj->GetComponent<TransformComponent>())
            tf->posX = barLeft + progress * barWidth;
    }

    // --- Stop at end: pause when the replay reaches the last frame ---
    if (!ghost->IsReplayPaused() && duration > 0.f && clock >= duration)
        ghost->ToggleReplayPause();

    // --- Play / Pause icon: swap texture whenever the pause state changes ---
    const bool isPaused = ghost->IsReplayPaused();
    if (isPaused != m_wasPaused)
    {
        UpdatePlayPauseIcon(isPaused);
        m_wasPaused = isPaused;
    }

    // --- Mouse input (convert GLFW top-left Y to screen-space bottom-left Y) ---
    const float windowH  = (float)gfx->GetWindowHeight();
    const float mouseX   = (float)input->GetMouseX();
    const float mouseY   = windowH - (float)input->GetMouseY();

    const bool mouseDown         = (glfwGetMouseButton(glfwGetCurrentContext(), GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS);
    const bool mouseJustPressed  = mouseDown  && !m_mouseWasDown;
    const bool mouseJustReleased = !mouseDown &&  m_mouseWasDown;
    m_mouseWasDown               = mouseDown;

    // AABB hit-test using each object's actual transform (respects scene layout)
    auto HitTest = [&](const char* name) -> bool
    {
        GameObject* obj = factory.FindGameObject(name);
        if (!obj) return false;
        auto* tf = obj->GetComponent<TransformComponent>();
        if (!tf)  return false;
        return mouseX >= tf->posX - tf->scaleX * 0.5f
            && mouseX <= tf->posX + tf->scaleX * 0.5f
            && mouseY >= tf->posY - tf->scaleY * 0.5f
            && mouseY <= tf->posY + tf->scaleY * 0.5f;
    };

    // --- Begin slider drag when clicking inside the timeline bar ---
    if (mouseJustPressed && !m_sliderDragging)
    {
        if (HitTest(OBJ_BAR))
            m_sliderDragging = true;
    }

    // While dragging, seek to the mouse position along the bar
    if (m_sliderDragging && mouseDown && barWidth > 0.f && duration > 0.f)
    {
        const float t = std::max(0.f, std::min(1.f, (mouseX - barLeft) / barWidth));
        ghost->SeekReplayAbsolute(t * duration);
    }

    if (mouseJustReleased)
        m_sliderDragging = false;

    // --- Button clicks (only on a fresh press, not while dragging) ---
    if (mouseJustPressed && !m_sliderDragging)
    {
        // Play / Pause toggle
        if (HitTest(OBJ_PLAY))
            ghost->ToggleReplayPause();

        // Rewind 5 seconds
        if (HitTest(OBJ_BWD))
            ghost->SeekReplay(-MP_SEEK_SECS);

        // Skip forward 5 seconds
        if (HitTest(OBJ_FWD))
            ghost->SeekReplay(MP_SEEK_SECS);

        // Restart: seek to beginning and resume if paused
        if (HitTest(OBJ_VIDEO))
        {
            ghost->SeekReplayAbsolute(0.f);
            if (ghost->IsReplayPaused())
                ghost->ToggleReplayPause();
        }
    }
}
