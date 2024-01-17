#pragma once
#include "BaseParticleSystem.h"
#include "Component.h"
#include "glm.hpp"

class Texture;
class ParticleSystem;

class ComponentParticleSystem : public Component
{
public:
	ComponentParticleSystem(Object* parent);
	~ComponentParticleSystem();

	void Update(float delta) override;
	void Draw(mat4 pv, vec3 position, DrawMode mode) override;

	void DrawGUI() override;

	void SaveToDisk(const string& filename);
	void LoadFromDisk(const string& filename);
	string name = "newParticleSystem";
	string path = "crawler/particle/";

	bool IsEnabled() const { return particleSystem != nullptr && particleSystem->isEnabled; }
	bool IsPlaying() const { return particleSystem != nullptr && particleSystem->isPlaying; }
	bool IsInitialised() const { return particleSystem != nullptr && particleSystem->isInitialised; }
	bool IsConfigured() const { return particleSystem != nullptr; }

    ParticleSystem* particleSystem = nullptr;

    bool m_debugDrawBounds = false;
};