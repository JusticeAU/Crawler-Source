#pragma once
#include "Graphics.h"
#include <vector>
#include <map>
#include <string>

using glm::vec4;
using glm::vec3;
using glm::vec2;

using std::string;
using std::vector;
using std::map;

class Object;

class Mesh
{
public:
	struct Vertex
	{
		vec3 position;
		vec3 colour;
		vec3 normal;
		vec2 uv;
		vec4 tangent;
		int boneID[4] = {-1,-1,-1,-1};
		float boneWeight[4];
	};

	void Initialise(unsigned int vertCount, const Vertex* vertices, unsigned int indexCount = 0, unsigned int* indices = nullptr);
	static void CalculateTangents(Vertex* vertices, unsigned int vertexCount, const std::vector<unsigned int>& indices);

public:
	unsigned int tris;
	unsigned int vao, vbo, ibo;

	// Node Heirarchy Dev
	vector<Object*> childNodes;

	// Bone mapping
	map<string, int> boneMapping;
	int numBones = 0;
};