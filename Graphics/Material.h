#pragma once
#include "Graphics.h"

using glm::vec3;

class Material
{
public:
	void DrawGUI();

	vec3 Ka; // ambient colour of the surface
	vec3 Kd; // diffuse colour of the surface
	vec3 Ks; // specular colour of the surface
	float specularPower; // tightness of specular highlights
};