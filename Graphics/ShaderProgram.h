#pragma once
#include "Graphics.h"
#include "glm.hpp"
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
	void SetFloat3ArrayUniform(std::string arrayName, int elements, const glm::vec3* firstValue);
	void SetVectorUniform(std::string variableName, glm::vec3 value);
	void SetMatrixUniform(std::string variableName, glm::mat4 value);
	void SetIntUniform(std::string variableName, int value);
};