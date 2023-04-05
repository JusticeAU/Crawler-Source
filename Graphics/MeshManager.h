#pragma once
#include "Model.h"
#include <string>
#include <map>

using std::string;
using std::map;

class Object;
struct aiNode;
struct aiMesh;
struct aiScene;

class MeshManager
{
public:
	~MeshManager();
	MeshManager(MeshManager const& other) = delete;
	MeshManager& operator=(const MeshManager& other) = delete;

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