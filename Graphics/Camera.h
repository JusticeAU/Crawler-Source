#pragma once
#include "Graphics.h"
#include "Input.h"

class Camera
{
public:
	Camera(float aspect, GLFWwindow* window);
	void Update(float delta);
	void Move(glm::vec3 delta);
	void DrawGUI();
	glm::mat4 GetMatrix();

	static Camera* s_instance;

	float m_horizontal = -90.0f;
	float m_vertical = 0.0f;
	float moveSpeed = 1.5f;
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

