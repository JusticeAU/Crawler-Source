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