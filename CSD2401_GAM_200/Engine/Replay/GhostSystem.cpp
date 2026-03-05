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
#include <cstdint>
#include "Factory.h"
#include "transform.h"

// ===== Helpers =====

std::string GhostSystem::MakePath(const std::string& levelName)
{
    std::filesystem::create_directories("../Assets/Replay");
    return "../Assets/Replay/" + levelName + ".ghost";
}

// ===== ISystem =====

void GhostSystem::Initialize()
{
    std::cout << "Initializing system: GhostSystem" << std::endl;
}

void GhostSystem::Update(float dt)
{
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
    std::ofstream file(path, std::ios::binary | std::ios::trunc);
    if (!file)
    {
        std::cerr << "[GhostSystem] Failed to open for write: " << path << std::endl;
        return;
    }

    // Header
    file.write(reinterpret_cast<const char*>(&m_bestTime), sizeof(float));
    uint32_t count = static_cast<uint32_t>(m_bestRecording.size());
    file.write(reinterpret_cast<const char*>(&count), sizeof(uint32_t));

    // Keyframes
    for (const auto& kf : m_bestRecording)
    {
        file.write(reinterpret_cast<const char*>(&kf.timestamp), sizeof(float));
        file.write(reinterpret_cast<const char*>(&kf.posX),      sizeof(float));
        file.write(reinterpret_cast<const char*>(&kf.posY),      sizeof(float));
        file.write(reinterpret_cast<const char*>(&kf.scaleX),    sizeof(float));
        file.write(reinterpret_cast<const char*>(&kf.scaleY),    sizeof(float));

        uint16_t nameLen = static_cast<uint16_t>(kf.animName.size());
        file.write(reinterpret_cast<const char*>(&nameLen), sizeof(uint16_t));
        file.write(kf.animName.c_str(), nameLen);

        file.write(reinterpret_cast<const char*>(&kf.animRow), sizeof(int));
        file.write(reinterpret_cast<const char*>(&kf.animCol), sizeof(int));
    }
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
    std::ifstream file(path, std::ios::binary);
    if (!file)
    {
        std::cout << "[GhostSystem] No saved recording found for: " << levelName << std::endl;
        return false;
    }

    float    bestTime = FLT_MAX;
    uint32_t count    = 0;

    file.read(reinterpret_cast<char*>(&bestTime), sizeof(float));
    file.read(reinterpret_cast<char*>(&count),    sizeof(uint32_t));

    if (!file || count == 0)
        return false;

    GhostRecording loaded;
    loaded.reserve(count);

    for (uint32_t i = 0; i < count; ++i)
    {
        GhostKeyframe kf;
        file.read(reinterpret_cast<char*>(&kf.timestamp), sizeof(float));
        file.read(reinterpret_cast<char*>(&kf.posX),      sizeof(float));
        file.read(reinterpret_cast<char*>(&kf.posY),      sizeof(float));
        file.read(reinterpret_cast<char*>(&kf.scaleX),    sizeof(float));
        file.read(reinterpret_cast<char*>(&kf.scaleY),    sizeof(float));

        uint16_t nameLen = 0;
        file.read(reinterpret_cast<char*>(&nameLen), sizeof(uint16_t));
        kf.animName.resize(nameLen);
        file.read(kf.animName.data(), nameLen);

        file.read(reinterpret_cast<char*>(&kf.animRow), sizeof(int));
        file.read(reinterpret_cast<char*>(&kf.animCol), sizeof(int));

        if (!file)
            break;

        loaded.push_back(std::move(kf));
    }

    m_bestTime      = bestTime;
    m_bestRecording = std::move(loaded);

    std::cout << "[GhostSystem] Loaded recording for " << levelName
        << " — " << m_bestRecording.size() << " frames, best time: "
        << m_bestTime << "s" << std::endl;

    return true;
}
