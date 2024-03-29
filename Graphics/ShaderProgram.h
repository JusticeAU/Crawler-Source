#pragma once
#include "Graphics.h"
#include "glm.hpp"
#include <string>

using std::string;

class ShaderProgram
{
public:
	void LoadFromFiles(std::string vertFilename, std::string fragFilename);
	void LoadFromFiles(std::string geomFilename, std::string vertFilename, std::string fragFilename);
	
	void CreateShaderProgram();
	void LoadStageFromFile(std::string filename, int shaderStage);
	void Link();

	void Bind();

	void Reload();

	GLuint GetShaderProgramID() { return shaderProgramID; }

	void SetBoolUniform(std::string variableName, bool value);
	void SetFloatUniform(std::string variableName, float value);
	void SetFloat3ArrayUniform(std::string arrayName, int elements, const glm::vec3* firstValue);
	void SetVector2Uniform(std::string variableName, glm::vec2 value);
	void SetVector3Uniform(std::string variableName, glm::vec3 value);
	void SetMatrixUniform(std::string variableName, glm::mat4 value);
	void SetMatrixArrayUniform(std::string variableName, int elements, const glm::mat4* firstValue);
	void SetIntUniform(std::string variableName, int value);
	void SetIVec4Uniform(std::string variableName, glm::ivec4 value);
	void SetUIntUniform(std::string variableName, unsigned int value);

	void SetUniformBlockIndex(string uniformBlockName, const unsigned int index);

	string name = "";
protected:
	GLuint geometryShaderID = 0;
	GLuint vertexShaderID = 0;
	GLuint fragmentShaderID = 0;
	GLuint shaderProgramID = 0;

	string geomFilename = "";
	string vertFilename = "";
	string fragFilename ="";

	bool loaded = false;
	bool isParticleSystemShader = false;
};