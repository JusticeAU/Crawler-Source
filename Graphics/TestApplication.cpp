#include "TestApplication.h"
#include "Graphics.h"
#include "MathUtils.h"
#include "MeshManager.h"
#include "TextureManager.h"

TestApplication::TestApplication(GLFWwindow* window) : window(window)
{
	float aspect;
	int width, height;
	glfwGetWindowSize(window, &width, &height);
	aspect = width / (float)height;


	MeshManager::Init();
	TextureManager::Init();

	Input::Init(window);
	
	camera = new Camera(aspect,  window);

	Scene::CreateObject();
}

void TestApplication::Update(float delta)
{
	MeshManager::DrawGUI();
	TextureManager::DrawGUI();

	Input::Update();
	camera->Update(delta);
	camera->DrawGUI();

	scene.Update(delta);
	scene.DrawObjects();
	scene.DrawGUI();
	scene.CleanUp();
}
