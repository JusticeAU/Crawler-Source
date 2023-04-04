#pragma once
#include "Graphics.h"
#include <string>
#include <vector>

using glm::vec4;
using glm::vec3;
using glm::vec2;
using glm::quat;

using std::string;

class Mesh
{
public:
	struct Vertex
	{
		vec3 position = {0,0,0};
		vec3 colour = { 0,0,0 };
		vec3 normal = { 0,0,0 };
		vec2 uv = { 0,0 };
		vec4 tangent = { 0,0,0,0 };
		int boneID[4] = {-1,-1,-1,-1};
		float boneWeight[4] = { 0,0,0,0 };
	};

	void Initialise(unsigned int vertCount, const Vertex* vertices, unsigned int indexCount = 0, unsigned int* indices = nullptr);
	static void CalculateTangents(Vertex* vertices, unsigned int vertexCount, const std::vector<unsigned int>& indices);
public:
	unsigned int tris;
	unsigned int vao, vbo, ibo;
};