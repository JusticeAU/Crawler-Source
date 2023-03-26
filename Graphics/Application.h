#pragma once
#include "glm.hpp"
#include "ShaderProgram.h"
#include "Camera.h"
#include "Scene.h"
#include "Input.h"

class Application
{
public:
	Application();
	~Application();
	void Run();
	void Update(float delta);
protected:
	GLFWwindow* window;

	Camera* camera;
	Scene* scene;
};