#include "AudioManager.h"
#include "LogUtils.h"
#include "Input.h"
#include "imgui.h"
#include <filesystem>

using std::vector;
namespace fs = std::filesystem;

AudioManager::AudioManager()
{
	gSoloud.init();
}

AudioManager::~AudioManager()
{
	gSoloud.deinit();
	
	for (auto material : m_loaded)
		delete material.second;

	for (auto material : m_stream)
		delete material.second;
}

void AudioManager::Init()
{
	if (!s_instance) s_instance = new AudioManager();
	else LogUtils::Log("Tried to Init AudioManager when it was already initialised");
}

void AudioManager::Update(float delta)
{
	if (m_audioListener != nullptr)
	{
		gSoloud.set3dListenerParameters(
			m_audioListener->position.x,
			m_audioListener->position.y,
			m_audioListener->position.z,
			m_audioListener->forward.x,
			m_audioListener->forward.y,
			m_audioListener->forward.z,
			0,0,1); // Up is here

		gSoloud.update3dAudio(); // This tells all current playing sounds to respect the above update.
	}

	// Adjust volumes - pretty hacky but just something while in dev.
	if (Input::Keyboard(GLFW_KEY_KP_SUBTRACT).Down() && globalVolume > 0.0f)
		globalVolume -= 0.1f;
	if (Input::Keyboard(GLFW_KEY_KP_ADD).Down() && globalVolume < 1.0f)
		globalVolume += 0.1f;


	if (globalVolume != globalVolumeOld)
	{
		if (globalVolume < 0)
			globalVolume = 0;

		LogUtils::Log("Setting Volume: " + std::to_string(globalVolume).substr(0,3));
		globalVolumeOld = globalVolume;
		gSoloud.setGlobalVolume(globalVolume);
	}

	ProcessQueue(delta);
}

void AudioManager::DrawGUI()
{
	//ImGui::SetNextWindowPos({ 800, 0 }, ImGuiCond_FirstUseEver);
	//ImGui::SetNextWindowSize({ 400, 900 }, ImGuiCond_FirstUseEver);
	//ImGui::SetNextWindowCollapsed(true, ImGuiCond_FirstUseEver);
	ImGui::Begin("Audio", nullptr/*, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize*/);

	ImGui::BeginDisabled();
	float soLoudVolume = s_instance->gSoloud.getGlobalVolume();
	ImGui::InputFloat("Global Volume", &soLoudVolume);
	int loadCount = (int)s_instance->m_loaded.size();
	ImGui::DragInt("Loaded Count", &loadCount);
	ImGui::DragFloat3("Audio Listener Position", (float*)&s_instance->m_audioListener->position);
	ImGui::DragFloat3("Audio Listener Forward", (float*)&s_instance->m_audioListener->forward);

	ImGui::EndDisabled();
	ImGui::DragFloat3("Test Audio Source Position", (float*)&s_instance->test3Dpos);
	for (auto m : s_instance->m_loaded)
	{
		if (ImGui::Button(m.first.c_str()))
			AudioManager::PlaySound(m.first);
		ImGui::PushID(m.first.c_str());
		ImGui::SameLine();
		if (ImGui::Button("3D"))
		{
			AudioManager::PlaySound(m.first,s_instance->test3Dpos);
		}
		ImGui::PopID();
	}

	ImGui::BeginDisabled();
	int streamCount = (int)s_instance->m_stream.size();
	ImGui::DragInt("Stream Count", &streamCount);
	ImGui::EndDisabled();
	
	if (ImGui::Button("Play"))
		AudioManager::StartMusic();
	ImGui::SameLine();
	if (ImGui::Button("Stop"))
		AudioManager::StopMusic();
	ImGui::SameLine();
	if (ImGui::Button(s_instance->gSoloud.getPause(s_instance->m_currentTrack) ? "Unpause" : "Pause"))
		(s_instance->gSoloud.getPause(s_instance->m_currentTrack) ? AudioManager::UnPauseMusic : AudioManager::PauseMusic)();

	for (auto m : s_instance->m_stream)
	{
		if (ImGui::Button(m.first.c_str()))
			AudioManager::ChangeMusic(m.first);
	}
	ImGui::End();
}

void AudioManager::StartMusic()
{
	s_instance->gSoloud.fadeVolume(s_instance->m_currentTrack, 1.0f, .5f);
}

void AudioManager::ChangeMusic(string name, bool loop)
{
	s_instance->gSoloud.fadeVolume(s_instance->m_currentTrack, 0.0f, 1.0f);
	s_instance->m_currentTrack = s_instance->gSoloud.play(*s_instance->m_stream[name]);
	s_instance->gSoloud.setLooping(s_instance->m_currentTrack, loop);
}

void AudioManager::StopMusic()
{
	s_instance->gSoloud.fadeVolume(s_instance->m_currentTrack, 0.0f, .5f);
}

void AudioManager::PauseMusic()
{
	s_instance->gSoloud.setPause(s_instance->m_currentTrack, true);
}

void AudioManager::UnPauseMusic()
{
	s_instance->gSoloud.setPause(s_instance->m_currentTrack, false);
}

SoLoud::handle AudioManager::PlaySound(string soundname, bool loop, float volume)
{
	s_instance->m_loaded[soundname]->setLooping(loop);
	return s_instance->gSoloud.play(*s_instance->m_loaded[soundname], volume);
}

SoLoud::handle AudioManager::PlaySound(string soundname, glm::vec3 position3D, bool loop)
{
	SoLoud::handle handle = s_instance->gSoloud.play3d(
		*s_instance->m_loaded[soundname],
		position3D.x,
		position3D.y,
		position3D.z);
	if (loop) s_instance->gSoloud.setLooping(handle, true);
	return handle;
}

SoLoud::handle AudioManager::PlayStream(string streamname, bool loop)
{
	s_instance->m_stream[streamname]->setLooping(loop);
	return s_instance->gSoloud.play(*s_instance->m_stream[streamname]);
}

SoLoud::handle AudioManager::PlayStream(string streamname, glm::vec3 position3D, bool loop)
{
	s_instance->m_stream[streamname]->setLooping(loop);
	return s_instance->gSoloud.play3d(
		*s_instance->m_stream[streamname],
		position3D.x,
		position3D.y,
		position3D.z);
}

void AudioManager::Stop(SoLoud::handle handle)
{
	s_instance->gSoloud.stop(handle);
}

void AudioManager::SetAudioListener(AudioListener* listener)
{
	s_instance->m_audioListener = listener;
}

void AudioManager::Set3dSourcePosition(SoLoud::handle handle, vec3 position)
{
	s_instance->gSoloud.set3dSourcePosition(handle, position.x, position.y, position.z);
}

void AudioManager::Set3dSourceMinMaxDistance(SoLoud::handle handle, float min, float max)
{
	s_instance->gSoloud.set3dSourceMinMaxDistance(handle, min, max);
}

void AudioManager::PushSound(string soundname, float time, glm::vec3 position3D)
{
	AudioStep step;
	step.load = soundname;
	step.time = time;
	step.position = position3D;
	s_instance->m_stepQueue.push(step);
	if(s_instance->m_stepQueue.size() == 1) s_instance->PlayFirstInQueue();
}

void AudioManager::EmptyQueue()
{
	while (!s_instance->m_stepQueue.empty())
		s_instance->m_stepQueue.pop();
}

void AudioManager::SetAudioSourceAttentuation(string name, unsigned int attentuationModel, float attentionationRollOffFactor)
{
	SoLoud::AudioSource* sound = s_instance->m_loaded.at(name);
	if (!sound) return;

	sound->set3dAttenuation(attentuationModel, attentionationRollOffFactor);
}

void AudioManager::SetAudioSourceMinMaxDistance(string name, float minDistance, float maxDistance, bool shouldTickIfInAudible)
{
	SoLoud::AudioSource* sound = s_instance->m_loaded.at(name);
	if (!sound) return;

	sound->setInaudibleBehavior(shouldTickIfInAudible, true);
	sound->set3dMinMaxDistance(minDistance, maxDistance);
}

void AudioManager::LoadFromFile(const char* filename)
{
	Wav* sound = new Wav();
	sound->load(filename);
	m_loaded.emplace(filename, sound);
}

void AudioManager::StreamFromFile(const char* filename)
{
	WavStream* sound = new WavStream();
	sound->load(filename);
	m_stream.emplace(filename, sound);
}

void AudioManager::PlayFirstInQueue()
{
	AudioStep& step = s_instance->m_stepQueue.front();
	PlaySound(step.load, step.position);
}

void AudioManager::ProcessQueue(float delta)
{
	if (s_instance->m_stepQueue.size() > 0)
	{
		s_instance->m_stepQueue.front().time -= delta;
		if (s_instance->m_stepQueue.front().time <= 0)
		{
			s_instance->m_stepQueue.pop();
			if (s_instance->m_stepQueue.size() > 0) PlayFirstInQueue();
		}
	}
}

void AudioManager::LoadAllFiles(string folder)
{
	LogUtils::Log("Loading Audio Files");
	for (auto d : fs::recursive_directory_iterator(folder + "/sound/load"))
	{
		if (d.path().extension() == ".wav" || d.path().extension() == ".ogg")
		{
			string output = "Loading: " + d.path().generic_string();
			LogUtils::Log(output.c_str());
			s_instance->LoadFromFile(d.path().generic_string().c_str());
		}
	}

	for (auto d : fs::recursive_directory_iterator(folder + "/sound/stream"))
	{
		if (d.path().extension() == ".wav" || d.path().extension() == ".mp3" || d.path().extension() == ".ogg")
		{
			string output = "Prepping for streaming: " + d.path().generic_string();
			LogUtils::Log(output.c_str());
			s_instance->StreamFromFile(d.path().generic_string().c_str());
		}
	}
}

AudioManager* AudioManager::s_instance = nullptr;