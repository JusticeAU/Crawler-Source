#pragma once
#include "Graphics.h"

class Camera
{
public:
	Camera(float aspect);
	void Move(glm::vec3 delta);
	void Rotate(glm::vec3 delta);
	glm::mat4 GetMatrix();

	static Camera* s_instance;

protected:
	glm::vec3 position;
	glm::vec3 rotation;

	float aspect;

	glm::mat4 view;
	glm::mat4 projection;

	glm::mat4 matrix;
};

