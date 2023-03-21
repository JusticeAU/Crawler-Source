#include "TestApplication.h"
#include "Graphics.h"
#include "MathUtils.h"
#include "MeshManager.h"

TestApplication::TestApplication(GLFWwindow* window) : window(window)
{
	float aspect;
	int width, height;
	glfwGetWindowSize(window, &width, &height);
	aspect = width / (float)height;


	MeshManager::Init();
	camera = new Camera(aspect,  window);
	input = new Input(window);
	

	Scene::CreateObject();
}

void TestApplication::Update(float delta)
{
	MeshManager::DrawGUI();

	input->Update();
	camera->Update(delta);

	scene.Update(delta);
	scene.DrawObjects();
	scene.DrawGUI();
	scene.CleanUp();
}
