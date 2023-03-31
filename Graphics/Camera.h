#pragma once
#include "Graphics.h"
#include "Input.h"

using glm::mat4;
using glm::vec3;

class Camera
{
public:
	Camera(float aspect, GLFWwindow* window);
	void Update(float delta);
	void Move(glm::vec3 delta);
	void DrawGUI();
	mat4 GetMatrix();
	vec3 GetPosition() { return position; };

	static Camera* s_instance;

	float m_horizontal = -90.0f;
	float m_vertical = 0.0f;
	float moveSpeed = 50.0f;
	float lookSpeed = .3f;
	float nearClip = 0.1f;
	float farClip = 2000.0f;

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

	void UpdateMatrix();
};

