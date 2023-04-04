#pragma once
#include "Model.h"
#include <string>
#include <map>
#include "Mesh.h"

using std::string;
using std::map;

class Object;
class aiNode;
class aiMesh;
class aiScene;

class MeshManager
{
public:
	static void Init();

	static Mesh* GetMesh(string name);

	static void DrawGUI();
	static MeshManager* s_instance;
	static const map<string, Mesh*>* Meshes() { return &s_instance->meshes; }
	static Mesh* LoadFromAiMesh(const aiMesh* mesh, Model::BoneStructure* boneStructure, const char* name);
	static void CopyNodeHierarchy(const aiScene* scene, aiNode* node, Object* parent, Model::BoneStructure* boneStructure = nullptr);
protected:
	MeshManager();
	map<string, Mesh*> meshes;

	void CreateCube();
	void CreateQuad();
};