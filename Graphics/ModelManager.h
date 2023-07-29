#pragma once
#include "Graphics.h"
#include <string>
#include <map>

class Model;
class Animation;

using std::string;
using std::map;
using glm::mat4;

class ModelManager
{
public:
	static void Init();
	static void LoadAllFiles(string folder, mat4 transformOverride = mat4(1));

	static Model* GetModel(string name);
	static Animation* GetAnimation(string name);


	static void DrawGUI();
	static ModelManager* s_instance;
	static const map<string, Model*>* Resources() { return &s_instance->resources; }
	static const map<string, Animation*>* Animations() { return &s_instance->animations; }

	void LoadFromFile(const char* filename, mat4 transformOverride);
protected:
	ModelManager();
	map<string, Model*> resources;
	map<string, Animation*> animations;

};

