#pragma once
#include "Graphics.h"
#include <string>

using glm::vec3;
using std::string;

class Texture;
class ShaderProgram;

class Material
{
public:
	void DrawGUI();
	void SaveToFile();

	string name;
	string filePath;

	vec3 Ka = { 0.0f, 0.0f, 0.0f }; // ambient colour of the surface
	vec3 Kd = { 1.0f, 1.0f, 1.0f };; // diffuse colour of the surface
	vec3 Ks = { 1.0f, 1.0f, 1.0f };; // specular colour of the surface
	float specularPower = 20.0f; // tightness of specular highlights

	Texture* mapKd = nullptr; // Diffuse texture map
	string mapKdName = "";
	Texture* mapKs = nullptr; // specular texture map
	string mapKsName = "";
	Texture* mapBump = nullptr; // normal map
	string mapBumpName = "";

	ShaderProgram* shader	= nullptr;
	string shaderName		= "";

	// PBR
	Texture* albedoMap		= nullptr;
	Texture* normalMap		= nullptr;
	Texture* metallicMap	= nullptr;
	Texture* roughnessMap	= nullptr;
	Texture* aoMap			= nullptr;
	string albedoMapName	= "";
	string normalMapName	= "";
	string metallicMapName	= "";
	string roughnessMapName = "";
	string aoMapName		= "";

};