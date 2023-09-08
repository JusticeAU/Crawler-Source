#include "ShaderProgram.h"
#include "FileUtils.h"
#include <iostream>
#include "LogUtils.h"

void ShaderProgram::LoadFromFiles(std::string vertFilename, std::string fragFilename)
{
	vertexShaderID = glCreateShader(GL_VERTEX_SHADER);
	fragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);
	shaderProgramID = glCreateProgram();

	this->vertFilename = vertFilename;
	this->fragFilename = fragFilename;
	
	GLchar errorLog[512];
	GLint successStatus = 0;

	// Load and compile vertex shader
	std::string vertexShaderSource = FileUtils::LoadFileAsString(vertFilename);
	if (vertexShaderSource != "")
	{
		const char* vertexShaderSourceC = vertexShaderSource.c_str();
		glShaderSource(vertexShaderID, 1, &vertexShaderSourceC, nullptr);
		glCompileShader(vertexShaderID);
		// Check for success
		
		glGetShaderiv(vertexShaderID, GL_COMPILE_STATUS, &successStatus);
		if (successStatus == GL_FALSE)
		{
			LogUtils::Log("Vertex shader compilation failure");
			glGetShaderInfoLog(vertexShaderID, 512, nullptr, errorLog);
			LogUtils::Log(errorLog);
			loaded = false;
		}
		else
			LogUtils::Log("Vertex shader compilation success!");
	}
	else
		LogUtils::Log("Vertex shader load failure - empty or missing file.");
	
	// Load and compile fragment shader
	std::string fragmentShaderSource = FileUtils::LoadFileAsString(fragFilename);
	if (fragmentShaderSource != "")
	{
		const char* fragmentShaderSourceC = fragmentShaderSource.c_str();
		glShaderSource(fragmentShaderID, 1, &fragmentShaderSourceC, nullptr);
		glCompileShader(fragmentShaderID);

		// Check for success
		glGetShaderiv(fragmentShaderID, GL_COMPILE_STATUS, &successStatus);
		if (successStatus == GL_FALSE)
		{
			LogUtils::Log("Fragment shader compilation failure");
			glGetShaderInfoLog(fragmentShaderID, 512, nullptr, errorLog);
			LogUtils::Log(errorLog);
			loaded = false;
		}
		else
			LogUtils::Log("Fragment shader compilation success!");
	}
	else
		LogUtils::Log("Fragment shader load failure - empty or missing file.");

	// Link shaders
	glAttachShader(shaderProgramID, vertexShaderID);
	glAttachShader(shaderProgramID, fragmentShaderID);
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
	glDeleteShader(vertexShaderID);
	glDeleteShader(fragmentShaderID);
	glDeleteProgram(shaderProgramID);

	// Start again.
	LoadFromFiles(vertFilename, fragFilename);
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
