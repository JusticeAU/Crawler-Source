#pragma once
#include "Application.h"
#include "glm.hpp"
#include <vector>
#include "ShaderProgram.h"
#include "Camera.h"
#include "Scene.h"

class TestApplication : public Application
{
public:
	TestApplication(GLFWwindow* window);
	void Update(float delta) override;
protected:
	GLFWwindow* window;
	std::vector<glm::vec4> colors;
	float t = 0.0f;
	int colorIndex = 0;
	int nextColor = 1;

	Scene scene;

	Camera* camera;
};