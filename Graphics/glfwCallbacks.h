#pragma once
#include "Graphics.h"
#include "Camera.h"

void WindowResizeCallback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
	Camera::s_instance->SetAspect(width / (float)height);
}