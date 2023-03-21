#pragma once
#include "Graphics.h"
#include <vector>

using glm::vec4;
using glm::vec3;
using glm::vec2;

using std::vector;

class Mesh
{
public:
	struct Vertex
	{
		vec3 position;
		vec3 colour;
		vec3 normal;
		vec2 uv;
	};
	void Initialise(unsigned int vertCount, const Vertex* vertices, unsigned int indexCount = 0, unsigned int* indices = nullptr);

public:
	unsigned int tris;
	unsigned int vao, vbo, ibo;
};