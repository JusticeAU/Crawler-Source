#pragma once
#include "Application.h"
#include "glm.hpp"
#include "ShaderProgram.h"
#include "Camera.h"
#include "Scene.h"
#include "Input.h"

class TestApplication : public Application
{
public:
	TestApplication(GLFWwindow* window);
	void Update(float delta) override;
protected:
	GLFWwindow* window;

	Camera* camera;
	Scene scene;
};