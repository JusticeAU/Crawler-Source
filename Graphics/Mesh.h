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
		vec3 position = {0,0,0};
		vec3 colour = { 0,0,0 };
		vec3 normal = { 0,0,0 };
		vec2 uv = { 0,0 };
		vec4 tangent = { 0,0,0,0 };
		int boneID[4] = {-1,-1,-1,-1};
		float boneWeight[4] = { 0,0,0,0 };
	};

	struct BoneInfo
	{
		glm::mat4 offset = glm::mat4(1);
	};

	struct Animation
	{
		struct AnimationKey
		{
			vec3 position;
			glm::quat rotation;
			vec3 scale;
		};

		struct AnimationChannel
		{
			string name;
			AnimationKey keys[500];
		};

		string name;
		float duration;
		float ticksPerSecond;
		int numChannels;
		map<string, AnimationChannel> channels;
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
	vector<BoneInfo> boneInfo;

	vector<Animation> animations;
};