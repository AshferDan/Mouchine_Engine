#ifndef AUDIO_H
#define AUDIO_H

#include "fmod.hpp"
#include "fmod_studio.hpp"
#include "fmod_errors.h"
#include "System.h"

#include <iostream>
#include <string>
#include <map>
#include <vector>
#include <list>

class AudioEngine {
private:
	FMOD::System* system;
	std::map<std::string, FMOD::Sound*> sounds;
	std::map<std::string, FMOD::Channel*> channels;

public:
	AudioEngine();
	~AudioEngine();

	// core functionalities
	bool initialiseAudio();
	void updateAudio();
	void stopAudio();

	// sound loading
	bool registerSound(const std::string& name, FMOD::Sound* sound);
	bool loadSound(const std::string& name, const std::string& filepath, bool loop = false);
	void unloadSound(const std::string& name);
	void unloadAllSound();

	// playback
	bool playSound(const std::string& name, const std::string& channelName = "");
	void stopSound(const std::string& channelName);
	void stopAllSound();
	//bool eventPlaying(const std::string& eventName);
	//void stopEvent (const string& eventName);

	// volume and effects
	void setVolume(const std::string& channelName, float volume);
	void masterVolume(float volume);
	bool isPlaying(const std::string& channelName);
	void muteSound(const std::string& channelName);

	// error handling
	void checkError(FMOD_RESULT result, const std::string& operation);
	FMOD::System* GetFMODSystem() const { return system; }

};


#endif // AUDIO_H
