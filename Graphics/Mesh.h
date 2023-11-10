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
	Mesh();
	~Mesh();
	Mesh(Mesh const& other) = delete;
	Mesh& operator=(const Mesh* other) = delete;

	struct Vertex
	{
		vec3 position =	{ 0,0,0 };
		vec3 colour = { 0,0,0 };
		vec3 normal = { 0,0,0 };
		vec2 uv = { 0,0 };
		vec4 tangent = { 0,0,0,1 };
		int boneID[4] = {-1,-1,-1,-1};
		float boneWeight[4] = { 0.0f,0.0f,0.0f,0.0f };
	};

	struct AABB
	{
		vec3 lowerA = { 0,0,0 };
		vec3 lowerB = { 0,0,0 };
		vec3 lowerC = { 0,0,0 };
		vec3 lowerD = { 0,0,0 };

		vec3 upperA = { 0,0,0 };
		vec3 upperB = { 0,0,0 };
		vec3 upperC = { 0,0,0 };
		vec3 upperD = { 0,0,0 };
	};

	void Initialise(unsigned int vertCount, const Vertex* vertices, unsigned int indexCount = 0, unsigned int* indices = nullptr);
	static void CalculateTangents(Vertex* vertices, unsigned int vertexCount, const std::vector<unsigned int>& indices);
public:
	std::string name = "";
	unsigned int tris;
	unsigned int vao, vbo, ibo;
	bool initialised = false;
	AABB aabb;
	vec3 aabbMin = { 0,0,0 };
	vec3 aabbMax = { 0,0,0 };
};