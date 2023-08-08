#pragma once
#include <string>
#include <locale>

class StringUtils
{
public:
	static std::string ToLower(std::string input)
	{
		std::string output = "";
		for (auto elem : input)
			output += std::tolower(elem);

		return output;
	}
};