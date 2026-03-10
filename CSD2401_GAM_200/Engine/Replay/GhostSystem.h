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
 * Usage flow:
 *   1. Call Configure("Player", "GhostPlayer")  once after the system is created.
 *   2. Call StartRecording()    at run start.
 *   3. Call StopRecording()     when the run ends.
 *   4. Call SaveBestRun(levelName, finishTime) to persist if it's a new best.
 *   5. Call LoadRecording(levelName) on the next run to load the saved ghost.
 *   6. Call StartPlayback()     to drive the ghost object.
 *
 * No game-specific object names are hardcoded in the engine.
 * All names are supplied by the game via Configure().
 */
class GhostSystem : public ISystem
{
public:
    void Initialize() override;
    void Update(float dt) override;
    std::string GetName() override { return "GhostSystem"; }

    // ===== Configuration =====

    /**
     * @brief Supply the game-specific object names the ghost system operates on.
     *        Call this once from game code before using EnterReplayMode / ToggleGhost.
     * @param liveTarget   Name of the live player GameObject (e.g. "Hero").
     * @param ghostTarget  Name of the ghost GameObject to create/drive (e.g. "GhostHero").
     * @param liveVfx      Optional VFX companion of the live player (e.g. "Hero_vfx").
     * @param ghostVfx     Optional VFX companion of the ghost (e.g. "GhostHero_vfx").
     */
    void Configure(const std::string& liveTarget,
                   const std::string& ghostTarget,
                   const std::string& liveVfx  = "",
                   const std::string& ghostVfx = "")
    {
        m_liveTarget  = liveTarget;
        m_ghostTarget = ghostTarget;
        m_liveVfx     = liveVfx;
        m_ghostVfx    = ghostVfx;
    }

    // ===== Recording =====

    /**
     * @brief Begin recording targetName's transform and animation each frame.
     *        If omitted, uses the liveTarget set in Configure().
     */
    void StartRecording(const std::string& targetName = "");

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

    /**
     * @brief Start driving the ghost object from the best recording.
     *        If omitted, uses the ghostTarget set in Configure().
     */
    void StartPlayback(const std::string& ghostObjectName = "");

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
     * @brief Enter replay watch mode using the names set in Configure().
     *        Clones liveTarget → ghostTarget if the ghost doesn't exist,
     *        hides the live player, switches camera to the ghost, starts playback.
     *        No-op if no recording is available or Configure() was not called.
     */
    void EnterReplayMode();

    /**
     * @brief Exit replay watch mode.
     *        Destroys the cloned ghost, restores the live player, resets camera.
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
     * @brief Flag the ghost system to auto-load a recording and enter replay watch
     *        mode as soon as the next scene finishes loading.
     *        Call Configure() and then this before RequestSceneLoad() from the menu.
     * @param recordingName  The recording file to load (e.g. "Level1_testing").
     */
    void RequestReplayOnLoad(const std::string& recordingName)
    {
        m_pendingRecordingName = recordingName;
        m_pendingReplayLoad    = true;
    }

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

    // Game-supplied object names (set via Configure())
    std::string m_liveTarget;   ///< e.g. "Hero"
    std::string m_ghostTarget;  ///< e.g. "GhostHero"
    std::string m_liveVfx;      ///< e.g. "Hero_vfx"  (optional)
    std::string m_ghostVfx;     ///< e.g. "GhostHero_vfx" (optional)

    // Replay watch mode
    bool        m_isReplayMode = false;

    // Runtime ghost toggle (key 0)
    bool        m_ghostVisible  = false;
    bool        m_key0WasHeld   = false;
    void        ToggleGhost();

    // Deferred replay: set before scene load, fires once liveTarget exists in the new scene
    bool        m_pendingReplayLoad    = false;
    std::string m_pendingRecordingName;
};
