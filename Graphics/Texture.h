#pragma once
#include "Graphics.h"
#include <string>

using std::string;

class Texture
{
public:
	void LoadFromFile(string filename);
	void Bind(unsigned int slot);
protected:
	GLuint texID;
};

