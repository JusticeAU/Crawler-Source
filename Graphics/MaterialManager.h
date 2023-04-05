#pragma once
#include "Material.h"
#include <string>
#include <map>

using std::string;
using std::map;

class MaterialManager
{
public:
	~MaterialManager();
	MaterialManager(MaterialManager const& other) = delete;
	MaterialManager& operator=(const MaterialManager& other) = delete;

	static void Init();

	static Material* GetMaterial(string name);
	static void DrawGUI();
	static MaterialManager* s_instance;
	static const map<string, Material*>* Materials() { return &s_instance->materials; }
protected:
	MaterialManager();
	map<string, Material*> materials;

	void LoadFromFile(const char* filename);
	void LoadAllFiles();

	string newFileName = "models/model/modelname.mtl";
};

