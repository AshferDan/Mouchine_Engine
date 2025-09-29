#include "audio.h"

AudioEngine::AudioEngine() : sounds(), channels(), system(nullptr){}
AudioEngine::~AudioEngine() { stopAudio(); }

/*  Core Functionalities  */
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

	std::cout << "audio initialised sucessfully" << std::endl;
	return true;
}

void AudioEngine::updateAudio() {
	// must be called every frame in game loop
	if (system) {
		system->update();
	}
}

void AudioEngine::stopAudio() {
	stopAllSound();
	unloadAllSound();

	if (system) {
		system->close();
		system->release();
		system = nullptr;
	}

	std::cout << "audio stopped successfully" << std::endl;
}

/*   Sound loading  */
bool AudioEngine::loadSound(const std::string& name, const std::string& filepath, bool loop) {
	if (!system) {
		std::cout << "o man its not initialised" << std::endl;
		return false;
	}
	// check sound exist
	if (sounds.find(name) != sounds.end()) {
		std::cout << "sound: " << name << " alr loaded" << std::endl;
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
		checkError(result, "loading sound " + filepath);
		return false;
	}
	sounds[name] = sound;
	std::cout << "loaded sound from " << filepath << std::endl;
	return true;
}

void AudioEngine::unloadSound(const std::string& name) {
	auto it = sounds.find(name);
	if (it != sounds.end()) {
		it->second->release();
		sounds.erase(it);
		std::cout << "unloaded sound: " << name << std::endl;
	}
}

void AudioEngine::unloadAllSound() {
	for (auto& pair : sounds) {
		pair.second->release();
	}
	sounds.clear();
	channels.clear();
	std::cout << "all sounds unloaded: " << std::endl;
}

/*  Playback controls  */

bool AudioEngine::playSound(const std::string& name, const std::string& channelName = "") {
	if (!system) {
		std::cout << "audio system not initialised..." << std::endl;
		return false;
	}

	auto soundIt = sounds.find(name);
	if (soundIt == sounds.end()) {
		std::cout << "sound cant be found: " << name << std::endl;
		return false;
	}

	FMOD::Channel* channel = nullptr;
	FMOD_RESULT result = system->playSound(soundIt->second, nullptr, false, &channel);
	if (result != FMOD_OK) {
		checkError(result, "playing sound: " + name);
		return false;
	}
	
	if (!channelName.empty()) {
		channels[channelName] = channel;
	}

	std::cout << "playing soundd: " << name << std::endl;
	return true;
}

void AudioEngine::stopSound(const std::string& channelName) {
	auto it = channels.find(channelName);
	if (it != channels.end() && it->second) {
		bool isPlaying = false;

		it->second->isPlaying(&isPlaying);
		if (isPlaying) {
			it->second->stop();
			std::cout << "stopped sound on channle: " << channelName << std::endl;
		}
		channels.erase(it);
	}
}

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
	std::cout << "stopped all sounds" << std::endl;
}

bool AudioEngine::eventPlaying(const std::string& eventName) {
	// to be implemented when theres 'events'
	return isPlaying(eventName);
}

/*  Volume and effects controls  */

void AudioEngine::setVolume(const std::string& channelName, float volume) {
	auto it = channels.find(channelName);
	if (it != channels.end() && it->second) {
		volume = std::max(0.0f, std::min(1.0f, volume));
		it->second->setVolume(volume); // set volume for only that channel
	}
}

void AudioEngine::masterVolume(float volume) {
	if (system) {
		FMOD::ChannelGroup* masterGroup = nullptr;
		system->getMasterChannelGroup(&masterGroup);
		if (masterGroup) {
			volume = std::max(0.0f, std::min(1.0f, volume));
			masterGroup->setVolume(volume); // set volume for all audio
			std::cout << "set master volume to" << volume << std::endl;
		}
	}
}

bool AudioEngine::isPlaying(const std::string& channelName) {
	auto it = channels.find(channelName);
	if (it != channels.end() && it->second) {
		bool isPlaying = false;
		it->second->isPlaying(&isPlaying);
		return isPlaying; // true if playing, false if stopped
	}
	return false;
}

void AudioEngine::muteSound(const std::string& channelName) {
	auto it = channels.find(channelName);
	if (it != channels.end() && it->second) {
		bool muted = false;
		it->second->getMute(&muted); // current mute state
		it->second->setMute(!muted); // if muted, unmute it,  if unmuted, mute it
		std::cout << (muted ? "unmuted" : "muted") << "channel" << channelName << std::endl;
	}
}

/*  Error Handling  */

void AudioEngine::checkError(FMOD_RESULT result, const std::string& operation) {
	if (result != FMOD_OK) {
		std::cout << "fmod error in " << operation << ": " << FMOD_ErrorString(result) << result << std::endl;
	}
}

