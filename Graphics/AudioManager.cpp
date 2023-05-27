#include "AudioManager.h"
#include "LogUtils.h"
#include "imgui.h"
#include <filesystem>

using std::vector;
namespace fs = std::filesystem;

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

void AudioManager::DrawGUI()
{
	//ImGui::SetNextWindowPos({ 800, 0 }, ImGuiCond_FirstUseEver);
	//ImGui::SetNextWindowSize({ 400, 900 }, ImGuiCond_FirstUseEver);
	//ImGui::SetNextWindowCollapsed(true, ImGuiCond_FirstUseEver);
	ImGui::Begin("Audio", nullptr/*, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize*/);

	ImGui::BeginDisabled();
	int loadCount = (int)s_instance->m_loaded.size();
	ImGui::DragInt("Loaded Count", &loadCount);
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

void AudioManager::ChangeMusic(string name)
{
	s_instance->gSoloud.fadeVolume(s_instance->m_currentTrack, 0.0f, 1.0f);
	s_instance->m_currentTrack = s_instance->gSoloud.play(*s_instance->m_stream[name]);
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

void AudioManager::PlaySound(string soundname)
{
	s_instance->gSoloud.play(*s_instance->m_loaded[soundname]);
}

void AudioManager::PlaySound(string soundname, glm::vec3 position3D)
{
	s_instance->gSoloud.play3d(
		*s_instance->m_loaded[soundname],
		position3D.x,
		position3D.y,
		position3D.z);
}

void AudioManager::Set3DListener(glm::vec3 position, glm::vec3 lookingAt)
{
	s_instance->gSoloud.set3dListenerPosition(position.x, position.y, position.z);
	s_instance->gSoloud.set3dListenerAt(lookingAt.x, lookingAt.y, lookingAt.z);

}

AudioManager::AudioManager()
{
	gSoloud.init();
	LoadAllFiles();
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

void AudioManager::LoadAllFiles()
{
	LogUtils::Log("Loading Audio Files");
	for (auto d : fs::recursive_directory_iterator("audio/load"))
	{
		if (d.path().extension() == ".wav")
		{
			string output = "Loading: " + d.path().generic_string();
			LogUtils::Log(output.c_str());
			LoadFromFile(d.path().generic_string().c_str());
		}
	}

	for (auto d : fs::recursive_directory_iterator("audio/stream"))
	{
		if (d.path().extension() == ".wav" || d.path().extension() == ".mp3")
		{
			string output = "Prepping for streamin: " + d.path().generic_string();
			LogUtils::Log(output.c_str());
			StreamFromFile(d.path().generic_string().c_str());
		}
	}
}

AudioManager* AudioManager::s_instance = nullptr;