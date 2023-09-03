#pragma once
#include "Graphics.h"
#include <vector>

class ShaderProgram;

class LineRenderer
{
public:
	static void Initialise();
	static void DrawLine(glm::vec3 a, glm::vec3 b, glm::vec3 colour = { 1,1,1 });

	static void Render(glm::mat4 pvMatrix);

	static LineRenderer* s_instance;
	std::vector<glm::vec3> verts;
	std::vector<glm::vec3> colours;
	GLuint vertBufferID;
	GLuint colourBufferID;
	ShaderProgram* shader = nullptr;
};

