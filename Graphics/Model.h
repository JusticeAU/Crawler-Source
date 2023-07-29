#pragma once
#include "Mesh.h"
#include <map>
#include <vector>
#include <string>

class Mesh;
class Animation;
class ShaderProgram;

class Object;

using std::string;
using std::vector;
using std::map;

using glm::mat4;

// A model is a container for meshes, bone structures and animation data. They are created by the Model Manager and data populated during import.
class Model
{
public:
	struct BoneStructure
	{
		int numBones = 0;
		map<string, int> boneMapping;	// boneName and index pair. The index is useed to address in to boneOffset array and assign transformations in to the buffer for the vertex shader.
		vector<glm::mat4> boneOffsets;	// contains the offset for the bone.
	};

	string name;

	vector<Mesh*> meshes;
	vector<Animation*> animations;
	
	Object* childNodes = nullptr;
	BoneStructure* boneStructure = nullptr;

	mat4 modelTransform = mat4(1);

	void DrawAllSubMeshes();
	void DrawSubMesh(int index);

	int GetMeshCount() { return meshes.size(); }
	Mesh* GetMesh(int index) { return meshes[index]; }
};

