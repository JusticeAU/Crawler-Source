#pragma once
#include "Application.h"
#include "glm.hpp"
#include <vector>
#include "ShaderProgram.h"
#include "Camera.h"

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
	ShaderProgram* shader;

	GLuint bufferID;

	// Unit Cube
	float someFloats[216]
	{
		-0.5, -0.5, -0.5,    0, 0, 1,
		-0.5, 0.5, -0.5,    0, 0, 1,
		0.5, -0.5, -0.5,    0, 0, 1,
		-0.5, 0.5, -0.5,    0, 0, 1,
		0.5, -0.5, -0.5,    0, 0, 1,
		0.5, 0.5, -0.5,    0, 0, 1,

		-0.5, -0.5, 0.5,    1, 1, 0,
		-0.5, 0.5, 0.5,    1, 1, 0,
		0.5, -0.5, 0.5,    1, 1, 0,
		-0.5, 0.5, 0.5,    1, 1, 0,
		0.5, -0.5, 0.5,    1, 1, 0,
		0.5, 0.5, 0.5,    1, 1, 0,


		-0.5, -0.5, -0.5,    0, 1, 0,
		-0.5, -0.5, 0.5,    0, 1, 0,
		0.5, -0.5, -0.5,    0, 1, 0,
		-0.5, -0.5, 0.5,    0, 1, 0,
		0.5, -0.5, -0.5,    0, 1, 0,
		0.5, -0.5, 0.5,    0, 1, 0,

		-0.5, 0.5, -0.5,    1, 0, 1,
		-0.5, 0.5, 0.5,    1, 0, 1,
		0.5, 0.5, -0.5,    1, 0, 1,
		-0.5, 0.5, 0.5,    1, 0, 1,
		0.5, 0.5, -0.5,    1, 0, 1,
		0.5, 0.5, 0.5,    1, 0, 1,


		-0.5, -0.5, -0.5,    1, 0, 0,
		-0.5, -0.5, 0.5,    1, 0, 0,
		-0.5, 0.5, -0.5,    1, 0, 0,
		-0.5, -0.5, 0.5,    1, 0, 0,
		-0.5, 0.5, -0.5,    1, 0, 0,
		-0.5, 0.5, 0.5,    1, 0, 0,

		0.5, -0.5, -0.5,    0, 1, 1,
		0.5, -0.5, 0.5,    0, 1, 1,
		0.5, 0.5, -0.5,    0, 1, 1,
		0.5, -0.5, 0.5,    0, 1, 1,
		0.5, 0.5, -0.5,    0, 1, 1,
		0.5, 0.5, 0.5,    0, 1, 1,
	};

	Camera* camera;
};