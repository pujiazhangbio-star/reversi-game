#ifndef AUDIO_MANAGER_H
#define AUDIO_MANAGER_H
#include<SFML/Audio.hpp>
#include<map>
#include<string>
class AudioManager {
private:
	std::map<std::string, sf::SoundBuffer>buffers;
	std::map<std::string, sf::Sound>sounds;
	float volume;
	bool enabled;
	AudioManager();
	static AudioManager* instance;
public:
	static AudioManager& getInstance();
	AudioManager(const AudioManager&) = delete;
	AudioManager& operator=(const AudioManager&) = delete;
	bool loadSound(const std::string& name, const std::string& filename);
	void playSound(const std::string& name);
	void setVolume(float volume);
	float getVolume()const { return volume; }
	// Enable or disable audio playback
	void setEnabled(bool enable);
	bool isEnabled()const { return enabled; }
	void stopAll();
	void cleanup();
};
#endif


