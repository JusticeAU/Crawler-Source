#pragma once
#include "AudioListener.h"
#include "AudioStep.h"
#include <string>
#include <unordered_map>
#include <queue>

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
	static void LoadAllFiles(string folder);

	void Update(float delta);

	static void DrawGUI();
	static AudioManager* s_instance;

	static const unordered_map<string, Wav*>* Loaded() { return &s_instance->m_loaded; }
	static const unordered_map<string, WavStream*>* Streams() { return &s_instance->m_stream; }

	static void StartMusic();
	static void ChangeMusic(string name);
	static void StopMusic();

	static void PauseMusic();
	static void UnPauseMusic();

	static SoLoud::handle PlaySound(string soundname, bool loop = false);
	static SoLoud::handle PlaySound(string soundname, glm::vec3 position3D, bool loop = false);
	static SoLoud::handle PlayStream(string streamname, bool loop = false);
	static SoLoud::handle PlayStream(string streamname, glm::vec3 position3D, bool loop = false);

	static void Stop(SoLoud::handle handle);
	static void SetAudioListener(AudioListener* listener);
	static void Set3dSourcePosition(SoLoud::handle handle, vec3 position);
	static void Set3dSourceMinMaxDistance(SoLoud::handle handle, float min, float max);
	
	static void PushSound(string soundname, float time, glm::vec3 position3D);
	static void EmptyQueue();

	static void SetAudioSourceAttentuation(string name, unsigned int attentuationModel, float attentionationRollOffFactor);
	static void SetAudioSourceMinMaxDistance(string name, float minDistance, float maxDistance);


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

	float globalVolume = 1.0f;
	// For seperate volume sliders such as ambience, sfx, music etc. We would use SoLoud mixing buses.
	float globalVolumeOld = 1.0f;

	std::queue<AudioStep> m_stepQueue;
	void ProcessQueue(float delta);
	void PlayFirstInQueue();
};

