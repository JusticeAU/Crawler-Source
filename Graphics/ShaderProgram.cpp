#include "ShaderProgram.h"
#include "FileUtils.h"
#include <iostream>
#include "LogUtils.h"

void ShaderProgram::LoadFromFiles(std::string vertFilename, std::string fragFilename)
{
	vertexShaderID = glCreateShader(GL_VERTEX_SHADER);
	fragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);
	shaderProgramID = glCreateProgram();
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
			LogUtils(errorLog);
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
			LogUtils(errorLog);
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
		LogUtils::Log("Shader Link Success");

	if (loaded)
		LogUtils::Log("Shader loaded successfully");

}