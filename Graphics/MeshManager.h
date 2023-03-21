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
protected:
	map<string, Mesh*> meshes;
	static MeshManager* s_instance;

	void CreateCube();
	void CreateQuad();
};