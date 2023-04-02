#pragma once
#include "Graphics.h"
#include <vector>
#include <map>
#include <string>

using glm::vec4;
using glm::vec3;
using glm::vec2;
using glm::quat;

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

	struct Animation // an animation contains some meta data and a collection of channels (bones) which contain a collection of keys (position, rotation, scale).
	{
		struct AnimationKey // a key is a keyframe of information about a particular bone.
		{
			vec3 position;
			quat rotation;
			vec3 scale;
		};

		struct AnimationChannel // a channel is a bone
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

	// Node Hierarchy
	vector<Object*> childNodes;

	// Bone mapping
	int numBones = 0;
	map<string, int> boneMapping;	// boneName and index pair. The index is useed to address in to boneInfo and assign transformations in to the buffer for the vertex shader.
	vector<BoneInfo> boneInfo;	// contains the offset for the bone.

	vector<Animation> animations;
};