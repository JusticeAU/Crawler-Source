#pragma once
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

class FileUtils
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
	static void WriteVec(std::ostream& ostream, glm::vec3& vec3)
	{
		ostream.write(reinterpret_cast<const char*>(&vec3.x), sizeof(float) * 3);
	}
	static glm::vec3 ReadVec(std::istream& istream, glm::vec3& value)
	{
		istream.read(reinterpret_cast<char*>(&value), sizeof(float) * 3);
		return value;
	}
	static void WriteFloat(std::ostream& ostream, float& value)
	{
		ostream.write(reinterpret_cast<const char*>(&value), sizeof(float));
	}
	static float ReadFloat(std::istream& istream, float& value)
	{
		istream.read(reinterpret_cast<char*>(&value), sizeof(float));
		return value;
	}
	static void WriteInt(std::ostream& ostream, int value)
	{
		ostream.write(reinterpret_cast<const char*>(&value), sizeof(int));
	}
	static int ReadInt(std::istream& istream, int& value)
	{
		istream.read(reinterpret_cast<char*>(&value), sizeof(int));
		return value;
	}
	static void WriteUInt(std::ostream& ostream, unsigned int value)
	{
		ostream.write(reinterpret_cast<const char*>(&value), sizeof(unsigned int));
	}
	static int ReadUInt(std::istream& istream, unsigned int& value)
	{
		istream.read(reinterpret_cast<char*>(&value), sizeof(unsigned int));
		return value;
	}
	static void WriteString(std::ostream& ostream, std::string value)
	{
		unsigned int stringLength = (unsigned int)value.length();
		char* stringC = new char[stringLength+1]();
		strcpy_s(stringC, sizeof(char) * (stringLength+1), value.c_str());
		ostream.write(reinterpret_cast<const char*>(&stringLength), sizeof(unsigned int));
		ostream.write(stringC, sizeof(char) * stringLength);
		delete[] stringC;
	}
	static std::string ReadString(std::istream& istream, std::string& value)
	{
		unsigned int stringLength;
		istream.read(reinterpret_cast<char*>(&stringLength), sizeof(unsigned int));
		char* stringC = new char[stringLength+1]();
		istream.read(stringC, sizeof(char) * stringLength);
		std::string newString;
		value = stringC;
		newString = value;
		delete[] stringC;
		return newString;
	}
};