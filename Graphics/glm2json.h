#pragma once

#include "glm.hpp"
#include "json.hpp"

namespace glm
{
	static void to_json(nlohmann::ordered_json& j, const vec3& vec3)
	{
		j = { {"x", vec3.x}, {"y", vec3.y}, {"z", vec3.z} };
	}

	static void from_json(const nlohmann::ordered_json& j, vec3& vec3)
	{
		j.at("x").get_to(vec3.x);
		j.at("y").get_to(vec3.y);
		j.at("z").get_to(vec3.z);
	}
}