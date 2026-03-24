/* Start Header ************************************************************************/
/*!
\file       GhostSystem.cpp
\author     Jonathan Fam Qian Wei
\par        Fam.q@digipen.edu
\date       2025
Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*/
/* End Header **************************************************************************/

#include "GhostSystem.h"
#include <iostream>
#include <fstream>
#include <filesystem>
#include <algorithm>
#include "Factory.h"
#include "transform.h"
#include "CameraManager.h"
#include "CameraComponent.h"
#include "Collidercomponent.h"
#include "Physics.h"
#include "Serialization.h"
#include "InputManager.h"
#include <GLFW/glfw3.h>
#include <nlohmann/json.hpp>
using json = nlohmann::ordered_json;

// ===== Media Player layout constants (tuned for zoom = 0.5 / 1920x1080) =====
// All offsets are relative to the camera position.
static constexpr float MP_Y_OFFSET   = -920.0f;  ///< Distance below camera centre (near bottom)
static constexpr float MP_BAR_CX     =   50.0f;  ///< Timeline bar centre X offset
static constexpr float MP_BAR_W      = 1000.0f;  ///< Timeline bar total width
static constexpr float MP_BAR_H      =   50.0f;  ///< Timeline bar height
static constexpr float MP_PLAY_X     = -550.0f;  ///< Play / Pause button centre X offset
static constexpr float MP_PLAY_SIZE  =   80.0f;  ///< Play / Pause button half-size (square)
static constexpr float MP_BWD_X      = -650.0f;  ///< Backward button centre X offset
static constexpr float MP_BWD_SIZE   =   80.0f;  ///< Backward button half-size
static constexpr float MP_FWD_X      =  650.0f;  ///< Forward button centre X offset
static constexpr float MP_FWD_SIZE   =   80.0f;  ///< Forward button half-size
static constexpr float MP_SLIDER_W   =   60.0f;  ///< Slider thumb width
static constexpr float MP_SLIDER_H   =   80.0f;  ///< Slider thumb height

// ===== Helpers =====

std::string GhostSystem::MakePath(const std::string& fileName)
{
    // Determine level subfolder (Level_1, Level_2, or Level_3) from the stored level name
    std::string subFolder;
    for (int i = 1; i <= 3; ++i)
    {
        std::string key = "Level_" + std::to_string(i);
        if (m_levelName.find(key) != std::string::npos)
        {
            subFolder = key;
            break;
        }
    }

    std::string dir = subFolder.empty()
        ? "../Assets/Replay"
        : "../Assets/Replay/" + subFolder;

    std::filesystem::create_directories(dir);
    return dir + "/" + fileName + ".json";
}

// ===== ISystem =====

void GhostSystem::Initialize()
{
    std::cout << "Initializing system: GhostSystem" << std::endl;
}

void GhostSystem::Update(float dt)
{
    // Deferred replay-on-load: wait until liveTarget exists (scene is ready), then start replay
    if (m_pendingReplayLoad && !m_liveTarget.empty())
    {
        if (GameObjectFactory::Instance().FindGameObject(m_liveTarget))
        {
            m_pendingReplayLoad = false;
            LoadRecording(m_pendingRecordingName);
            EnterReplayMode();
        }
    }

    // Deferred Vs-Ghost-on-load: load recording without entering watch mode
    if (m_pendingVsGhostLoad && !m_liveTarget.empty())
    {
        if (GameObjectFactory::Instance().FindGameObject(m_liveTarget))
        {
            m_pendingVsGhostLoad = false;
            LoadRecording(m_pendingRecordingName);
            std::cout << "[GhostSystem] Vs-Ghost recording loaded: " << m_pendingRecordingName << std::endl;
        }
    }

    // Key-0: toggle ghost on/off — disabled during full replay watch mode
    if (!m_isReplayMode)
    {
        bool key0Now = (glfwGetKey(glfwGetCurrentContext(), GLFW_KEY_0) == GLFW_PRESS);
        if (key0Now && !m_key0WasHeld)
            ToggleGhost();
        m_key0WasHeld = key0Now;
    }

    // Recorder samples AFTER the game has updated this tick
    m_recorder.Update(dt);

    // Player drives the ghost object this tick
    m_player.Update(dt);

    // Update media player UI when watching a replay
    if (m_isReplayMode && m_mediaPlayerSpawned)
        UpdateMediaPlayer(dt);

}

// ===== Recording =====

void GhostSystem::StartRecording(const std::string& targetName)
{
    const std::string& target = targetName.empty() ? m_liveTarget : targetName;
    m_recorder.StartRecording(target);
    std::cout << "[GhostSystem] Recording started for: " << targetName << std::endl;
}

void GhostSystem::StopRecording()
{
    m_recorder.StopRecording();
    std::cout << "[GhostSystem] Recording stopped. Frames: "
        << m_recorder.GetRecording().size() << std::endl;
}

bool GhostSystem::SaveBestRun(const std::string& levelName, float finishTime)
{
    if (finishTime >= m_bestTime)
        return false;

    m_bestTime      = finishTime;
    m_bestRecording = m_recorder.GetRecording();

    SaveToDisk(levelName);
    std::cout << "[GhostSystem] New best: " << finishTime << "s — saved to disk." << std::endl;
    return true;
}

// ===== Playback =====

void GhostSystem::StartPlayback(const std::string& ghostObjectName)
{
    if (m_bestRecording.empty())
    {
        std::cout << "[GhostSystem] No recording available — playback skipped." << std::endl;
        return;
    }
    const std::string& target = ghostObjectName.empty() ? m_ghostTarget : ghostObjectName;
    m_player.StartPlayback(target, m_bestRecording);
    std::cout << "[GhostSystem] Ghost playback started on: " << ghostObjectName << std::endl;
}

void GhostSystem::StopPlayback()
{
    m_player.StopPlayback();
}

// ===== Ghost Toggle =====

void GhostSystem::ToggleGhost()
{
    m_ghostVisible = !m_ghostVisible;

    if (m_liveTarget.empty() || m_ghostTarget.empty())
    {
        std::cout << "[GhostSystem] ToggleGhost: Call Configure() first.\n";
        return;
    }

    auto& factory = GameObjectFactory::Instance();
    GameObject* ghost    = factory.FindGameObject(m_ghostTarget);
    GameObject* ghostVfx = m_ghostVfx.empty() ? nullptr : factory.FindGameObject(m_ghostVfx);

    if (m_ghostVisible)
    {
        if (!IsGhostAvailable())
        {
            m_ghostVisible = false;
            std::cout << "[GhostSystem] Toggle: no recording available — ghost stays hidden.\n";
            return;
        }

        // Clone live player → ghost if not already in the scene
        if (!ghost)
        {
            GameObject* live = factory.FindGameObject(m_liveTarget);
            if (live)
            {
                ghost = factory.CloneGameObject(live->GetID(), m_ghostTarget);
                if (ghost)
                {
                    ghost->RemoveComponent<ColliderComponent>();
                    ghost->RemoveComponent<PhysicsComponent>();
                }
            }
        }
        if (!ghostVfx && !m_ghostVfx.empty())
        {
            GameObject* liveVfx = factory.FindGameObject(m_liveVfx);
            if (liveVfx) ghostVfx = factory.CloneGameObject(liveVfx->GetID(), m_ghostVfx);
        }

        if (ghost)    ghost->SetActive(true);
        if (ghostVfx) ghostVfx->SetActive(true);

        // Park ghost at the first recorded frame — playback starts when the player leaves SafeZone
        if (ghost && !m_bestRecording.empty())
        {
            auto* tf = ghost->GetComponent<TransformComponent>();
            if (tf)
            {
                tf->posX   = m_bestRecording[0].posX;
                tf->posY   = m_bestRecording[0].posY;
                tf->scaleX = m_bestRecording[0].scaleX;
                tf->scaleY = m_bestRecording[0].scaleY;
            }
        }
        std::cout << "[GhostSystem] Ghost toggled ON — waiting for SafeZone exit to begin playback.\n";
    }
    else
    {
        m_player.StopPlayback();
        if (ghost)    factory.QueueDestroyGameObject(ghost->GetID());
        if (ghostVfx) factory.QueueDestroyGameObject(ghostVfx->GetID());
        std::cout << "[GhostSystem] Ghost toggled OFF.\n";
    }
}

// ===== Replay Watch Mode =====

void GhostSystem::EnterReplayMode()
{
    if (m_bestRecording.empty())
    {
        std::cout << "[GhostSystem] EnterReplayMode: No recording available.\n";
        return;
    }
    if (m_liveTarget.empty() || m_ghostTarget.empty())
    {
        std::cout << "[GhostSystem] EnterReplayMode: Call Configure() first.\n";
        return;
    }

    auto& factory = GameObjectFactory::Instance();

    // Hide the live player and its VFX
    GameObject* live    = factory.FindGameObject(m_liveTarget);
    GameObject* liveVfx = m_liveVfx.empty() ? nullptr : factory.FindGameObject(m_liveVfx);
    if (live)    live->SetActive(false);
    if (liveVfx) liveVfx->SetActive(false);

    // Create ghost by cloning live player if it doesn't already exist
    GameObject* ghost = factory.FindGameObject(m_ghostTarget);
    if (!ghost && live)
    {
        ghost = factory.CloneGameObject(live->GetID(), m_ghostTarget);
        std::cout << "[GhostSystem] Created " << m_ghostTarget << " by cloning " << m_liveTarget << ".\n";
        if (ghost)
        {
            ghost->RemoveComponent<ColliderComponent>();
            ghost->RemoveComponent<PhysicsComponent>();
        }
    }

    // Create ghost VFX by cloning live VFX if needed
    GameObject* ghostVfx = m_ghostVfx.empty() ? nullptr : factory.FindGameObject(m_ghostVfx);
    if (!ghostVfx && !m_ghostVfx.empty() && liveVfx)
    {
        ghostVfx = factory.CloneGameObject(liveVfx->GetID(), m_ghostVfx);
        std::cout << "[GhostSystem] Created " << m_ghostVfx << " by cloning " << m_liveVfx << ".\n";
    }

    if (ghost)    ghost->SetActive(true);
    if (ghostVfx) ghostVfx->SetActive(true);

    // Switch camera to follow the ghost
    CameraComponent* cam = CameraManager::Instance().GetActiveCamera();
    if (cam && ghost)
        cam->SetFollowTarget(ghost);

    m_player.StartPlayback(m_ghostTarget, m_bestRecording);

    m_isReplayMode = true;
    SpawnMediaPlayer();
    std::cout << "[GhostSystem] Replay mode entered.\n";
}

void GhostSystem::ExitReplayMode()
{
    DestroyMediaPlayer();
    m_player.StopPlayback();

    auto& factory = GameObjectFactory::Instance();

    // Destroy the cloned ghost objects so they don't linger
    GameObject* ghost = factory.FindGameObject(m_ghostTarget);
    if (ghost) factory.QueueDestroyGameObject(ghost->GetID());

    if (!m_ghostVfx.empty())
    {
        GameObject* ghostVfx = factory.FindGameObject(m_ghostVfx);
        if (ghostVfx) factory.QueueDestroyGameObject(ghostVfx->GetID());
    }

    // Restore live player and its VFX
    GameObject* live    = factory.FindGameObject(m_liveTarget);
    GameObject* liveVfx = m_liveVfx.empty() ? nullptr : factory.FindGameObject(m_liveVfx);
    if (live)    live->SetActive(true);
    if (liveVfx) liveVfx->SetActive(true);

    // Switch camera back to live player
    CameraComponent* cam = CameraManager::Instance().GetActiveCamera();
    if (cam && live)
        cam->SetFollowTarget(live);

    m_isReplayMode = false;
    std::cout << "[GhostSystem] Replay mode exited.\n";
}

// ===== Persistence =====

bool GhostSystem::LoadRecording(const std::string& levelName)
{
    return LoadFromDisk(levelName);
}

bool GhostSystem::IsGhostAvailable() const
{
    return !m_bestRecording.empty();
}

// ===== Disk I/O =====

void GhostSystem::SaveToDisk(const std::string& levelName)
{
    std::string path = MakePath(levelName);
    std::ofstream file(path);
    if (!file)
    {
        std::cerr << "[GhostSystem] Failed to open for write: " << path << std::endl;
        return;
    }

    json root;
    root["level"]           = m_levelName;
    root["bestTime"]        = m_bestTime;
    root["shellsCollected"] = m_shellsCollected;

    json frames = json::array();
    for (const auto& kf : m_bestRecording)
    {
        json frame;
        frame["t"]          = kf.timestamp;
        frame["x"]          = kf.posX;
        frame["y"]          = kf.posY;
        frame["sx"]         = kf.scaleX;
        frame["sy"]         = kf.scaleY;
        frame["anim"]       = kf.animName;
        frame["animRow"]    = kf.animRow;
        frame["animCol"]    = kf.animCol;
        frame["vfxAnim"]    = kf.vfxAnimName;
        frame["vfxAnimRow"] = kf.vfxAnimRow;
        frame["vfxAnimCol"] = kf.vfxAnimCol;
        frames.push_back(std::move(frame));
    }
    root["frames"] = std::move(frames);

    file << root.dump(2);
}

void GhostSystem::ResetRecording()
{
    m_recorder.Reset();
    std::cout << "[GhostSystem] Recording reset — buffer cleared.\n";
}

void GhostSystem::SaveCurrentRun(const std::string& levelName)
{
    const auto& rec = m_recorder.GetRecording();
    if (rec.empty())
    {
        std::cout << "[GhostSystem] SaveCurrentRun: no frames recorded — skipped.\n";
        return;
    }

    m_bestRecording = rec;
    m_bestTime      = rec.back().timestamp;

    SaveToDisk(levelName);
    std::cout << "[GhostSystem] SaveCurrentRun: saved " << rec.size()
              << " frames (" << m_bestTime << "s) to '" << levelName << "'.\n";
}

bool GhostSystem::LoadFromDisk(const std::string& levelName)
{
    std::string path = MakePath(levelName);
    std::ifstream file(path);
    if (!file)
    {
        std::cout << "[GhostSystem] No saved recording found for: " << levelName << std::endl;
        return false;
    }

    json root;
    try { file >> root; }
    catch (const json::parse_error& e)
    {
        std::cerr << "[GhostSystem] JSON parse error: " << e.what() << std::endl;
        return false;
    }

    m_levelName      = root.value("level",          std::string{});
    float bestTime   = root.value("bestTime",       FLT_MAX);
    m_shellsCollected = root.value("shellsCollected", 0);
    const auto& frames = root["frames"];

    if (frames.empty())
        return false;

    GhostRecording loaded;
    loaded.reserve(frames.size());

    for (const auto& frame : frames)
    {
        GhostKeyframe kf;
        kf.timestamp   = frame.value("t",          0.0f);
        kf.posX        = frame.value("x",          0.0f);
        kf.posY        = frame.value("y",          0.0f);
        kf.scaleX      = frame.value("sx",         1.0f);
        kf.scaleY      = frame.value("sy",         1.0f);
        kf.animName    = frame.value("anim",       std::string{});
        kf.animRow     = frame.value("animRow",    0);
        kf.animCol     = frame.value("animCol",    0);
        kf.vfxAnimName = frame.value("vfxAnim",   std::string{});
        kf.vfxAnimRow  = frame.value("vfxAnimRow", 0);
        kf.vfxAnimCol  = frame.value("vfxAnimCol", 0);
        loaded.push_back(std::move(kf));
    }

    m_bestTime      = bestTime;
    m_bestRecording = std::move(loaded);

    std::cout << "[GhostSystem] Loaded recording for " << levelName
        << " — " << m_bestRecording.size() << " frames, best time: "
        << m_bestTime << "s" << std::endl;

    return true;
}

// ===== Media Player =====

void GhostSystem::SpawnMediaPlayer()
{
    if (m_mediaPlayerSpawned)
        return;
    Serialization::LoadPrefabWithReturn("Media_Player");
    m_mediaPlayerSpawned = true;
}

void GhostSystem::DestroyMediaPlayer()
{
    if (!m_mediaPlayerSpawned)
        return;

    auto& factory = GameObjectFactory::Instance();
    static const char* names[] = {
        "Media_Player", "Button_Play", "Button_Pause",
        "Replay_forward", "Replay_backward",
        "Replay_box", "Replay_slider", "Replay_bar_brown"
    };
    for (const char* name : names)
    {
        GameObject* obj = factory.FindGameObject(name);
        if (obj) factory.QueueDestroyGameObject(obj->GetID());
    }
    m_mediaPlayerSpawned = false;
    m_sliderDragging     = false;
}

void GhostSystem::UpdateMediaPlayer(float dt)
{
    (void)dt;

    auto& factory = GameObjectFactory::Instance();

    // --- Camera anchor ---
    vec2  camPos = CameraManager::Instance().GetCameraPosition();
    float camX   = camPos.x;
    float camY   = camPos.y;
    float baseY  = camY + MP_Y_OFFSET;

    // Helper: move object and optionally resize it
    auto MoveObj = [&](const char* name, float wx, float wy, float sx = -1.f, float sy = -1.f)
    {
        GameObject* obj = factory.FindGameObject(name);
        if (!obj) return;
        auto* tf = obj->GetComponent<TransformComponent>();
        if (!tf) return;
        tf->posX = wx;
        tf->posY = wy;
        if (sx > 0.f) { tf->scaleX = sx; tf->scaleY = sy; }
    };

    // Reposition static elements
    MoveObj("Replay_bar_brown", camX + MP_BAR_CX, baseY, MP_BAR_W + 10.f, MP_BAR_H);
    MoveObj("Replay_box",       camX + MP_BAR_CX, baseY, MP_BAR_W,        MP_BAR_H);
    MoveObj("Replay_backward",  camX + MP_BWD_X,  baseY, MP_BWD_SIZE,     MP_BWD_SIZE);
    MoveObj("Replay_forward",   camX + MP_FWD_X,  baseY, MP_FWD_SIZE,     MP_FWD_SIZE);

    // Play / Pause button visibility toggle
    bool paused = m_player.IsPaused();
    {
        GameObject* btnPlay  = factory.FindGameObject("Button_Play");
        GameObject* btnPause = factory.FindGameObject("Button_Pause");
        if (btnPlay)  { btnPlay->SetActive(!paused);  MoveObj("Button_Play",  camX + MP_PLAY_X, baseY, MP_PLAY_SIZE, MP_PLAY_SIZE); }
        if (btnPause) { btnPause->SetActive(paused);   MoveObj("Button_Pause", camX + MP_PLAY_X, baseY, MP_PLAY_SIZE, MP_PLAY_SIZE); }
    }

    // Slider position reflects playback progress
    float duration = m_player.GetDuration();
    float clock    = m_player.GetClock();
    float progress = (duration > 0.f) ? (clock / duration) : 0.f;
    progress = (progress < 0.f) ? 0.f : (progress > 1.f) ? 1.f : progress;

    float barLeft = camX + MP_BAR_CX - MP_BAR_W * 0.5f;
    float sliderX = barLeft + progress * MP_BAR_W;
    MoveObj("Replay_slider", sliderX, baseY, MP_SLIDER_W, MP_SLIDER_H);

    // --- Input handling ---
    float mouseX = static_cast<float>(InputManager::GetMouseWorldX());
    float mouseY = static_cast<float>(InputManager::GetMouseWorldY());

    bool mouseDown        = (glfwGetMouseButton(glfwGetCurrentContext(), GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS);
    bool mouseJustPressed = mouseDown && !m_mouseWasDown;
    bool mouseJustReleased = !mouseDown && m_mouseWasDown;
    m_mouseWasDown = mouseDown;

    // Point-in-AABB test (centre + half-extents)
    auto InRect = [](float mx, float my, float cx, float cy, float hw, float hh) -> bool
    {
        return (mx >= cx - hw) && (mx <= cx + hw) && (my >= cy - hh) && (my <= cy + hh);
    };

    // Begin slider drag when clicking inside the timeline bar
    if (mouseJustPressed && !m_sliderDragging)
    {
        if (InRect(mouseX, mouseY, camX + MP_BAR_CX, baseY, MP_BAR_W * 0.5f, MP_BAR_H * 0.5f))
            m_sliderDragging = true;
    }

    // While dragging, seek to the mouse position along the bar
    if (m_sliderDragging && mouseDown)
    {
        float t = (mouseX - barLeft) / MP_BAR_W;
        t = (t < 0.f) ? 0.f : (t > 1.f) ? 1.f : t;
        m_player.SeekAbsolute(t * duration);
    }

    // Release drag
    if (mouseJustReleased)
        m_sliderDragging = false;

    // Button clicks (only on fresh press, not while dragging the slider)
    if (mouseJustPressed && !m_sliderDragging)
    {
        // Play / Pause toggle
        if (InRect(mouseX, mouseY, camX + MP_PLAY_X, baseY, MP_PLAY_SIZE * 0.5f, MP_PLAY_SIZE * 0.5f))
            m_player.TogglePause();

        // Rewind 10 s
        if (InRect(mouseX, mouseY, camX + MP_BWD_X, baseY, MP_BWD_SIZE * 0.5f, MP_BWD_SIZE * 0.5f))
            m_player.Seek(-10.f);

        // Skip forward 10 s
        if (InRect(mouseX, mouseY, camX + MP_FWD_X, baseY, MP_FWD_SIZE * 0.5f, MP_FWD_SIZE * 0.5f))
            m_player.Seek(10.f);
    }
}
