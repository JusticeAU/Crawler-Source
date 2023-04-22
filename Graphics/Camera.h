#pragma once
#include "Graphics.h"
#include "Input.h"
#include <string>

using glm::mat4;
using glm::vec3;
using std::string;

class FrameBuffer;

// provides a pvm matrix for rendering and some APIs to move it around.
class Camera
{
public:
	Camera(float aspect, GLFWwindow* window, string name);
	void Update(float delta);
	void Move(glm::vec3 delta);
	void DrawGUI();
	mat4 GetView() { return view; }
	mat4 GetProjection() { return projection; }
	mat4 GetMatrix();
	vec3 GetPosition() { return position; };
	void SetAspect(float value) { aspect = value; UpdateMatrix(); }
	FrameBuffer* GetFrameBuffer();
	void ResizeFrameBuffer(int width, int height);

	static Camera* s_instance;

	float m_horizontal = -90.0f;
	float m_vertical = 0.0f;
	float moveSpeed = 20.0f;
	float lookSpeed = .3f;
	float nearClip = 0.1f;
	float farClip = 2000.0f;

	string name = "";

protected:
	glm::vec3 position;
	glm::vec3 forward;
	glm::vec3 right;
	glm::vec3 up;

	float aspect;

	glm::mat4 view;
	glm::mat4 projection;
	glm::mat4 matrix;

	GLFWwindow* window = nullptr;

	FrameBuffer* frameBuffer;

	bool isAdjusting = false;

	void UpdateMatrix();
	void SetFramebuffer(FrameBuffer* fb) { frameBuffer = fb; };
};

