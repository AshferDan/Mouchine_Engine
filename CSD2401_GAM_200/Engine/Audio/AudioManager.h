/* Start Header ************************************************************************/
/*!
\file       AudioManager.cpp
\author     Leck Xin Yee [100% - 76 lines]
\par        xinyee.leck@digipen.edu
\date       November, 29, 2025

Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*/
/* End Header **************************************************************************/

#pragma once
#pragma once
#include "audio.h"
#include "IMessageHandler.h"
#include <string>
#include <map>
#include <vector>

/**
 * @struct SoundData
 * @brief Stores all properties required to load and play a sound.
 */
struct SoundData {
    std::string name;
    std::string filepath;
    bool loop = false;
    float volume = 1.0f;
    float pitch = 1.0f;
    std::string category;
};

/**
 * @struct AudioEvent
 * @brief Maps an event name to the sound that should be triggered.
 */
struct AudioEvent {
    std::string eventName;
    std::string soundName;
};


/**
 * @class AudioManager
 * @brief High-level manager for sound loading, playback, categories, events, and mute logic.
 */
class AudioManager : public IMessageHandler {
private:
    AudioEngine* audioEngine;

    // store loaded sound data
    std::map<std::string, SoundData> soundDatabase;

    // event to sound mappings
    std::map<std::string, std::string> eventToSoundMap;

    // category volumes (music, sfx, voice, etc.)
    std::map<std::string, float> categoryVolumes;

    // base volumes to save
    std::map<std::string, float> baseCategoryVolumes;
    float masterVolume = 1.0f;

    // Mute state
    bool m_isMuted;
    std::map<std::string, float> m_preMuteCategoryVolumes;
    std::map<std::string, bool> m_categoryMuteStates;
    std::map<std::string, float> m_preMuteIndividualCategoryVolumes;

public:
    /**
     * @brief Constructs the AudioManager with an existing AudioEngine.
     * @param engine Pointer to AudioEngine used for low-level playback.
     */
    AudioManager(AudioEngine* engine);

    /**
     * @brief Destroys the AudioManager and stops all sounds.
     */
    ~AudioManager();

    /**
     * @brief Loads a single sound based on metadata.
     * @param sound Structure containing sound properties.
     * @return True if loading succeeded.
     */
    bool LoadSound(const SoundData& sound);

    /**
     * @brief Loads all sounds stored in the sound database.
     * @return True if all sounds loaded successfully.
     */
    bool LoadAllSounds();

    /**
     * @brief Triggers an event, playing its mapped sound.
     * @param eventName Name of the event.
     */
    void TriggerEvent(const std::string& eventName);

    /**
     * @brief Plays a sound by name.
     * @param soundName Identifier of the sound.
     * @param channelName Optional channel override.
     */
    void PlaySound(const std::string& soundName, const std::string& channelName = "");

    /**
     * @brief Sets the volume of a specific sound.
     * @param soundName Name of the sound.
     * @param volume New volume (0.0�1.0).
     */
    void SetSoundVolume(const std::string& soundName, float volume);

    /**
     * @brief Sets the volume of a sound category.
     * @param category Category name (music, sfx, etc.)
     * @param volume New volume (0.0�1.0).
     */
    void SetCategoryVolume(const std::string& category, float volume);

    /**
     * @brief Gets the volume of a sound category.
     * @param category Category name (music, sfx, etc.)
     * @return Current volume (0.0�1.0), or 1.0 if category not found.
     */
    float GetCategoryVolume(const std::string& category) const;

    /**
     * @brief Gets the base volume of a category before master multiplier.
     * @param category Category name (music, sfx, etc.)
     * @return Base volume (0.0�1.0), or 1.0 if category not found.
     */
    float GetBaseCategoryVolume(const std::string& category) const;

    /**
     * @brief Sets the base volume of a category before master multiplier.
     * @param category Category name (music, sfx, etc.)
     * @param volume Base volume (0.0�1.0).
     */
    void SetBaseCategoryVolume(const std::string& category, float volume);

    /**
     * @brief Gets the master volume multiplier.
     * @return Master volume (0.0�1.0).
     */
    float GetMasterVolume() const;

    /**
     * @brief Sets the master volume multiplier and applies to all categories.
     * @param volume Master volume (0.0�1.0).
     */
    void SetMasterVolume(float volume);

    /**
     * @brief Sets pitch for a specific sound.
     * @param soundName Name of the sound.
     * @param pitch Pitch multiplier.
     */
    void SetSoundPitch(const std::string& soundName, float pitch);

    /**
     * @brief Stops playback on a specific channel.
     * @param channelName Name of the playback channel.
     */
    void StopSound(const std::string& channelName);

    /**
     * @brief Stops all active sound channels.
     */
    void StopAllSounds();

    /**
     * @brief Pauses or resumes all sounds.
     * @param pause True to pause, false to resume.
     */
    void PauseAllSounds(bool pause);

    /**
     * @brief Toggles muting on the FMOD master channel group.
     */
    void ToggleMute();

    /**
     * @brief Toggles muting on the FMOD master channel group.
     */
    void ReloadSound(const std::string& soundName);

    /**
     * @brief Toggles the AudioManager's master mute state.
     */
    void ToggleMasterMute();

    /**
     * @brief Checks whether the audio manager is currently muted.
     * @return True if muted.
     */
    bool IsMuted() const;

    /**
     * @brief Toggles muting for a specific audio category.
     * @param category The category to toggle (e.g., "music", "sfx").
     */
    void ToggleCategoryMute(const std::string& category);

    /**
     * @brief Checks whether a specific category is currently muted.
     * @param category The category to check.
     * @return True if the category is muted.
     */
    bool IsCategoryMuted(const std::string& category) const;

    /**
     * @brief Handles PlaySoundMessage from the message bus.
     * @param msg Message containing sound name.
     */

    void HandleMessage(const PlaySoundMessage& msg) override;

    /**
     * @brief Handles PauseSoundMessage from the message bus.
     * @param msg Message containing channel and pause flag.
     */
    void HandleMessage(const PauseSoundMessage& msg) override;

    /**
     * @brief Prints all sound configuration (debug only).
     */
    void PrintConfig() const;

    // serialisation stuff
    /**
     * @brief Gets the internal sound database (modifiable).
     */
    const std::map<std::string, SoundData>& GetSoundDatabase() const { return soundDatabase; }

    /**
     * @brief Gets the event-to-sound mapping table.
     */
    std::map<std::string, SoundData>& GetSoundDatabase() { return soundDatabase; }

    /**
     * @brief Gets the event-to-sound mapping table.
     */
    const std::map<std::string, std::string>& GetEventMappings() const { return eventToSoundMap; }

    /**
     * @brief Registers a new sound entry.
     * @param sound Sound metadata.
     */
    void AddSound(const SoundData& sound);

    /**
     * @brief Assigns a sound to a specific event.
     * @param eventName Name of event.
     * @param soundName Name of sound.
     */
    void AddEventMapping(const std::string& eventName, const std::string& soundName);

    /**
     * @brief Removes a sound and any associated events.
     * @param soundName Name of the sound.
     */
    void RemoveSound(const std::string& soundName);

    /**
     * @brief Clears all sound data and event mappings.
     */
    void Clear();

    /**
     * @brief Starts fading out a specific category of audio.
     * @param category The category to fade out (e.g., "music", "sfx").
     * @param duration The duration of the fade in seconds.
     */
    void FadeOutCategory(const std::string& category, float duration);

    /**
     * @brief Starts fading in a specific category of audio.
     * @param category The category to fade in (e.g., "music", "sfx").
     * @param duration The duration of the fade in seconds.
     * @param targetVolume The target volume to fade to (default 1.0).
     */
    void FadeInCategory(const std::string& category, float duration, float targetVolume = 1.0f);

    /**
     * @brief Starts fading out all audio categories.
     * @param duration The duration of the fade in seconds.
     */
    void FadeOutAll(float duration);

    /**
     * @brief Updates the fade state (should be called every frame).
     * @param dt Delta time in seconds.
     */
    void UpdateFade(float dt);

    /**
     * @brief Resets all fade states and restores default category volumes.
     */
    void ResetFadeStates();

    /**
     * @brief Fade state tracking structure (public for UI access).
     */
    struct FadeState {
        bool isFading = false;
        bool isFadeIn = false;  // true for fade in, false for fade out
        float startVolume = 1.0f;
        float targetVolume = 1.0f;
        float currentTime = 0.0f;
        float duration = 1.0f;
    };

    /**
     * @brief Gets the fade states for all categories (for UI display).
     */
    const std::map<std::string, FadeState>& GetFadeStates() const { return categoryFadeStates; }

    /**
     * @brief Checks if any category is currently fading.
     * @return True if any fade is in progress.
     */
    bool IsAnyFadeActive() const;

    /*  3D Audio / Spatial Sound  */
    /**
     * @brief Sets the 3D position of the audio listener (usually the camera/player).
     * @param x X position in world space.
     * @param y Y position in world space.
     */
    void Set3DListenerPosition(float x, float y);

    /**
     * @brief Sets the 3D position of a sound source.
     * @param channelName Name of the channel playing the sound.
     * @param x X position of the sound source.
     * @param y Y position of the sound source.
     */
    void Set3DSoundPosition(const std::string& channelName, float x, float y);

    /**
     * @brief Loads a 3D positional sound with distance attenuation and panning.
     * @param name Identifier for the sound.
     * @param filepath Path to the audio file.
     * @param loop Whether the sound should loop.
     * @param minDistance Minimum distance before sound starts attenuating.
     * @param maxDistance Maximum distance where sound is silent.
     * @param category Audio category (music, sfx, voice, etc.).
     * @param volume Initial volume (0.0-1.0).
     * @param pitch Pitch multiplier.
     * @return True if loading was successful.
     */
    bool Load3DSound(const std::string& name, const std::string& filepath, bool loop = false,
                     float minDistance = 100.0f, float maxDistance = 1000.0f,
                     const std::string& category = "sfx", float volume = 1.0f, float pitch = 1.0f);

    /*  DSP Effects  */
    /**
     * @brief Enables or disables a low pass filter on a specific sound
     * @param soundName Name of the sound to apply the filter to
     * @param enable True to enable the filter, false to disable
     * @param cutoffFrequency Frequency in Hz above which sounds are attenuated (default: 5000 Hz)
     * @param resonance Filter resonance or Q factor (default: 1.0)
     * @return True if the operation was successful
     */
    bool SetLowPassFilter(const std::string& soundName, bool enable, float cutoffFrequency = 5000.0f, float resonance = 1.0f);

    /**
     * @brief Updates the cutoff frequency of an existing low pass filter on a sound
     * @param soundName Name of the sound with the filter
     * @param cutoffFrequency New cutoff frequency in Hz (range: 10-22000)
     * @return True if the operation was successful
     */
    bool UpdateLowPassFilterFrequency(const std::string& soundName, float cutoffFrequency);

    /**
     * @brief Enables or disables a low pass filter on all audio
     * @param enable True to enable the filter, false to disable
     * @param cutoffFrequency Frequency in Hz above which sounds are attenuated (default: 5000 Hz)
     * @param resonance Filter resonance or Q factor (default: 1.0)
     * @return True if the operation was successful
     */
    bool SetMasterLowPassFilter(bool enable, float cutoffFrequency = 5000.0f, float resonance = 1.0f);

    /**
     * @brief Updates the cutoff frequency of the master low pass filter
     * @param cutoffFrequency New cutoff frequency in Hz (range: 10-22000)
     * @return True if the operation was successful
     */
    bool UpdateMasterLowPassFilterFrequency(float cutoffFrequency);

    /**
     * @brief Applies a low pass filter to all currently playing sounds, excluding the specified ones.
     *        Also stores the active state so sounds started after this call get the filter too.
     * @param enable True to enable, false to disable
     * @param cutoffFrequency Cutoff frequency in Hz (default: 800 Hz)
     * @param resonance Filter resonance/Q factor (default: 1.0)
     * @param excluded Sound names that should NOT be filtered
     */
    void SetLowPassFilterExcluding(bool enable, float cutoffFrequency,
                                   float resonance,
                                   const std::vector<std::string>& excluded);

    /**
     * @brief Saves current audio settings from AUDIO SETTINGS!
     * @return True if settings were saved successfully.
     */
    bool SaveSettings();

    /**
     * @brief Loads audio settings from audio settings
     * @return True if settings were loaded successfully.
     */
    bool LoadSettings();

private:
    std::map<std::string, FadeState> categoryFadeStates;

    // Global low-pass filter state (used to auto-apply on PlaySound)
    bool                     m_globalLowPassActive    = false;
    float                    m_globalLowPassCutoff    = 800.0f;
    float                    m_globalLowPassResonance = 1.0f;
    std::vector<std::string> m_lowPassExclusions;
};