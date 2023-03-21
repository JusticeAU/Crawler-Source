#pragma once
#include <string>
#include <map>
#include "Mesh.h"

using std::string;
using std::map;

class MeshManager
{
public:
	MeshManager();
	static Mesh* GetMesh(string name);

	static bool LoadMesh(string name);
	static void DrawGUI();
	static MeshManager* s_instance;
	map<string, Mesh*> meshes;
protected:
	vector<string> meshNames;

	void CreateCube();
	void CreateQuad();
	void LoadFromFile(const char* filename);

	void LoadAllFiles();
};