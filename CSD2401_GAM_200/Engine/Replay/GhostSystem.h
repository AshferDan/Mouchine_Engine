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
    /**
     * @brief Called once by the engine at system startup. 
     *        Resets the recorder, player, and all internal flags.
     */
    void Initialize() override;

    /**
     * @brief Called each fixed-timestep tick.
     *        Advances the recorder (if recording) or player (if playing back),
     *        and handles the key-0 ghost-visibility toggle.
     * @param dt Fixed delta time for this tick (seconds).
     */
    void Update(float dt) override;

    /**
     * @brief Gets the system identifier name.
     * @return The string "GhostSystem".
     */
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

    /**
     * @brief Supply the background layer object names to record and replay.
     *        Positions of these objects are sampled each frame during recording and
     *        restored each frame during playback. Pass "" for unused slots.
     *        Call before StartRecording / EnterReplayMode.
     * @param b0..b3 Names of up to 4 background layer GameObjects.
     */
    void ConfigureBackground(const std::string& b0, const std::string& b1 = "",
                             const std::string& b2 = "", const std::string& b3 = "")
    {
        m_bgLayers[0] = b0; m_bgLayers[1] = b1;
        m_bgLayers[2] = b2; m_bgLayers[3] = b3;
    }

    /**
     * @brief Sets the human-readable level name that is embedded into saved replays.
     *        Call this from game code before SaveCurrentRun / SaveBestRun.
     * @param levelName The name of the level (e.g. "M4_LEVEL_FIXED").
     */
    void SetLevelName(const std::string& levelName) { m_levelName = levelName; }

    /**
     * @brief Gets the level name stored in the most recently loaded recording.
     * @return The level name as a string (empty if none).
     */
    std::string GetLevelName() const { return m_levelName; }

    // ===== Recording =====

    /**
     * @brief Begin recording the specified target's transform and animation each frame.
     * @param targetName Name of the target to record. If empty, uses the liveTarget set in Configure().
     */
    void StartRecording(const std::string& targetName = "");

    /**
     * @brief Stops the current recording process. 
     *        The recording is kept in memory until SaveBestRun or the next StartRecording.
     */
    void StopRecording();

    /**
     * @brief Compares finishTime against the current best. If better, replaces the best
     *        recording, saves it to disk, and returns true.
     * @param levelName  Used to derive the save file path (e.g. "M4_LEVEL").
     * @param finishTime The player's completion time in seconds.
     * @return True if this run is the new best and was saved.
     */
    bool SaveBestRun(const std::string& levelName, float finishTime);

    // ===== Playback =====

    /**
     * @brief Starts driving the ghost object from the best recording.
     * @param ghostObjectName Name of the ghost object. If empty, uses the ghostTarget set in Configure().
     */
    void StartPlayback(const std::string& ghostObjectName = "");

    /**
     * @brief Stops driving the ghost object and ends playback.
     */
    void StopPlayback();

    /**
     * @brief Toggles the pause state of the replay playback.
     *        When paused, the ghost freezes in place but remains visible.
     */
    void ToggleReplayPause() { m_player.TogglePause(); }

    /**
     * @brief Shifts the replay clock by the specified delta time.
     *        Clamps to [0, recording end]. Safe to call while paused.
     * @param deltaSeconds The amount of time to shift (negative to rewind).
     */
    void SeekReplay(float deltaSeconds) { m_player.Seek(deltaSeconds); }

    /**
     * @brief Checks if the replay playback is currently paused.
     * @return True if the replay is paused.
     */
    bool IsReplayPaused() const { return m_player.IsPaused(); }

    // ===== Persistence =====

    /**
     * @brief Loads the best recording from disk for the given level.
     *        Call this before StartPlayback on a new session.
     * @param levelName Level identifier matching what was used in SaveBestRun.
     * @return True if a file was found and loaded successfully.
     */
    bool LoadRecording(const std::string& levelName);

    // ===== Queries =====

    /**
     * @brief Checks if a best recording is available in memory or loaded from disk.
     * @return True if a ghost recording is available.
     */
    bool IsGhostAvailable() const;

    /**
     * @brief Gets the best run time recorded so far.
     * @return The best run time in seconds, or FLT_MAX if none.
     */
    float GetBestRunTime() const { return m_bestTime; }

    /**
     * @brief Sets the number of shells collected during the current run.
     *        Call this before SaveCurrentRun / SaveBestRun.
     * @param count Shells collected this run.
     */
    void SetShellsCollected(int count) { m_shellsCollected = count; }

    /**
     * @brief Gets the shells collected count stored in the last loaded/saved recording.
     * @return Shell count.
     */
    int GetShellsCollected() const { return m_shellsCollected; }

    /**
     * @brief Sets the rating achieved during the current run.
     *        Call this before SaveCurrentRun / SaveBestRun.
     * @param rating Rating percentage (0-100).
     */
    void SetRating(float rating) { m_rating = rating; }

    /**
     * @brief Gets the rating stored in the last loaded/saved recording.
     * @return Rating percentage (0-100).
     */
    float GetRating() const { return m_rating; }

    /**
     * @brief Checks if the ghost is actively playing back.
     * @return True while the ghost is playing.
     */
    bool IsGhostPlaying() const { return m_player.IsPlaying(); }

    // ===== Media Player / Replay UI =====

    /**
     * @brief Returns the current replay playback time in seconds.
     */
    float GetReplayClock() const { return m_player.GetClock(); }

    /**
     * @brief Returns the total duration of the loaded recording in seconds.
     */
    float GetReplayDuration() const { return m_player.GetDuration(); }

    /**
     * @brief Seeks the replay to an absolute time (clamped to [0, duration]).
     * @param t Target time in seconds.
     */
    void SeekReplayAbsolute(float t) { m_player.SeekAbsolute(t); }

    // ===== Replay Watch Mode =====

    /**
     * @brief Enters the dedicated replay watch mode using configured object names.
     *        Clones liveTarget to ghostTarget, hides live player, and switches camera.
     */
    void EnterReplayMode();

    /**
     * @brief Exits the replay watch mode and restores the original scene state.
     *        Destroys the ghost clone, restores the live player, and resets camera.
     */
    void ExitReplayMode();

    /**
     * @brief Checks if the system is currently in replay watch mode.
     * @return True while in replay watch mode.
     */
    bool IsReplayMode() const { return m_isReplayMode; }

    /**
     * @brief Checks if the ghost is currently toggled visible (via key-0 toggle).
     * @return True if the ghost is visible.
     */
    bool IsGhostVisible() const { return m_ghostVisible; }

    /**
     * @brief Resets the ghost visibility toggle to hidden.
     *        Should be called at the start of each level session.
     */
    void ResetGhostToggle() { m_ghostVisible = false; }

    /**
     * @brief Programmatically toggles ghost visibility (identical to pressing key-0).
     */
    void ToggleGhostVisible() { ToggleGhost(); }

    /**
     * @brief Saves the current recording to disk unconditionally (no best-time check).
     * @param levelName Used to derive the save file path.
     */
    void SaveCurrentRun(const std::string& levelName);

    /**
     * @brief Discards any in-progress recording and resets the recorder state.
     *        Call this when the player dies or restarts.
     */
    void ResetRecording();

    /**
     * @brief Flags the system to auto-load a recording and enter replay mode after scene load.
     * @param recordingName The name of the recording file to load.
     */
    void RequestReplayOnLoad(const std::string& recordingName)
    {
        m_pendingRecordingName = recordingName;
        m_pendingReplayLoad    = true;
    }

    /**
     * @brief Flags the system to auto-load a recording after scene load WITHOUT entering watch mode.
     *        The ghost will be available for the key-0 toggle (Vs Ghost mode).
     * @param recordingName The name of the recording file to load.
     */
    void RequestVsGhostOnLoad(const std::string& recordingName)
    {
        m_pendingRecordingName  = recordingName;
        m_pendingVsGhostLoad    = true;
    }

    // ===== Finish Trigger =====

private:
    void SaveToDisk(const std::string& levelName);
    bool LoadFromDisk(const std::string& levelName);

    std::string MakePath(const std::string& fileName);

    ReplayRecorder m_recorder;
    ReplayPlayer   m_player;
    GhostRecording m_bestRecording;
    float          m_bestTime        = FLT_MAX;
    int            m_shellsCollected = 0;
    float          m_rating          = 0.0f;

    // Game-supplied object names (set via Configure())
    std::string m_liveTarget;   ///< e.g. "Hero"
    std::string m_ghostTarget;  ///< e.g. "GhostHero"
    std::string m_liveVfx;      ///< e.g. "Hero_vfx"  (optional)
    std::string m_ghostVfx;     ///< e.g. "GhostHero_vfx" (optional)
    std::string m_levelName;    ///< Human-readable level name serialised into replay JSON
    std::string m_bgLayers[4];  ///< Background layer object names (set via ConfigureBackground())

    // Replay watch mode
    bool        m_isReplayMode = false;

    // Runtime ghost toggle (key 0)
    bool        m_ghostVisible  = false;
    bool        m_key0WasHeld   = false;
    void        ToggleGhost();

    // Deferred replay: set before scene load, fires once liveTarget exists in the new scene
    bool        m_pendingReplayLoad    = false;
    bool        m_pendingVsGhostLoad   = false;
    std::string m_pendingRecordingName;
};
