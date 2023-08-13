#pragma once
#include "Graphics.h"
#include "Camera.h"

#include "TextureManager.h"
#include "FrameBuffer.h"

#include "Window.h"

void WindowResizeCallback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);								// Update GL viewport size.
	Window::SetViewPortSize({width, height});				// Update our cached size for it
	Camera::s_instance->SetAspect(width / (float)height);			// update aspect ratio of our 'editor' camera only (not in world cameras, thats a TO DO / should i even?)
	TextureManager::RefreshFrameBuffers();
}