#pragma once
#include "ShaderProgram.h"
#include <string>
#include <map>
#include <vector>

using std::string;
using std::map;
using std::vector;

class ShaderManager
{
public:
	static void Init();

	static ShaderProgram* GetShaderProgram(string name);
	static void AddShaderProgram(string name, ShaderProgram* shader) { s_instance->shaderPrograms.emplace(name, shader); }
	static bool ShaderProgramExists(string name) { return s_instance->shaderPrograms.find(name) != s_instance->shaderPrograms.end(); }
	static void DrawGUI();
	static void RecompileAllShaderPrograms();
	static ShaderManager* s_instance;
	static const map<string, ShaderProgram*>* ShaderPrograms() { return &s_instance->shaderPrograms; }
	
	static int GetPostProcessShaderCount() { return (int)s_instance->m_postProcessShaderNames.size(); }
	static string GetPostProcessShaderName(int i) { return s_instance->m_postProcessShaderNames[i]; }

protected:
	ShaderManager();
	map<string, ShaderProgram*> shaderPrograms;

	void LoadFromFile(string filename);

	void LoadAllFiles();

	vector<string> m_postProcessShaderNames;
};

