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
#include "Factory.h"
#include "transform.h"
#include "CameraManager.h"
#include "CameraComponent.h"
#include <GLFW/glfw3.h>
#include <nlohmann/json.hpp>
using json = nlohmann::ordered_json;

// ===== Helpers =====

std::string GhostSystem::MakePath(const std::string& levelName)
{
    std::filesystem::create_directories("../Assets/Replay");
    return "../Assets/Replay/" + levelName + ".json";
}

// ===== ISystem =====

void GhostSystem::Initialize()
{
    std::cout << "Initializing system: GhostSystem" << std::endl;
}

void GhostSystem::Update(float dt)
{
    // Deferred replay-on-load: wait until Hero exists (scene is ready), then start replay
    if (m_pendingReplayLoad)
    {
        if (GameObjectFactory::Instance().FindGameObject("Hero"))
        {
            m_pendingReplayLoad = false;
            LoadRecording("Level1_testing");
            EnterReplayMode();
        }
    }

    // Key-0: toggle ghost on/off (rising-edge detection)
    bool key0Now = (glfwGetKey(glfwGetCurrentContext(), GLFW_KEY_0) == GLFW_PRESS);
    if (key0Now && !m_key0WasHeld)
        ToggleGhost();
    m_key0WasHeld = key0Now;

    // Recorder samples AFTER the game has updated this tick
    m_recorder.Update(dt);

    // Player drives the ghost object this tick
    m_player.Update(dt);

    // Finish trigger: auto-stop + save when target crosses the X threshold
    if (!m_finishDone && !m_finishTarget.empty() && m_recorder.IsRecording())
    {
        GameObject* obj = GameObjectFactory::Instance().FindGameObject(m_finishTarget);
        if (obj)
        {
            auto* tf = obj->GetComponent<TransformComponent>();
            if (tf)
            {
                float x = 0.f, y = 0.f;
                tf->GetPosition(x, y);

                if (x >= m_finishX)
                {
                    StopRecording();

                    const auto& rec = m_recorder.GetRecording();
                    float runTime = rec.empty() ? 0.f : rec.back().timestamp;

                    bool newBest = SaveBestRun(m_finishLevel, runTime);
                    std::cout << "[GhostSystem] Finish trigger fired at x=" << x
                              << "  time=" << runTime << "s"
                              << (newBest ? "  — NEW BEST saved!" : "  — not a new best.") << "\n";

                    m_finishDone = true;
                }
            }
        }
    }
}

// ===== Recording =====

void GhostSystem::StartRecording(const std::string& targetName)
{
    m_recorder.StartRecording(targetName);
    m_finishDone = false;
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
    m_player.StartPlayback(ghostObjectName, m_bestRecording);
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

    GameObject* ghost    = GameObjectFactory::Instance().FindGameObject("GhostHero");
    GameObject* ghostVfx = GameObjectFactory::Instance().FindGameObject("GhostHero_vfx");

    if (m_ghostVisible)
    {
        if (!IsGhostAvailable())
        {
            m_ghostVisible = false;
            std::cout << "[GhostSystem] Toggle: no recording available — ghost stays hidden.\n";
            return;
        }

        auto& factory = GameObjectFactory::Instance();

        // Clone Hero → GhostHero if not already in the scene
        if (!ghost)
        {
            GameObject* hero = factory.FindGameObject("Hero");
            if (hero) ghost = factory.CloneGameObject(hero->GetID(), "GhostHero");
        }
        if (!ghostVfx)
        {
            GameObject* heroVfx = factory.FindGameObject("Hero_vfx");
            if (heroVfx) ghostVfx = factory.CloneGameObject(heroVfx->GetID(), "GhostHero_vfx");
        }

        if (ghost)    ghost->SetActive(true);
        if (ghostVfx) ghostVfx->SetActive(true);
        m_player.StartPlayback("GhostHero", m_bestRecording);
        std::cout << "[GhostSystem] Ghost toggled ON.\n";
    }
    else
    {
        m_player.StopPlayback();
        auto& factory = GameObjectFactory::Instance();
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
        std::cout << "[GhostSystem] EnterReplayMode: No recording available." << std::endl;
        return;
    }

    auto& factory = GameObjectFactory::Instance();

    // Hide the live player and its VFX
    GameObject* hero    = factory.FindGameObject("Hero");
    GameObject* heroVfx = factory.FindGameObject("Hero_vfx");
    if (hero)    hero->SetActive(false);
    if (heroVfx) heroVfx->SetActive(false);

    // Create GhostHero by cloning Hero if it doesn't already exist in the scene
    GameObject* ghost = factory.FindGameObject("GhostHero");
    if (!ghost && hero)
    {
        ghost = factory.CloneGameObject(hero->GetID(), "GhostHero");
        std::cout << "[GhostSystem] Created GhostHero by cloning Hero.\n";
    }

    // Create GhostHero_vfx by cloning Hero_vfx if needed
    GameObject* ghostVfx = factory.FindGameObject("GhostHero_vfx");
    if (!ghostVfx && heroVfx)
    {
        ghostVfx = factory.CloneGameObject(heroVfx->GetID(), "GhostHero_vfx");
        std::cout << "[GhostSystem] Created GhostHero_vfx by cloning Hero_vfx.\n";
    }

    if (ghost)    ghost->SetActive(true);
    if (ghostVfx) ghostVfx->SetActive(true);

    // Switch camera follow target to GhostHero
    CameraComponent* cam = CameraManager::Instance().GetActiveCamera();
    if (cam && ghost)
        cam->SetFollowTarget(ghost);

    // Start playback from the beginning
    m_player.StartPlayback("GhostHero", m_bestRecording);

    m_isReplayMode = true;
    std::cout << "[GhostSystem] Replay mode entered.\n";
}

void GhostSystem::ExitReplayMode()
{
    m_player.StopPlayback();

    auto& factory = GameObjectFactory::Instance();

    // Destroy the cloned ghost objects so they don't linger
    GameObject* ghost = factory.FindGameObject("GhostHero");
    if (ghost) factory.QueueDestroyGameObject(ghost->GetID());

    GameObject* ghostVfx = factory.FindGameObject("GhostHero_vfx");
    if (ghostVfx) factory.QueueDestroyGameObject(ghostVfx->GetID());

    // Restore live player and its VFX
    GameObject* hero    = factory.FindGameObject("Hero");
    GameObject* heroVfx = factory.FindGameObject("Hero_vfx");
    if (hero)    hero->SetActive(true);
    if (heroVfx) heroVfx->SetActive(true);

    // Switch camera back to Hero
    CameraComponent* cam = CameraManager::Instance().GetActiveCamera();
    if (cam && hero)
        cam->SetFollowTarget(hero);

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
    root["bestTime"] = m_bestTime;

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
    m_finishDone = false;
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

// ===== Finish Trigger =====

void GhostSystem::SetFinishTrigger(const std::string& targetName, float xThreshold,
                                   const std::string& levelName)
{
    m_finishTarget = targetName;
    m_finishX      = xThreshold;
    m_finishLevel  = levelName;
    m_finishDone   = false;
    std::cout << "[GhostSystem] Finish trigger set: watch '" << targetName
              << "' at x>=" << xThreshold << " for level '" << levelName << "'\n";
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

    float bestTime = root.value("bestTime", FLT_MAX);
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
