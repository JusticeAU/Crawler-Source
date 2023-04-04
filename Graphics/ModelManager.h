#pragma once
#include <string>
#include <map>

class Model;

using std::string;
using std::map;

class ModelManager
{
public:
	static void Init();

	static Model* GetModel(string name);

	static void DrawGUI();
	static ModelManager* s_instance;
	static const map<string, Model*>* Resources() { return &s_instance->resources; }
protected:
	ModelManager();
	map<string, Model*> resources;

	void LoadFromFile(const char* filename);
	void LoadAllFiles();
};

