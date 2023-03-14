#pragma once
#include "Graphics.h"
#include <string>

class ShaderProgram
{
public:
	GLuint vertexShaderID;
	GLuint fragmentShaderID;
	GLuint shaderProgramID;

	bool loaded = true;

	void LoadFromFiles(std::string vertFilename, std::string fragFilename);
	void Bind();
	void SetFloatUniform(std::string variableName, float value);
};