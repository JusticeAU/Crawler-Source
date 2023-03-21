#pragma once
#include <string>
#include <map>
#include "Mesh.h"

using std::string;
using std::map;

class MeshManager
{
public:
	static void Init();

	static Mesh* GetMesh(string name);

	static bool LoadMesh(string name);
	static void DrawGUI();
	static MeshManager* s_instance;
	static const map<string, Mesh*>* Meshes() { return &s_instance->meshes; }
protected:
	MeshManager();
	map<string, Mesh*> meshes;

	void CreateCube();
	void CreateQuad();
	void LoadFromFile(const char* filename);

	void LoadAllFiles();
};