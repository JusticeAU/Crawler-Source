#pragma once
#include "Graphics.h"
#include "serialisation.h"

using glm::vec3;

class Light
{
public:
	Light(vec3 position = { 0,0,0 }, vec3 colour = { 1,1,1 }) : position(position), colour(colour) {}
	vec3 position;
	vec3 colour;
	float intensity = 10.0f;
};

static void to_json(nlohmann::ordered_json& j, const Light& light)
{
	j["position"] = light.position;
	j["colour"] = light.colour;
	j["intensity"] = light.intensity;
}

static void from_json(const nlohmann::ordered_json& j, Light& light)
{
	j.at("position").get_to(light.position);
	j.at("colour").get_to(light.colour);
	j.at("intensity").get_to(light.intensity);
}