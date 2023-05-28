#pragma once
#include "Graphics.h"
#include "Input.h"
#include "AudioListener.h"
#include <string>

using glm::mat4;
using glm::vec3;
using std::string;

class FrameBuffer;

// provides a pvm matrix for rendering and some APIs to move it around.
class Camera
{
public:
	Camera(float aspect, string name);
	void Update(float delta);
	void Move(glm::vec3 delta);
	void DrawGUI();
	mat4 GetView() { return view; }
	mat4 GetProjection() { return projection; }
	mat4 GetMatrix();
	vec3 GetPosition() { return position; };
	void SetAspect(float value) { aspect = value; UpdateMatrix(); }
	FrameBuffer* GetFrameBuffer();
	AudioListener* GetAudioListener() { return &m_audioListener; }

	static Camera* s_instance;

	float m_horizontal = -90.0f;
	float m_vertical = 0.0f;
	float moveSpeed = 20.0f;
	float lookSpeed = .3f;
	float nearClip = 0.1f;
	float farClip = 2000.0f;

	string name = "";

	glm::vec3 position;
	glm::vec3 forward;
	glm::vec3 right;
	glm::vec3 up;
protected:
	AudioListener m_audioListener;
	float aspect;

	glm::mat4 view;
	glm::mat4 projection;
	glm::mat4 matrix;

	FrameBuffer* frameBuffer;

	bool isAdjusting = false;

	void UpdateMatrix();
	void UpdateAudioListener();
	void SetFramebuffer(FrameBuffer* fb) { frameBuffer = fb; };
};

