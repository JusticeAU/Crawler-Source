#pragma once
#include "Graphics.h"

using glm::vec3;

class Light
{
public:
	Light(vec3 position = {0,0,0}, vec3 colour = {1,1,1});
	vec3 position;
	vec3 colour;
	float intensity = 10.0f;
};

