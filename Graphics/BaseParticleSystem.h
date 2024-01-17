#pragma once
#include "Graphics.h"
#include "serialisation.h"

class ShaderProgram;
class Texture;

class ParticleSystem
{
public:
	virtual ~ParticleSystem();

	void Update(float delta);
	virtual void DrawGUI() {};

	virtual void UpdateParticles(float delta) = 0;
	virtual void Draw() = 0;

	virtual void Initialise() = 0;

	virtual void SetShaderUniforms() {};
	virtual void BindDrawTextures() {};
	virtual void UnBindDrawTextures() {};

	static void InitialiseSharedResources();
	static inline bool isSharedResourcesInitialised = false;
	static inline Texture* randomVec3s1D = nullptr;

	void Play() { isPlaying = true; }
	void Pause() { isPlaying = false; }
	//void Stop();
	//void Reset();
	//void Prime();
	void SetEnabled(bool enabled = true) { isEnabled = enabled; }

	void DrawBuffers() const;
	virtual void SetVertexAttribPointers() const;
	virtual void ClearVertexAttribPointers() const;
	void SwapBuffers();
	void DeleteBuffers();
	void GetShaders();
	virtual void InitialiseTransformFeedbackVaryings(GLuint shaderProgramID) = 0;

	// Serialised Data (all systems)
	std::string updateShaderString = "";
	std::string drawShaderString = "engine/shader/Billboard";


	// Internal data
	unsigned int type = 0;
	bool isInitialised = false;
	bool isFirst = true;
	bool isEnabled = false;
	bool isPlaying = false;

	glm::mat4 transform = glm::mat4(1);
	float timeScale = 1.0f;

	ShaderProgram* updateShader = nullptr;
	ShaderProgram* drawShader = nullptr;

	unsigned int particleStructSize = 0;
	unsigned int particleStructPositionOffset = 0;

	unsigned int currVB = 0;
	unsigned int currTFB = 1;
	GLuint particleBuffer[2];
	GLuint transformFeedback[2];

	enum Type
	{
		Volume,
		Emitter
	};

	string typeString[2]
	{
		"Volume",
		"Emitter"
	};
};