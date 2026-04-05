/* Start Header ************************************************************************/
/*!
\file       GameCallbacks.h
\author     Jonathan Fam Qian Wei
\par        Fam.q@digipen.edu
\date       25/4/2026

Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*/
/* End Header **************************************************************************/

#pragma once
#include <string>

/**
 * @interface GameCallBackSystem
 * @brief Abstract interface that the game project implements to receive engine
 *        lifecycle events and inject game-specific system updates.
 *
 *        This decouples the engine from concrete game types: the engine depends
 *        only on this interface, never on MenuLogicSystem, WinLoseLogicSystem, etc.
 *
 *        Register an implementation with CoreEngine::SetGameCallbacks() before
 *        starting the game loop.
 */
class IGameCallbacksSystem
{
public:
    virtual ~IGameCallbacksSystem() = default;

    // ── Update hooks ──────────────────────────────────────────────────────────
    // Called inside the fixed-timestep loop, only while the game is playing.
    virtual void FixedUpdatePlaying(float ) {}

    // Called inside the fixed-timestep loop every tick (even when paused).
    virtual void FixedUpdateAlways(float) {}

    // Called in the variable-timestep section once per frame
    virtual void VariableUpdate(float) {}

    // ── Scene / Editor lifecycle ──────────────────────────────────────────────
    /** Called when the editor Stop button is pressed (before state reverts). */
    virtual void OnEditorStop() {}

    /** Called after LevelEditor::LoadLevel() completes. */
    virtual void OnLevelLoaded(const std::string& /*path*/) {}

    /** Called after a pending scene load finishes (re-init game systems). */
    virtual void OnSceneLoadReinit() {}

    // ── State queries ─────────────────
    virtual bool IsPauseMenuActive()  const { return false; }
    virtual bool IsWinScreenActive()  const { return false; }
    virtual bool IsLoseScreenActive() const { return false; }
    virtual bool IsHowToPlayOpen()    const { return false; }
    virtual bool IsPauseSettingsOpen() const { return false; }
    virtual bool IsGhostMenuOpen()    const { return false; }
    virtual bool IsOnMainMenu()       const { return false; }
    virtual std::string GetCurrentScenePath() const { return ""; }

    // ── Actions (triggered by engine systems) ────────────────────────────────
    virtual void TogglePauseMenu()     {}
    virtual void OpenHowToPlay()       {}
    virtual void CloseHowToPlay()      {}
    virtual void ClosePauseSettings()  {}
    virtual void CloseGhostMenu()      {}
    virtual void TriggerWin()          {}
    virtual void TriggerLose()         {}

    // ── Pickup system ─────────────────────────────────────────────────────────
    virtual void CollectCoin()           {}
    virtual void CollectShell()          {}
    virtual int  GetCurrentCoins()  const { return 0; }
    virtual int  GetCurrentShells() const { return 0; }
    virtual int  GetTotalCoins()    const { return 0; }
    virtual int  GetTotalShells()   const { return 0; }
    virtual void ReinitPickups()         {}

    // ── Timer system ──────────────────────────────────────────────────────────
    virtual float GetRemainingTime()          const { return 0.0f; }
    virtual float GetLevelTimerLength()       const { return 60.0f; }
    virtual void  SetLevelTimerLength(float)        {}
    virtual void  AddTime(float)                    {}

    // ── Audio fade durations (loaded from scene metadata) ────────────────────
    virtual void SetAudioFadeDurations(float /*winIn*/, float /*loseIn*/, float /*replayOut*/, float /*quitOut*/, float /*nextOut*/, float /*buffer*/) {}
};
