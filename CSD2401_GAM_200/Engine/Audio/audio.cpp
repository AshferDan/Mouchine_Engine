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

	result = system->init(32, FMOD_INIT_NORMAL, nullptr);
	if (result != FMOD_OK) {
		checkError(result, "System init");
		return false;
	}

	std::cout << "audio initialised sucessfully" << std::endl;
	return true;
}

void AudioEngine::updateAudio() {
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
		std::cout << "sound: " << name << " loaded" << std::endl;
		return true;
	}
	FMOD::Sound* sound = nullptr;
	FMOD_MODE mode = FMOD_DEFAULT;

	if (loop) {
		mode |= FMOD_LOOP_NORMAL;
	}
	else {
		mode |= FMOD_LOOP_OFF;
	}
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

void AudioEngine::stopSound(const std::string& channelName) {}

void AudioEngine::stopAllSound() {}

bool AudioEngine::eventPlaying(const std::string& eventName) {}

/*  Volume and effects controls  */

void AudioEngine::setVolume(const std::string& channelName, float volume) {}

void AudioEngine::updateVolume(float volume) {}

void AudioEngine::isPlaying(const std::string& channelName) {}

void AudioEngine::muteSound(float volume) {}

/*  Error Handling  */

void AudioEngine::checkError(FMOD_RESULT result, const std::string& operation) {
	if (result != FMOD_OK) {
		std::cout << "fmod error in " << operation << ": " << FMOD_ErrorString(result) << result << std::endl;
	}
}

