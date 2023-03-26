#pragma once
#include "Graphics.h"
#include <string>

using glm::vec3;
using std::string;

class Texture;

class Material
{
public:
	void DrawGUI();

	vec3 Ka; // ambient colour of the surface
	vec3 Kd; // diffuse colour of the surface
	vec3 Ks; // specular colour of the surface
	float specularPower; // tightness of specular highlights

	Texture* mapKd; // Diffuse texture map
	string mapKdName;
	Texture* mapKs; // specular texture map
	string mapKsName;
	Texture* mapBump; // normal map
	string mapBumpName;
};