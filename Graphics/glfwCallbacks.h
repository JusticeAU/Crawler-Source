#pragma once
#include "Graphics.h"

#include "TextureManager.h"
#include "FrameBuffer.h"

#include "Scene.h"
#include "SceneEditorCamera.h"
#include "ComponentCamera.h"

#include "Window.h"

void WindowResizeCallback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);								// Update GL viewport size.
	Window::SetViewPortSize({width, height});				// Update our cached size for it
	//Camera::s_instance->SetAspect(width / (float)height);			// update aspect ratio of our 'editor' camera only (not in world cameras, thats a TO DO / should i even?)
	Scene::s_editorCamera->camera->SetAspect(width / (float)height);
	for (auto& c : Scene::s_instance->componentCameras) // this is not perfect - it'll only proc for the active scene - shouldnt be a problem. ill likely need to add in a check when we select a new camera.
		c->SetAspect(width / (float)height);
	TextureManager::RefreshFrameBuffers();
}