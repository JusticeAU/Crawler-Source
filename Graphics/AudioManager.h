#pragma once
#include "AudioListener.h"
#include <string>
#include <unordered_map>

#include "soloud.h"
#include "soloud_wav.h"
#include "soloud_wavstream.h"
#include "soloud_audiosource.h"

#include "glm.hpp"

using std::string;
using std::unordered_map;

using SoLoud::Soloud;
using SoLoud::Wav;
using SoLoud::WavStream;

class AudioManager
{
public:
	~AudioManager();
	AudioManager(AudioManager const& other) = delete;
	AudioManager& operator=(const AudioManager& other) = delete;

	static void Init();

	void Update();

	static void DrawGUI();
	static AudioManager* s_instance;

	static void StartMusic();
	static void ChangeMusic(string name);
	static void StopMusic();

	static void PauseMusic();
	static void UnPauseMusic();

	static void PlaySound(string soundname);
	static void PlaySound(string soundname, glm::vec3 position3D);
	static void SetAudioListener(AudioListener* listener);
protected:
	AudioManager();

	Soloud gSoloud;
	unordered_map<string, Wav*> m_loaded;
	unordered_map<string, WavStream*> m_stream;

	SoLoud::handle m_currentTrack;

	glm::vec3 test3Dpos;

	AudioListener* m_audioListener = nullptr;

	void LoadFromFile(const char* filename);
	void StreamFromFile(const char* filename);
	void LoadAllFiles();
};

