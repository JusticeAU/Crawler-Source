#pragma once
#include "Graphics.h"
#include "Camera.h"

#include "LogUtils.h"

void WindowResizeCallback(GLFWwindow* window, int width, int height)
{
	LogUtils::Log("Resizing");
	glViewport(0, 0, width, height);
	Window::Get()->m_viewPortSize = {width, height};
	Camera::s_instance->SetAspect(width / (float)height);
	Camera::s_instance->ResizeFrameBuffer(width, height);
}