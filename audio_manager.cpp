#include"audio_manager.h"
#include<iostream>
AudioManager* AudioManager::instance = nullptr;
AudioManager::AudioManager():volume(70.0f),enabled(true){}
AudioManager&AudioManager::getInstance() {
	if (!instance) {
		instance = new AudioManager();
	}
	return*instance;
}
bool AudioManager::loadSound(const std::string& name, const std::string& filename) {
	sf::SoundBuffer buffer;
	if(!buffer.loadFromFile(filename)){
		std::cerr << "Failed to load sound:" << filename << std::endl;
		return false;
	}
	buffers[name] = buffer;
	sf::Sound sound;
	sound.setBuffer(buffers[name]);
	sound.setVolume(volume);
	sounds[name] = sound;
	return true;
}
void AudioManager::playSound(const std::string& name) {
	if (!enabled)return;
	auto it = sounds.find(name);
	if (it != sounds.end()) {
		it->second.stop();
		it->second.play();
	}
}
void AudioManager::setVolume(float vol) {
	volume = vol;
	for (auto& pair : sounds) {
		pair.second.setVolume(volume);
	}
}
void AudioManager::stopAll() {
	for (auto& pair : sounds) {
		pair.second.stop();
	}
}
void AudioManager::cleanup() {
	stopAll();
	buffers.clear();
	sounds.clear();
}

// New setter to match header (keeps backward compatibility if called elsewhere)
void AudioManager::setEnabled(bool enable) {
	enabled = enable;
	if (!enabled) stopAll();
}