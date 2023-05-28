#pragma once
#include "Component.h"
#include <string>
#include "soloud.h"
#include "soloud_audiosource.h"

class ComponentAudioSource : public Component
{
public:
	ComponentAudioSource(Object* parent);
	ComponentAudioSource(Object* parent, std::istream& istream);

	~ComponentAudioSource();
	ComponentAudioSource(ComponentAudioSource& other) = delete;
	const ComponentAudioSource& operator=(const ComponentAudioSource& other) = delete;

	void Update(float deltatime) override;
	void DrawGUI() override;

	void Write(std::ostream& ostream) override;

	void OnParentChange() override;

	Component* Clone(Object* parent);
protected:
	bool m_isStream = false;
	bool m_isLooping = false;
	string m_audioSourceName = "";

	SoLoud::handle m_handle;
	float m_timeSincePlayed = 0.0f;

	void Play();
	void Stop();
};

