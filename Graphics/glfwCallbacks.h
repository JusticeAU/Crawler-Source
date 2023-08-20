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
	for (auto& c : Scene::s_instance->componentCameras) // this is not perfect - it'll only proc for the active scene - shouldnt be a problem. ill likely need to add in a check when we select a new camera.
		c->SetAspect(width / (float)height);
	if(Scene::s_editorCamera) Scene::s_editorCamera->camera->SetAspect(width / (float)height);
	TextureManager::RefreshFrameBuffers();
}