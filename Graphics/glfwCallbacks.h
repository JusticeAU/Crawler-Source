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
	// Hack for when the game gets minamised, width and height gets set to 0 in this callback
	if (width == 0) width = 1;
	if (height == 0) height = 1;

	glViewport(0, 0, width, height);								// Update GL viewport size.
	Window::SetViewPortSize({width, height});				// Update our cached size for it
	for (auto& c : Scene::s_instance->componentCameras) // this is not perfect - it'll only proc for the active scene - shouldnt be a problem. ill likely need to add in a check when we select a new camera.
		c->SetAspect(width / (float)height);
	if(Scene::s_editorCamera) Scene::s_editorCamera->camera->SetAspect(width / (float)height);
	TextureManager::RefreshFrameBuffers();
}