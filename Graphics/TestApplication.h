#pragma once
#include "Application.h"
#include "glm.hpp"
#include <vector>
#include "ShaderProgram.h"

class TestApplication : public Application
{
public:
	TestApplication();
	void Update(float delta) override;
protected:
	std::vector<glm::vec4> colors;
	float t = 0.0f;
	int colorIndex = 0;
	int nextColor = 1;
	ShaderProgram* shader;

	GLuint bufferID;
	float someFloats[6]
	{
		0, 0,
		0, 1,
		1, 0
	};
};