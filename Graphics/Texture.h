#pragma once
#include "Graphics.h"
#include <string>

using std::string;

class Texture
{
public:
	Texture();
	~Texture();

	Texture(Texture const& other) = delete;
	Texture& operator=(Texture const& other) = delete;

	void LoadFromFile(string filename);
	void Bind(unsigned int slot);
protected:
	GLuint texID;
};

