#pragma once
#include <string>
#include "glm.hpp"
struct AudioStep
{
	float time = 0.0f;
	std::string load = "";
	std::string stream = "";
	glm::vec3 position = { 0,0,0 };
};