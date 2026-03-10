/* Start Header ************************************************************************/
/*!
\file       audio.cpp
\author     Leck Xin Yee [100% - 191 lines]
\par        xinyee.leck@digipen.edu
\date       October, 2, 2025

Copyright (C) 2025 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents
without the prior written consent of DigiPen Institute of
Technology is prohibited.
*/
/* End Header **************************************************************************/

#include "audio.h"
#include <algorithm>

AudioEngine::AudioEngine() : sounds(), channels(), lowPassFilters(), system(nullptr), masterLowPassFilter(nullptr) {}
AudioEngine::~AudioEngine() { stopAudio(); }

/*  Core Functionalities  */
/**
 * @brief Initializes the FMOD audio system with 32 max concurrent sounds
 */
bool AudioEngine::initialiseAudio() {
	FMOD_RESULT result;

	result = FMOD::System_Create(&system);
	if (result != FMOD_OK) {
		checkError(result, "System_Create");
		return false;
	}

	// set 32 as max amt of sounds playing at once
	result = system->init(32, FMOD_INIT_NORMAL, nullptr);
	if (result != FMOD_OK) {
		checkError(result, "System init");
		return false;
	}

	// Set 3D settings for spatial audio
	// distancefactor: units per meter (1.0 = 1 unit = 1 meter)
	// rolloffscale: how quickly sound attenuates (1.0 = normal, higher = faster attenuation)
	result = system->set3DSettings(1.0f, 1.0f, 1.0f);
	if (result != FMOD_OK) {
		checkError(result, "Setting 3D audio settings");
	}

	// Initialize listener position at origin
	FMOD_VECTOR position = { 0.0f, 0.0f, 0.0f };
	FMOD_VECTOR velocity = { 0.0f, 0.0f, 0.0f };
	FMOD_VECTOR forward = { 0.0f, 0.0f, 1.0f };
	FMOD_VECTOR up = { 0.0f, 1.0f, 0.0f };
	result = system->set3DListenerAttributes(0, &position, &velocity, &forward, &up);
	if (result != FMOD_OK) {
		checkError(result, "Setting initial 3D listener position");
	}

	std::cout << "Audio initialised sucessfully" << std::endl;
	return true;
}

/**
 * @brief Updates the FMOD audio system (must be called every frame)
 */
void AudioEngine::updateAudio() {
	// must be called every frame in game loop
	if (system) {
		system->update();
	}
}

/**
 * @brief Stops all audio, unloads all sounds, and shuts down the system
 */
void AudioEngine::stopAudio() {
	stopAllSound();
	unloadAllSound();

	// releases allt he low pass filters
	for (auto& pair : lowPassFilters) {
		if (pair.second) {
			pair.second->release();
		}
	}
	lowPassFilters.clear();

	if (masterLowPassFilter) {
		masterLowPassFilter->release();
		masterLowPassFilter = nullptr;
	}

	if (system) {
		system->close();
		system->release();
		system = nullptr;
	}

	std::cout << "Audio stopped successfully" << std::endl;
}

/*   Sound loading  */
/**
 * @brief Registers a pre-existing FMOD sound with a name identifier
 */
bool AudioEngine::registerSound(const std::string& name, FMOD::Sound* sound) {
	if (!system) {
		std::cout << "Audio system not initialised" << std::endl;
		return false;
	}

	// check if sound already exists
	if (sounds.find(name) != sounds.end()) {
		std::cout << "Sound " << name << " already registered" << std::endl;
		return true;
	}

	sounds[name] = sound;
	std::cout << "Registered sound: " << name << std::endl;
	return true;
}

/**
 * @brief Loads a sound file from disk with optional looping
 */
bool AudioEngine::loadSound(const std::string& name, const std::string& filepath, bool loop) {
	if (!system) {
		std::cout << "It's not initialised" << std::endl;
		return false;
	}
	// check sound exist
	if (sounds.find(name) != sounds.end()) {
		std::cout << "Sound: " << name << " alr loaded" << std::endl;
		return true;
	}
	FMOD::Sound* sound = nullptr;
	FMOD_MODE mode = FMOD_DEFAULT;	// basic audio settings

	if (loop) {
		mode |= FMOD_LOOP_NORMAL;
	}
	else {
		mode |= FMOD_LOOP_OFF;
	}
	// load the audio file
	FMOD_RESULT result = system->createSound(filepath.c_str(), mode, nullptr, &sound);
	if (result != FMOD_OK) {
		checkError(result, "Loading sound " + filepath);
		return false;
	}
	sounds[name] = sound;
	std::cout << "Loaded sound from " << filepath << std::endl;
	return true;
}

/**
 * @brief Unloads a specific sound and removes it from the sound map
 */
void AudioEngine::unloadSound(const std::string& name) {
	auto it = sounds.find(name);
	if (it != sounds.end()) {
		it->second->release();
		sounds.erase(it);
		std::cout << "Unloaded sound: " << name << std::endl;
	}
}

/**
 * @brief Unloads all sounds and clears the sound and channel maps
 */
void AudioEngine::unloadAllSound() {
	for (auto& pair : sounds) {
		pair.second->release();
	}
	sounds.clear();
	channels.clear();
	std::cout << "All sound unloaded" << std::endl;
}

/*  Playback controls  */
/**
 * @brief Plays a loaded sound on a named or unnamed channel
 */
bool AudioEngine::playSound(const std::string& name, const std::string& channelName) {
	if (!system) {
		std::cout << "Audio system not initialised..." << std::endl;
		return false;
	}

	auto soundIt = sounds.find(name);
	if (soundIt == sounds.end()) {
		std::cout << "Sound cant be found: " << name << std::endl;
		return false;
	}

	FMOD::Channel* channel = nullptr;
	// Start paused to true to prevent initial volume click
	FMOD_RESULT result = system->playSound(soundIt->second, nullptr, true, &channel);
	if (result != FMOD_OK) {
		checkError(result, "Sound playing: " + name);
		return false;
	}

	if (!channelName.empty()) {
		channels[channelName] = channel;
	}

	// Set initial volume to 0 before unpausing to prevent click
	if (channel) {
		channel->setVolume(0.0f);
		channel->setPaused(false);  // Now unpause with volume at 0
	}

	std::cout << "Sound playing: " << name << "\n" << std::endl;
	return true;
}

/**
 * @brief Stops playback on a specific channel and removes it from the channel map
 */
void AudioEngine::stopSound(const std::string& channelName) {
	auto it = channels.find(channelName);
	if (it != channels.end() && it->second) {
		bool isPlaying = false;

		it->second->isPlaying(&isPlaying);
		if (isPlaying) {
			it->second->stop();
			std::cout << "Stopped sound on channel: " << channelName << std::endl;
		}
		channels.erase(it);
	}
	else
	{
		std::cout << "[Audio] Channel not found\n";
	}
}

/**
 * @brief Stops playback on all active channels and clears the channel map
 */
void AudioEngine::stopAllSound() {
	for (auto& pair : channels) {
		if (pair.second) {
			bool isPlaying = false;
			pair.second->isPlaying(&isPlaying);
			if (isPlaying) {
				pair.second->stop();
			}
		}
	}
	channels.clear();
	std::cout << "Stopped all sound" << std::endl;
}

/**
 * @brief Pauses or resumes playback on a specific channel
 */
void AudioEngine::pauseSound(const std::string& channelName, bool pause) {
	auto it = channels.find(channelName);
	if (it != channels.end() && it->second) {
		it->second->setPaused(pause);
		std::cout << (pause ? "paused" : "playing") << channelName << std::endl;
	}
}

/**
 * @brief Pauses or resumes all active channels
 */
void AudioEngine::pauseAllSound(bool pause) {
	for (auto& pair : channels) {
		if (pair.second) {
			pair.second->setPaused(pause);
		}
	}
	std::cout << (pause ? "Paused" : "Resumed") << " all sounds" << std::endl;
}

// Volume and effects controls
/**
 * @brief Sets the volume for a specific channel (clamped between 0.0 and 1.0)
 */
void AudioEngine::setVolume(const std::string& channelName, float volume) {
	auto it = channels.find(channelName);
	if (it != channels.end() && it->second) {
		volume = std::max(0.0f, std::min(1.0f, volume));
		it->second->setVolume(volume); // set volume for only that channel
	}
}

/**
 * @brief Sets the master volume for all audio (clamped between 0.0 and 1.0)
 */
void AudioEngine::masterVolume(float volume) {
	if (system) {
		FMOD::ChannelGroup* masterGroup = nullptr;
		system->getMasterChannelGroup(&masterGroup);
		if (masterGroup) {
			volume = std::max(0.0f, std::min(1.0f, volume));
			masterGroup->setVolume(volume); // set volume for all audio
			std::cout << "Set master volume to" << volume << std::endl;
		}
	}
}

/**
 * @brief Toggles mute for all audio (master channel group)
 */
void AudioEngine::masterMute() {
	if (system) {
		FMOD::ChannelGroup* masterGroup = nullptr;
		system->getMasterChannelGroup(&masterGroup);
		if (masterGroup) {
			bool muted = false;
			masterGroup->getMute(&muted);
			masterGroup->setMute(!muted);
			std::cout << "[AudioEngine] All audio " << (muted ? "unmuted" : "muted") << std::endl;
		}
	}
}

/**
 * @brief Checks if a specific channel is currently playing
 */
bool AudioEngine::isPlaying(const std::string& channelName) {
	auto it = channels.find(channelName);
	if (it != channels.end() && it->second) {
		bool isPlaying = false;
		it->second->isPlaying(&isPlaying);
		return isPlaying; // true if playing, false if stopped
	}
	return false;
}

/**
 * @brief Toggles the mute status for a specific channel
 */
void AudioEngine::muteSound(const std::string& channelName) {
	auto it = channels.find(channelName);
	if (it != channels.end() && it->second) {
		bool muted = false;
		it->second->getMute(&muted); // current mute state
		it->second->setMute(!muted); // if muted, unmute it,  if unmuted, mute it
		std::cout << (muted ? "unmuted" : "muted") << "channel" << channelName << std::endl;
	}
}

void AudioEngine::setPitch(const std::string& channelName, float pitch)
{
	auto it = channels.find(channelName);
	if (it != channels.end() && it->second)
	{
		if (pitch < 0.0f)
			pitch = 0.0f;
		it->second->setPitch(pitch);
	}
}

/*  3D Audio / Spatial Sound  */
/**
 * @brief Sets the 3D position of the audio listener (usually the camera/player)
 * @param x X position in world space
 * @param y Y position in world space (use 0 for 2D games)
 */
void AudioEngine::set3DListenerPosition(float x, float y) {
	if (!system) return;

	// FMOD uses a 3D vector, but we're working in 2D so z = 0
	FMOD_VECTOR position = { x, y, 0.0f };
	FMOD_VECTOR velocity = { 0.0f, 0.0f, 0.0f }; // No doppler effect for now
	FMOD_VECTOR forward = { 0.0f, 0.0f, 1.0f };  // Looking forward (into screen)
	FMOD_VECTOR up = { 0.0f, 1.0f, 0.0f };        // Up is Y-axis

	FMOD_RESULT result = system->set3DListenerAttributes(0, &position, &velocity, &forward, &up);
	checkError(result, "set3DListenerPosition");
}

/**
 * @brief Sets the 3D position of a sound source (channel)
 * @param channelName Name of the channel playing the sound
 * @param x X position of the sound source in world space
 * @param y Y position in world space (use 0 for 2D games)
 */
void AudioEngine::set3DSoundPosition(const std::string& channelName, float x, float y) {
	auto it = channels.find(channelName);
	if (it == channels.end() || !it->second) return;

	// Set the 3D position for this channel
	FMOD_VECTOR position = { x, y, 0.0f };
	FMOD_VECTOR velocity = { 0.0f, 0.0f, 0.0f }; // No doppler effect

	FMOD_RESULT result = it->second->set3DAttributes(&position, &velocity);
	checkError(result, "set3DSoundPosition for " + channelName);
}

/**
 * @brief Loads a 3D positional sound with distance attenuation and panning
 * @param name Identifier for the sound
 * @param filepath Path to the audio file
 * @param loop Whether the sound should loop continuously
 * @param minDistance Minimum distance before sound starts attenuating (default: 100.0f)
 * @param maxDistance Maximum distance where sound is silent (default: 1000.0f)
 * @return True if loading was successful
 */
bool AudioEngine::load3DSound(const std::string& name, const std::string& filepath, bool loop, float minDistance, float maxDistance) {
	if (!system) {
		std::cout << "Audio system not initialised" << std::endl;
		return false;
	}

	// Check if sound already exists
	if (sounds.find(name) != sounds.end()) {
		std::cout << "Sound: " << name << " already loaded" << std::endl;
		return true;
	}

	FMOD::Sound* sound = nullptr;
	FMOD_MODE mode = FMOD_3D | FMOD_3D_LINEARROLLOFF; // 3D mode with linear distance attenuation

	if (loop) {
		mode |= FMOD_LOOP_NORMAL;
	}
	else {
		mode |= FMOD_LOOP_OFF;
	}

	// Load the audio file with 3D mode
	FMOD_RESULT result = system->createSound(filepath.c_str(), mode, nullptr, &sound);
	if (result != FMOD_OK) {
		checkError(result, "Loading 3D sound " + filepath);
		return false;
	}

	// Set min and max distance for 3D attenuation
	result = sound->set3DMinMaxDistance(minDistance, maxDistance);
	if (result != FMOD_OK) {
		checkError(result, "Setting 3D distance for " + name);
		sound->release();
		return false;
	}

	sounds[name] = sound;
	std::cout << "Loaded 3D sound: " << name << " (min: " << minDistance << ", max: " << maxDistance << ")" << std::endl;
	return true;
}

/*  DSP EFFECTS  */
/**
 * @brief Enables or disables a low pass filter on a specific channel
 */
bool AudioEngine::setLowPassFilter(const std::string& channelName, bool enable, float cutoffFrequency, float resonance)
{
	if (!system)
	{
		std::cout << "Audio system not initialised" << std::endl;
		return false;
	}

	auto channelIt = channels.find(channelName);
	if (channelIt == channels.end() || !channelIt->second)
	{
		std::cout << "Channel not found: " << channelName << std::endl;
		return false;
	}

	FMOD::Channel* channel = channelIt->second;

	if (enable)
	{
		// clamp parameters to safe ranges
		cutoffFrequency = std::max(10.0f, std::min(22000.0f, cutoffFrequency));
		resonance = std::max(1.0f, std::min(10.0f, resonance));

		FMOD::DSP* lowPass = nullptr;
		FMOD_RESULT result;

		// check if a filter already exists for this channel
		auto filterIt = lowPassFilters.find(channelName);
		if (filterIt != lowPassFilters.end() && filterIt->second)
		{
			// reuse existing filter if have
			lowPass = filterIt->second;
			lowPass->setParameterFloat(FMOD_DSP_LOWPASS_CUTOFF, cutoffFrequency);
			lowPass->setParameterFloat(FMOD_DSP_LOWPASS_RESONANCE, resonance);
			lowPass->setBypass(false);
		}
		else
		{
			// create a new low pass filter if dh
			result = system->createDSPByType(FMOD_DSP_TYPE_LOWPASS, &lowPass);
			if (result != FMOD_OK)
			{
				checkError(result, "Creating low pass filter for " + channelName);
				return false;
			}

			// set the parameters
			lowPass->setParameterFloat(FMOD_DSP_LOWPASS_CUTOFF, cutoffFrequency);
			lowPass->setParameterFloat(FMOD_DSP_LOWPASS_RESONANCE, resonance);

			// add dsp to channel
			result = channel->addDSP(0, lowPass);
			if (result != FMOD_OK)
			{
				checkError(result, "Adding low pass filter to channel " + channelName);
				lowPass->release();
				return false;
			}

			lowPassFilters[channelName] = lowPass;
		}

		std::cout << "Low pass filter enabled on " << channelName
			<< " (cutoff: " << cutoffFrequency << " Hz, resonance: " << resonance << ")" << std::endl;
	}
	else
	{
		// disable filter
		auto filterIt = lowPassFilters.find(channelName);
		if (filterIt != lowPassFilters.end() && filterIt->second)
		{
			filterIt->second->setBypass(true);
			std::cout << "Low pass filter disabled on " << channelName << std::endl;
		}
	}

	return true;
}

/**
 * @brief Updates the cutoff frequency of an existing low pass filter
 */
bool AudioEngine::updateLowPassFilterFrequency(const std::string& channelName, float cutoffFrequency)
{
	auto filterIt = lowPassFilters.find(channelName);
	if (filterIt == lowPassFilters.end() || !filterIt->second)
	{
		std::cout << "No low pass filter found on channel: " << channelName << std::endl;
		return false;
	}

	cutoffFrequency = std::max(10.0f, std::min(22000.0f, cutoffFrequency));

	FMOD_RESULT result = filterIt->second->setParameterFloat(FMOD_DSP_LOWPASS_CUTOFF, cutoffFrequency);
	if (result != FMOD_OK)
	{
		checkError(result, "Updating low pass filter frequency for " + channelName);
		return false;
	}

	return true;
}

/**
 * @brief Enables or disables a low pass filter on the master channel group
 */
bool AudioEngine::setMasterLowPassFilter(bool enable, float cutoffFrequency, float resonance)
{
	if (!system)
	{
		std::cout << "Audio system not initialised" << std::endl;
		return false;
	}

	FMOD::ChannelGroup* masterGroup = nullptr;
	FMOD_RESULT result = system->getMasterChannelGroup(&masterGroup);
	if (result != FMOD_OK || !masterGroup)
	{
		checkError(result, "Getting MASTER channel group");
		return false;
	}

	if (enable)
	{
		cutoffFrequency = std::max(10.0f, std::min(22000.0f, cutoffFrequency));
		resonance = std::max(1.0f, std::min(10.0f, resonance));

		if (masterLowPassFilter)
		{
			masterLowPassFilter->setParameterFloat(FMOD_DSP_LOWPASS_CUTOFF, cutoffFrequency);
			masterLowPassFilter->setParameterFloat(FMOD_DSP_LOWPASS_RESONANCE, resonance);
			masterLowPassFilter->setBypass(false);
		}
		else
		{
			result = system->createDSPByType(FMOD_DSP_TYPE_LOWPASS, &masterLowPassFilter);
			if (result != FMOD_OK)
			{
				checkError(result, "Creating MASTER low pass filter");
				return false;
			}

			masterLowPassFilter->setParameterFloat(FMOD_DSP_LOWPASS_CUTOFF, cutoffFrequency);
			masterLowPassFilter->setParameterFloat(FMOD_DSP_LOWPASS_RESONANCE, resonance);

			result = masterGroup->addDSP(0, masterLowPassFilter);
			if (result != FMOD_OK)
			{
				checkError(result, "Adding low pass filter to MASTER channel group");
				masterLowPassFilter->release();
				masterLowPassFilter = nullptr;
				return false;
			}
		}

		//std::cout << "Master low pass filter enabled (cutoff: " << cutoffFrequency
		//	<< " Hz, resonance: " << resonance << ")" << std::endl;
	}
	else
	{
		if (masterLowPassFilter)
		{
			masterLowPassFilter->setBypass(true);
			//std::cout << "Master low pass filter disabled" << std::endl;
		}
	}

	return true;
}

/**
 * @brief Updates the cutoff frequency of the master low pass filter
 */
bool AudioEngine::updateMasterLowPassFilterFrequency(float cutoffFrequency)
{
	if (!masterLowPassFilter)
	{
		std::cout << "No master low pass filter active" << std::endl;
		return false;
	}

	cutoffFrequency = std::max(10.0f, std::min(22000.0f, cutoffFrequency));

	FMOD_RESULT result = masterLowPassFilter->setParameterFloat(FMOD_DSP_LOWPASS_CUTOFF, cutoffFrequency);
	if (result != FMOD_OK)
	{
		checkError(result, "Updating MASTER low pass filter frequency");
		return false;
	}

	return true;
}

/*  Error Handling  */
/**
 * @brief Checks FMOD result and logs error message if operation failed
 */
void AudioEngine::checkError(FMOD_RESULT result, const std::string& operation) {
	if (result != FMOD_OK) {
		std::cout << "fmod error in " << operation << ": " << FMOD_ErrorString(result) << result << std::endl;
	}
}
