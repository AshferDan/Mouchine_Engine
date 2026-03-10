/* Start Header ************************************************************************/
/*!
\file       GhostSystem.h
\author     Jonathan Fam Qian Wei
\par        Fam.q@digipen.edu
\date       2025
Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*/
/* End Header **************************************************************************/

#pragma once
#include "System.h"
#include "ReplayData.h"
#include "ReplayRecorder.h"
#include "ReplayPlayer.h"
#include <string>
#include <cfloat>

/**
 * @brief Engine system that coordinates ghost player recording and playback.
 *
 * Usage flow (driven from C# scripts via the Ghost API):
 *   1. Call StartRecording("Hero")    at run start.
 *   2. Call StopRecording()           when the run ends.
 *   3. Call SaveBestRun(finishTime)   to persist if it's a new best.
 *   4. Call LoadRecording("M4_LEVEL") on the next run to load the saved ghost.
 *   5. Call StartPlayback("GhostHero") to drive the ghost object.
 */
class GhostSystem : public ISystem
{
public:
    void Initialize() override;
    void Update(float dt) override;
    std::string GetName() override { return "GhostSystem"; }

    // ===== Recording =====

    /** @brief Begin recording targetName's transform and animation each frame. */
    void StartRecording(const std::string& targetName);

    /** @brief Stop recording. Recording is kept in memory until SaveBestRun or StartRecording. */
    void StopRecording();

    /**
     * @brief Compare finishTime against the current best. If better, replace the best
     *        recording, save it to disk, and return true.
     * @param levelName  Used to derive the save file path (e.g. "M4_LEVEL").
     * @param finishTime The player's completion time in seconds.
     * @return True if this run is the new best.
     */
    bool SaveBestRun(const std::string& levelName, float finishTime);

    // ===== Playback =====

    /** @brief Start driving ghostObjectName from the best recording. */
    void StartPlayback(const std::string& ghostObjectName);

    /** @brief Stop driving the ghost object. */
    void StopPlayback();

    /** @brief Toggle pause — ghost freezes in place but stays visible. */
    void ToggleReplayPause() { m_player.TogglePause(); }

    /**
     * @brief Shift the replay clock by deltaSeconds (negative = rewind).
     *        Clamps to [0, recording end]. Safe to call while paused.
     */
    void SeekReplay(float deltaSeconds) { m_player.Seek(deltaSeconds); }

    /** @return True if the replay is currently paused. */
    bool IsReplayPaused() const { return m_player.IsPaused(); }

    // ===== Persistence =====

    /**
     * @brief Load the best recording from disk for the given level.
     *        Call this before StartPlayback on a new session.
     * @param levelName  Level identifier matching what was used in SaveBestRun.
     * @return True if a file was found and loaded successfully.
     */
    bool LoadRecording(const std::string& levelName);

    // ===== Queries =====

    /** @return True if a best recording is available (in memory or loaded from disk). */
    bool IsGhostAvailable() const;

    /** @return The best run time, or FLT_MAX if none recorded yet. */
    float GetBestRunTime() const { return m_bestTime; }

    /** @return True while the ghost is actively playing back. */
    bool IsGhostPlaying() const { return m_player.IsPlaying(); }

    // ===== Replay Watch Mode =====

    /**
     * @brief Enter a dedicated replay watch mode.
     *        Hides the live Hero, switches the camera to follow GhostHero,
     *        and starts playback from the beginning. Only F7/F8/F9/ESC respond.
     *        No-op if no recording is available.
     */
    void EnterReplayMode();

    /**
     * @brief Exit replay watch mode.
     *        Restores the live Hero, switches camera back to Hero, stops playback.
     */
    void ExitReplayMode();

    /** @return True while in the dedicated replay watch mode. */
    bool IsReplayMode() const { return m_isReplayMode; }

    /**
     * @brief Save the current recording to disk unconditionally (no best-time check).
     *        Useful for test recordings where you always want the run saved.
     * @param levelName  Used to derive the save file path (e.g. "Level1_testing").
     */
    void SaveCurrentRun(const std::string& levelName);

    /**
     * @brief Discard any in-progress recording and reset the recorder to a clean state.
     *        Call this when the player dies or restarts so the next run starts fresh.
     */
    void ResetRecording();

    /**
     * @brief Flag the ghost system to auto-load Level1_testing.json and enter
     *        replay watch mode as soon as the next scene finishes loading.
     *        Call this before RequestSceneLoad() from the menu.
     */
    void RequestReplayOnLoad() { m_pendingReplayLoad = true; }

    // ===== Finish Trigger =====

    /**
     * @brief Auto-stop recording and save the run when targetName's X position
     *        reaches or exceeds xThreshold. Fires once per recording session.
     * @param targetName  The GameObject to watch (e.g. "Hero").
     * @param xThreshold  X position that marks the finish line.
     * @param levelName   Level identifier used for the save file.
     */
    void SetFinishTrigger(const std::string& targetName, float xThreshold,
                          const std::string& levelName);

private:
    void SaveToDisk(const std::string& levelName);
    bool LoadFromDisk(const std::string& levelName);

    static std::string MakePath(const std::string& levelName);

    ReplayRecorder m_recorder;
    ReplayPlayer   m_player;
    GhostRecording m_bestRecording;
    float          m_bestTime = FLT_MAX;

    // Finish trigger state
    std::string m_finishTarget;
    float       m_finishX    = FLT_MAX;
    std::string m_finishLevel;
    bool        m_finishDone = false;

    // Replay watch mode
    bool        m_isReplayMode = false;

    // Runtime ghost toggle (key 0)
    bool        m_ghostVisible  = false;
    bool        m_key0WasHeld   = false;
    void        ToggleGhost();

    // Deferred replay: set before scene load, fires once GhostHero exists in the new scene
    bool        m_pendingReplayLoad = false;
};
