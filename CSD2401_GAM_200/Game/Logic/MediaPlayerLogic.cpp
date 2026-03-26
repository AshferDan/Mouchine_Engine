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
#include "Serialization.h"
#include "InputManager.h"
#include "CameraManager.h"
#include "CameraComponent.h"
#include <GLFW/glfw3.h>
#include <iostream>

// ---------------------------------------------------------------------------
// Layout constants — world-space units, relative to camera centre.
// Tuned for zoom = 0.5 / 1920 x 1080.
// ---------------------------------------------------------------------------
static constexpr float MP_Y_OFFSET  = -920.0f; ///< Distance below camera centre (near bottom)
static constexpr float MP_BAR_CX    =   50.0f; ///< Timeline bar centre X offset from camera
static constexpr float MP_BAR_W     = 1000.0f; ///< Timeline bar total width
static constexpr float MP_BAR_H     =   50.0f; ///< Timeline bar height
static constexpr float MP_PLAY_X    = -550.0f; ///< Play/Pause button centre X offset
static constexpr float MP_PLAY_SIZE =   80.0f; ///< Play/Pause button size (square)
static constexpr float MP_BWD_X     = -650.0f; ///< Backward (-10 s) button centre X offset
static constexpr float MP_BWD_SIZE  =   80.0f; ///< Backward button size
static constexpr float MP_FWD_X     =  650.0f; ///< Forward (+10 s) button centre X offset
static constexpr float MP_FWD_SIZE  =   80.0f; ///< Forward button size
static constexpr float MP_SLIDER_W  =   60.0f; ///< Slider thumb width
static constexpr float MP_SLIDER_H  =   80.0f; ///< Slider thumb height
static constexpr float MP_VIDEO_X   =  760.0f; ///< Exit-replay (ReplayVideo) button centre X
static constexpr float MP_VIDEO_W   =   80.0f; ///< Exit-replay button width
static constexpr float MP_VIDEO_H   =   80.0f; ///< Exit-replay button height

// ---------------------------------------------------------------------------
// ISystem interface
// ---------------------------------------------------------------------------

void MediaPlayerLogicSystem::Initialize()
{
    m_spawned        = false;
    m_mouseWasDown   = false;
    m_sliderDragging = false;
}

void MediaPlayerLogicSystem::Update(float dt)
{
    (void)dt;

    GhostSystem* ghost = CORE ? CORE->GetGhostSystem() : nullptr;
    if (!ghost)
        return;

    bool inReplayMode = ghost->IsReplayMode();

    // Spawn or destroy the UI to match replay mode state
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
    Serialization::LoadPrefabWithReturn("Media_Player");
    m_spawned = true;
    std::cout << "[MediaPlayerLogic] Media_Player spawned.\n";
}

void MediaPlayerLogicSystem::DestroyMediaPlayer()
{
    auto& factory = GameObjectFactory::Instance();

    static const char* names[] = {
        "Media_Player",
        "Replay_box",
        "Replay_playing",
        "Replay_bar_brown",
        "Replay_backward",
        "Replay_forward",
        "ReplayVideo",
        "Replay_slider"
    };

    for (const char* name : names)
    {
        GameObject* obj = factory.FindGameObject(name);
        if (obj)
            factory.QueueDestroyGameObject(obj->GetID());
    }

    m_spawned        = false;
    m_sliderDragging = false;
    std::cout << "[MediaPlayerLogic] Media_Player destroyed.\n";
}

void MediaPlayerLogicSystem::UpdateMediaPlayer()
{
    GhostSystem* ghost = CORE ? CORE->GetGhostSystem() : nullptr;
    if (!ghost)
        return;

    auto& factory = GameObjectFactory::Instance();

    // --- Camera anchor: all positions are relative to the camera centre ---
    vec2  camPos = CameraManager::Instance().GetCameraPosition();
    float camX   = camPos.x;
    float camY   = camPos.y;
    float baseY  = camY + MP_Y_OFFSET;

    // Helper: reposition an object and optionally resize it
    auto MoveObj = [&](const char* name, float wx, float wy, float sx = -1.f, float sy = -1.f)
    {
        GameObject* obj = factory.FindGameObject(name);
        if (!obj) return;
        auto* tf = obj->GetComponent<TransformComponent>();
        if (!tf)  return;
        tf->posX = wx;
        tf->posY = wy;
        if (sx > 0.f)
        {
            tf->scaleX = sx;
            tf->scaleY = sy;
        }
    };

    // Reposition all static media player elements
    MoveObj("Replay_bar_brown", camX + MP_BAR_CX, baseY, MP_BAR_W + 10.f, MP_BAR_H);
    MoveObj("Replay_box",       camX + MP_BAR_CX, baseY, MP_BAR_W,        MP_BAR_H);
    MoveObj("Replay_backward",  camX + MP_BWD_X,  baseY, MP_BWD_SIZE,     MP_BWD_SIZE);
    MoveObj("Replay_forward",   camX + MP_FWD_X,  baseY, MP_FWD_SIZE,     MP_FWD_SIZE);
    MoveObj("Replay_playing",   camX + MP_PLAY_X, baseY, MP_PLAY_SIZE,    MP_PLAY_SIZE);
    MoveObj("ReplayVideo",      camX + MP_VIDEO_X, baseY, MP_VIDEO_W,     MP_VIDEO_H);

    // --- Slider thumb: position reflects playback progress ---
    float duration = ghost->GetReplayDuration();
    float clock    = ghost->GetReplayClock();
    float progress = (duration > 0.f) ? (clock / duration) : 0.f;
    if (progress < 0.f) progress = 0.f;
    if (progress > 1.f) progress = 1.f;

    float barLeft = camX + MP_BAR_CX - MP_BAR_W * 0.5f;
    float sliderX = barLeft + progress * MP_BAR_W;
    MoveObj("Replay_slider", sliderX, baseY, MP_SLIDER_W, MP_SLIDER_H);

    // --- Mouse state ---
    float mouseX = static_cast<float>(InputManager::GetMouseWorldX());
    float mouseY = static_cast<float>(InputManager::GetMouseWorldY());

    bool mouseDown         = (glfwGetMouseButton(glfwGetCurrentContext(), GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS);
    bool mouseJustPressed  = mouseDown  && !m_mouseWasDown;
    bool mouseJustReleased = !mouseDown &&  m_mouseWasDown;
    m_mouseWasDown         = mouseDown;

    // AABB point test (centre + half-extents)
    auto InRect = [](float mx, float my, float cx, float cy, float hw, float hh) -> bool
    {
        return (mx >= cx - hw) && (mx <= cx + hw)
            && (my >= cy - hh) && (my <= cy + hh);
    };

    // --- Slider drag: begin when clicking inside the timeline bar ---
    if (mouseJustPressed && !m_sliderDragging)
    {
        if (InRect(mouseX, mouseY,
                   camX + MP_BAR_CX, baseY,
                   MP_BAR_W * 0.5f, MP_BAR_H * 0.5f))
        {
            m_sliderDragging = true;
        }
    }

    // While dragging, seek the replay to the mouse position along the bar
    if (m_sliderDragging && mouseDown)
    {
        float t = (mouseX - barLeft) / MP_BAR_W;
        if (t < 0.f) t = 0.f;
        if (t > 1.f) t = 1.f;
        ghost->SeekReplayAbsolute(t * duration);
    }

    // Release drag
    if (mouseJustReleased)
        m_sliderDragging = false;

    // --- Button clicks (only on fresh press, not while dragging) ---
    if (mouseJustPressed && !m_sliderDragging)
    {
        // Play / Pause toggle
        if (InRect(mouseX, mouseY,
                   camX + MP_PLAY_X, baseY,
                   MP_PLAY_SIZE * 0.5f, MP_PLAY_SIZE * 0.5f))
        {
            ghost->ToggleReplayPause();
        }

        // Rewind 10 s
        if (InRect(mouseX, mouseY,
                   camX + MP_BWD_X, baseY,
                   MP_BWD_SIZE * 0.5f, MP_BWD_SIZE * 0.5f))
        {
            ghost->SeekReplay(-10.f);
        }

        // Skip forward 10 s
        if (InRect(mouseX, mouseY,
                   camX + MP_FWD_X, baseY,
                   MP_FWD_SIZE * 0.5f, MP_FWD_SIZE * 0.5f))
        {
            ghost->SeekReplay(10.f);
        }

        // Exit replay mode (ReplayVideo button)
        if (InRect(mouseX, mouseY,
                   camX + MP_VIDEO_X, baseY,
                   MP_VIDEO_W * 0.5f, MP_VIDEO_H * 0.5f))
        {
            ghost->ExitReplayMode();
        }
    }
}
