#pragma once
#include <string>
#include <map>
#include "ShaderProgram.h"

using std::string;
using std::map;

class ShaderManager
{
public:
	static void Init();

	static ShaderProgram* GetShaderProgram(string name);
	static void DrawGUI();
	static ShaderManager* s_instance;
	static const map<string, ShaderProgram*>* ShaderPrograms() { return &s_instance->shaderPrograms; }
protected:
	ShaderManager();
	map<string, ShaderProgram*> shaderPrograms;

	void LoadFromFile(string filename);

	void LoadAllFiles();

	void RecompileAllShaderPrograms();
};

