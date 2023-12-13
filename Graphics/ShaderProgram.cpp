#include "ShaderProgram.h"
#include "FileUtils.h"
#include <iostream>
#include "LogUtils.h"

#include "ComponentParticleSystem.h"

void ShaderProgram::LoadFromFiles(std::string vertFilename, std::string fragFilename)
{
	CreateShaderProgram();
	LoadStageFromFile(vertFilename, GL_VERTEX_SHADER);
	LoadStageFromFile(fragFilename, GL_FRAGMENT_SHADER);
	Link();

}

void ShaderProgram::LoadFromFiles(std::string geomFilename, std::string vertFilename, std::string fragFilename)
{
	CreateShaderProgram();
	LoadStageFromFile(geomFilename, GL_GEOMETRY_SHADER);
	LoadStageFromFile(vertFilename, GL_VERTEX_SHADER);
	LoadStageFromFile(fragFilename, GL_FRAGMENT_SHADER);
	Link();
}

void ShaderProgram::CreateShaderProgram()
{
	shaderProgramID = glCreateProgram();
}

void ShaderProgram::LoadStageFromFile(std::string filename, int shaderStage)
{
	GLchar errorLog[512];
	GLint successStatus = 0;
	GLuint currentShaderID = 0;

	std::string shaderSource = FileUtils::LoadFileAsString(filename);
	const char* shaderSourceC = shaderSource.c_str();

	switch (shaderStage)
	{
	case GL_GEOMETRY_SHADER:
	{
		geometryShaderID = glCreateShader(GL_GEOMETRY_SHADER);
		currentShaderID = geometryShaderID;
		geomFilename = filename;
		break;
	}
	case GL_VERTEX_SHADER:
	{
		vertexShaderID = glCreateShader(GL_VERTEX_SHADER);
		currentShaderID = vertexShaderID;
		vertFilename = filename;
		break;
	}
	case GL_FRAGMENT_SHADER:
	{
		fragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);
		currentShaderID = fragmentShaderID;
		fragFilename = filename;
		break;
	}
	}

	glShaderSource(currentShaderID, 1, &shaderSourceC, nullptr);
	glCompileShader(currentShaderID);

	// Check for success
	glGetShaderiv(currentShaderID, GL_COMPILE_STATUS, &successStatus);
	if (successStatus == GL_FALSE)
	{
		LogUtils::Log("Shader compilation failure");
		glGetShaderInfoLog(currentShaderID, 512, nullptr, errorLog);
		LogUtils::Log(errorLog);
		loaded = false;
	}
	else
	{
		LogUtils::Log("Shader compilation success!");
		glAttachShader(shaderProgramID, currentShaderID);
	}
}

void ShaderProgram::Link()
{
	GLchar errorLog[512];
	GLint successStatus = 0;
	glLinkProgram(shaderProgramID);

	// Check for success
	glGetProgramiv(shaderProgramID, GL_LINK_STATUS, &successStatus);
	if (successStatus == GL_FALSE)
	{
		LogUtils::Log("Shader Link Failed");
		glGetProgramInfoLog(shaderProgramID, 512, nullptr, errorLog);
		LogUtils::Log(errorLog);
		loaded = false;
	}
	else
	{
		LogUtils::Log("Shader Link Success");
		loaded = true;
	}

	if (loaded)
		LogUtils::Log("Shader loaded successfully");
}

void ShaderProgram::Bind()
{
	if (!loaded)
	{
		LogUtils::Log("Trying to bind a shader that is not successfully loaded. Bailing.");
		return;
	}
	glUseProgram(shaderProgramID);
}

void ShaderProgram::Reload()
{
	// Delete old shaders.
	glDeleteShader(geometryShaderID);
	glDeleteShader(vertexShaderID);
	glDeleteShader(fragmentShaderID);
	glDeleteProgram(shaderProgramID);

	// Start again.
	if (geomFilename != "")
		LoadFromFiles(geomFilename, vertFilename, fragFilename);
	else
		LoadFromFiles(vertFilename, fragFilename);
}

void ShaderProgram::SetBoolUniform(std::string variableName, bool value)
{
	GLint uniformLocation = glGetUniformLocation(shaderProgramID, variableName.c_str());
	glUniform1i(uniformLocation, value);
}

void ShaderProgram::SetFloatUniform(std::string variableName, float value)
{
	GLint uniformLocation = glGetUniformLocation(shaderProgramID, variableName.c_str());
	glUniform1f(uniformLocation, value);
}

void ShaderProgram::SetFloat3ArrayUniform(std::string arrayName, int elements, const glm::vec3* firstValue)
{
	GLint uniformLocation = glGetUniformLocation(shaderProgramID, arrayName.c_str());
	glUniform3fv(uniformLocation, elements, (float*)firstValue);
}

void ShaderProgram::SetVector2Uniform(std::string variableName, glm::vec2 value)
{
	GLint uniformLocation = glGetUniformLocation(shaderProgramID, variableName.c_str());
	glUniform2f(uniformLocation, value.x, value.y);
}

void ShaderProgram::SetVector3Uniform(std::string variableName, glm::vec3 value)
{
	GLint uniformLocation = glGetUniformLocation(shaderProgramID, variableName.c_str());
	glUniform3f(uniformLocation, value.x, value.y, value.z);
}

void ShaderProgram::SetMatrixUniform(std::string variableName, glm::mat4 value)
{
	GLint uniformLocation = glGetUniformLocation(shaderProgramID, variableName.c_str());
	glUniformMatrix4fv(uniformLocation, 1, GL_FALSE, &value[0][0]);
}

void ShaderProgram::SetMatrixArrayUniform(std::string variableName, int elements, const glm::mat4* firstValue)
{
	GLint uniformLocation = glGetUniformLocation(shaderProgramID, variableName.c_str());
	// can use the transpose field here to flip the row/column major order of the matrix.
	glUniformMatrix4fv(uniformLocation, elements, GL_FALSE, (float*)firstValue);
}

void ShaderProgram::SetIntUniform(std::string variableName, int value)
{
	GLint uniformLocation = glGetUniformLocation(shaderProgramID, variableName.c_str());
	glUniform1i(uniformLocation, value);
}
void ShaderProgram::SetIVec4Uniform(std::string variableName, glm::ivec4 value)
{
	GLint uniformLocation = glGetUniformLocation(shaderProgramID, variableName.c_str());
	glUniform4i(uniformLocation, value[0], value[1], value[2], value[3]);
}
void ShaderProgram::SetUIntUniform(std::string variableName, unsigned int value)
{
	GLint uniformLocation = glGetUniformLocation(shaderProgramID, variableName.c_str());
	glUniform1ui(uniformLocation, value);
}

void ShaderProgram::SetUniformBlockIndex(string uniformBlockName, const unsigned int index)
{
	unsigned int blockID = glGetUniformBlockIndex(shaderProgramID, uniformBlockName.c_str());
	glUniformBlockBinding(shaderProgramID, blockID, index);
}