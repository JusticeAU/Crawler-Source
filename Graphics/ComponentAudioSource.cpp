#include "ComponentAudioSource.h"
#include "Object.h"
#include "AudioManager.h"

using std::to_string;

ComponentAudioSource::ComponentAudioSource(Object* parent) : Component("Audio Source", Component_AudioSource, parent)
{
}

ComponentAudioSource::ComponentAudioSource(Object* parent, std::istream& istream) : Component("Audio Source", Component_AudioSource, parent)
{
}

ComponentAudioSource::~ComponentAudioSource()
{
}

void ComponentAudioSource::Update(float deltatime)
{
	AudioManager::Set3dSourcePosition(m_handle, componentParent->GetWorldSpacePosition());
}

void ComponentAudioSource::DrawGUI()
{
	if (ImGui::Button("Play"))
		Play();
	ImGui::SameLine();
	if (ImGui::Button("Stop"))
		Stop();

	ImGui::Checkbox("Is Looping", &m_isLooping);
	ImGui::SameLine();
	if(ImGui::Checkbox("Is Stream", &m_isStream))
		m_audioSourceName = "";

	string audioSourceNameStr = "Audio Source##" + to_string(componentParent->id);
	if (ImGui::BeginCombo(audioSourceNameStr.c_str(), m_audioSourceName.c_str()))
	{
		if (!m_isStream)
		{
			for (auto a : *AudioManager::Loaded())
			{
				const bool is_selected = (a.first == m_audioSourceName);
				if (ImGui::Selectable(a.first.c_str(), is_selected))
					m_audioSourceName = a.first;

				// Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
				if (is_selected)
					ImGui::SetItemDefaultFocus();
			}
		}
		else
		{
			for (auto a : *AudioManager::Streams())
			{
				const bool is_selected = (a.first == m_audioSourceName);
				if (ImGui::Selectable(a.first.c_str(), is_selected))
					m_audioSourceName = a.first;

				// Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
				if (is_selected)
					ImGui::SetItemDefaultFocus();
			}
		}
		ImGui::EndCombo();
	}
}

void ComponentAudioSource::Write(std::ostream& ostream)
{
}

void ComponentAudioSource::OnParentChange()
{
}

Component* ComponentAudioSource::Clone(Object* parent)
{
    return nullptr;
}

void ComponentAudioSource::Play()
{
	if (m_audioSourceName != "")
	{
		if (m_isStream)
			m_handle = AudioManager::PlayStream(m_audioSourceName, componentParent->GetWorldSpacePosition(), m_isLooping);
		else
			m_handle = AudioManager::PlaySound(m_audioSourceName, componentParent->GetWorldSpacePosition(), m_isLooping);
	}
}

void ComponentAudioSource::Stop()
{
	AudioManager::Stop(m_handle);
}
