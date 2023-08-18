#pragma once
#include "Graphics.h"
#include <string>

using std::string;

class Texture
{
public:
	Texture();
	Texture(string filename);
	~Texture();

	Texture(Texture const& other) = delete;
	Texture& operator=(Texture const& other) = delete;

	void LoadFromFile(string filename);
	void Load();
	void Bind(unsigned int slot);

	/*void CreateSSAOColourBuffer();
	void ResizeSSAColourBuffer();*/

	void CreateSSAONoiseTexture(glm::vec3* noiseTexData);

public:
	bool loaded = false;
	string name;
	GLuint texID;
};

