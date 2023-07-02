#pragma once

#include "glm2json.h"
#include "json.hpp"
#include <string>
#include <fstream>
#include <iostream>

using std::string;
using nlohmann::json;
using nlohmann::ordered_json;

static void WriteJSONToDisk(string filename, ordered_json json)
{
	std::fstream file(filename, std::ios::out);
	file << std::setw(2) << json;
	file.close();
}

static ordered_json ReadJSONFromDisk(string filename)
{
	std::ifstream inFile(filename);
	auto input = ordered_json::parse(inFile);
	inFile.close();
	return input;
}


// Serialisation example
//namespace glm
//{
//	static void to_json(nlohmann::ordered_json& j, const vec3& vec3)
//	{
//		j = { {"x", vec3.x}, {"y", vec3.y}, {"z", vec3.z} };
//	}
//
//	static void from_json(const nlohmann::ordered_json& j, vec3& vec3)
//	{
//		j.at("x").get_to(vec3.x);
//		j.at("y").get_to(vec3.y);
//		j.at("z").get_to(vec3.z);
//	}
//}