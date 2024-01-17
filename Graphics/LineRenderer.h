#pragma once
#include "Graphics.h"
#include <vector>

class ShaderProgram;

// Immediate Mode 3D drawing.
class LineRenderer
{
public:
	// Must be called once - Sets the shader and creates the buffer.
	static void Initialise();
	// Add a Line to be drawn on the next render call.
	static void DrawLine(glm::vec3 a, glm::vec3 b, glm::vec3 colour = { 1,1,1 });
	// Add a box to be drawn on the next render call. The box is oriented to the ground.
	static void DrawFlatBox(glm::vec3 position, float size, glm::vec3 colour = { 1,1,1 });

	static void DrawAABB(glm::vec3 min, glm::vec3 max, glm::vec3 colour = { 1,1,1 });

	static void DrawBoxFromPoints(glm::vec3* arrayOfEightPoints, glm::vec3 colour = { 1,1,1 });

	// Upload the point and colour data do the GPU and then render.
	static void Render(glm::mat4 pvMatrix);

	static LineRenderer* s_instance;
	std::vector<glm::vec3> verts;
	std::vector<glm::vec3> colours;
	GLuint vertBufferID;
	GLuint colourBufferID;
	ShaderProgram* shader = nullptr;
};

