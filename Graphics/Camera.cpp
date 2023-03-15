#include "Camera.h"

Camera::Camera(float aspect)
{
	position = { 0, 0, -10 };
	rotation = { 0,0,0 };
	this->aspect = aspect;

	view = glm::translate(glm::mat4(1), position);
	projection = glm::perspective((float)3.14159 / 4, aspect, .1f, 100.0f);
	matrix = projection * view;
}

void Camera::Move(glm::vec3 delta)
{
	position += delta;
	view = glm::translate(glm::mat4(1), position);
	projection = glm::perspective((float)3.14159 / 4, aspect, .1f, 100.0f);
	matrix = projection * view;
}

void Camera::Rotate(glm::vec3 delta)
{
	rotation += delta;
	view = glm::translate(glm::mat4(1), position);
}

glm::mat4 Camera::GetMatrix() { return matrix; }