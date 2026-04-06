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
#include "RenderComponent.h"
#include "ScriptComponent.h"
#include <GLFW/glfw3.h>
#include <nlohmann/json.hpp>
using json = nlohmann::ordered_json;

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

    // Deferred Vs-Ghost-on-load: load recording then create and park the ghost beside the SafeZone
    if (m_pendingVsGhostLoad && !m_liveTarget.empty())
    {
        if (GameObjectFactory::Instance().FindGameObject(m_liveTarget))
        {
            m_pendingVsGhostLoad = false;
            LoadRecording(m_pendingRecordingName);

            // Create GhostHero and GhostHero_vfx parked beside the SafeZone.
            // Playback will start automatically when the player exits the SafeZone
            // (triggered by WinLoseLogic::CheckSafeZoneExit).
            if (!m_ghostVisible)
                ToggleGhost();

            std::cout << "[GhostSystem] Vs-Ghost ready: " << m_pendingRecordingName
                      << " — ghost parked beside SafeZone.\n";
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


}

// ===== Recording =====

void GhostSystem::StartRecording(const std::string& targetName)
{
    const std::string& target = targetName.empty() ? m_liveTarget : targetName;
    m_recorder.SetBgLayers(m_bgLayers[0], m_bgLayers[1], m_bgLayers[2], m_bgLayers[3]);
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

    // Keep the ghost semi-transparent during playback so it is visually distinct
    // from the live hero.
    constexpr float kPlaybackAlpha = 0.3f;
    auto applyAlpha = [](GameObject* obj, float alpha)
    {
        if (!obj) return;
        auto* rc = obj->GetComponent<RenderComponent>();
        if (rc)
        {
            vec4 c = rc->GetColor();
            c.w = alpha;
            rc->SetColor(c);
        }
    };
    auto& fac = GameObjectFactory::Instance();
    applyAlpha(fac.FindGameObject(target), kPlaybackAlpha);
    if (!m_ghostVfx.empty())
        applyAlpha(fac.FindGameObject(m_ghostVfx), kPlaybackAlpha);

    // This function is only called in Vs-Ghost mode (key-0, pause menu, SafeZone exit).
    // Backgrounds are owned by the live ParallaxBackground script — do not replay them.
    // EnterReplayMode() has its own path that does set bg layers for full watch-replay.
    m_player.SetBgLayers("", "", "", "");

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

                    // Remove scripts that must not run on the ghost:
                    // - ParallaxBackground: would shift camera bg layers
                    // - PickupManager / PlayerMovement / InputManager / Knockback:
                    //   these use shared static state and would steal pickups or
                    //   interfere with the live hero's power-up effects.
                    auto* sc = ghost->GetComponent<ScriptComponent>();
                    if (sc)
                    {
                        sc->RemoveScript("ParallaxBackground");
                        sc->RemoveScript("PickupManager");
                        sc->RemoveScript("PlayerMovement");
                        sc->RemoveScript("InputManager");
                        sc->RemoveScript("Knockback");
                    }
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

        // Park ghost and its VFX beside the SafeZone (at the first recorded frame position).
        // Playback will begin when the player leaves the SafeZone (triggered by game logic).
        if (ghost && !m_bestRecording.empty())
        {
            const float parkX = m_bestRecording[0].posX;
            const float parkY = m_bestRecording[0].posY;

            auto* tf = ghost->GetComponent<TransformComponent>();
            if (tf)
            {
                tf->posX   = parkX;
                tf->posY   = parkY;
                tf->scaleX = m_bestRecording[0].scaleX;
                tf->scaleY = m_bestRecording[0].scaleY;
            }

            // Park the VFX companion at the same location (matches the offset applied during playback).
            if (ghostVfx)
            {
                auto* vfxTf = ghostVfx->GetComponent<TransformComponent>();
                if (vfxTf)
                {
                    vfxTf->posX = parkX;
                    vfxTf->posY = parkY - 10.0f;
                }
            }

            // Appear semi-transparent while waiting beside the SafeZone.
            // Opacity is restored to 1.0 when playback begins (StartPlayback).
            constexpr float kParkedAlpha = 0.3f;
            auto setAlpha = [](GameObject* obj, float alpha)
            {
                if (!obj) return;
                auto* rc = obj->GetComponent<RenderComponent>();
                if (rc)
                {
                    vec4 c = rc->GetColor();
                    c.w = alpha;
                    rc->SetColor(c);
                }
            };
            setAlpha(ghost,    kParkedAlpha);
            setAlpha(ghostVfx, kParkedAlpha);
        }
        std::cout << "[GhostSystem] Ghost toggled ON — parked beside SafeZone, waiting for SafeZone exit to begin playback.\n";
    }
    else
    {
        m_vsGhostMode = false;   // session ended
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

            auto* sc = ghost->GetComponent<ScriptComponent>();
            if (sc)
            {
                sc->RemoveScript("ParallaxBackground");
                sc->RemoveScript("PickupManager");
                sc->RemoveScript("PlayerMovement");
                sc->RemoveScript("InputManager");
                sc->RemoveScript("Knockback");
            }
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

    m_player.SetBgLayers(m_bgLayers[0], m_bgLayers[1], m_bgLayers[2], m_bgLayers[3]);
    m_player.StartPlayback(m_ghostTarget, m_bestRecording);
    m_player.TogglePause(); // start paused — player must press play

    m_isReplayMode = true;
    std::cout << "[GhostSystem] Replay mode entered.\n";
}

void GhostSystem::ExitReplayMode()
{
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
    root["rating"]          = m_rating;

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
        frame["bgX0"]       = kf.bgLayerX[0];
        frame["bgX1"]       = kf.bgLayerX[1];
        frame["bgX2"]       = kf.bgLayerX[2];
        frame["bgX3"]       = kf.bgLayerX[3];
        frame["bgY0"]       = kf.bgLayerY[0];
        frame["bgY1"]       = kf.bgLayerY[1];
        frame["bgY2"]       = kf.bgLayerY[2];
        frame["bgY3"]       = kf.bgLayerY[3];
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
    m_rating          = root.value("rating", 0.0f);
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
        kf.vfxAnimName  = frame.value("vfxAnim",   std::string{});
        kf.vfxAnimRow   = frame.value("vfxAnimRow", 0);
        kf.vfxAnimCol   = frame.value("vfxAnimCol", 0);
        kf.bgLayerX[0]  = frame.value("bgX0", 0.f);
        kf.bgLayerX[1]  = frame.value("bgX1", 0.f);
        kf.bgLayerX[2]  = frame.value("bgX2", 0.f);
        kf.bgLayerX[3]  = frame.value("bgX3", 0.f);
        kf.bgLayerY[0]  = frame.value("bgY0", 0.f);
        kf.bgLayerY[1]  = frame.value("bgY1", 0.f);
        kf.bgLayerY[2]  = frame.value("bgY2", 0.f);
        kf.bgLayerY[3]  = frame.value("bgY3", 0.f);
        loaded.push_back(std::move(kf));
    }

    m_bestTime      = bestTime;
    m_bestRecording = std::move(loaded);

    std::cout << "[GhostSystem] Loaded recording for " << levelName
        << " — " << m_bestRecording.size() << " frames, best time: "
        << m_bestTime << "s" << std::endl;

    return true;
}

