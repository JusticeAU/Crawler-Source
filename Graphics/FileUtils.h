#pragma once
#include <iostream>
#include <fstream>
#include <sstream>

static class FileUtils
{
public:
	static std::string LoadFileAsString(std::string filepath)
	{
		std::ifstream file(filepath);
		std::stringstream buffer;

		if (file.is_open())
		{
			while (!file.eof())
			{
				std::string line;
				std::getline(file, line);
				buffer << line << std::endl;
			}
			return buffer.str();
		}
		else
		{
			std::cout << "Failed to load file: " + filepath << std::endl;
			return "";
		}
	}
};