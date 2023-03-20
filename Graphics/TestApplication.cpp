#include "TestApplication.h"
#include "Graphics.h"
#include "MathUtils.h"

TestApplication::TestApplication(GLFWwindow* window) : window(window)
{
	float aspect;
	int width, height;
	glfwGetWindowSize(window, &width, &height);
	aspect = width / (float)height;

	camera = new Camera(aspect,  window);
	input = new Input(window);
	
	scene.objects.push_back(new Object());
}

void TestApplication::Update(float delta)
{

	input->Update();
	camera->Update(delta);

	scene.Update(delta);
	scene.DrawObjects();
	scene.DrawGUI();
	scene.CleanUp();
}
