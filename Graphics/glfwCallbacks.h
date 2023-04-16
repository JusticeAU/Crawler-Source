#pragma once
#include "Graphics.h"
#include "Camera.h"

#include "LogUtils.h"
#include "TextureManager.h"
#include "FrameBuffer.h"

void WindowResizeCallback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);								// Update GL viewport size.
	Window::Get()->m_viewPortSize = {width, height};				// Update our cached size for it
	Camera::s_instance->SetAspect(width / (float)height);			// update aspect ratio of our 'editor' camera only (not in world cameras, thats a TO DO / should i even?)
	auto fbList = TextureManager::FrameBuffers();					// Update texture resolution of all framebuffers markeed as 'ScreenBuffers' - i.e. should be displayed directly to screen. keep them crisp.
	for (auto fb : *fbList)
	{
		if (fb.second != nullptr && fb.second->isScreenBuffer())
			fb.second->Resize(width, height);
	}
}