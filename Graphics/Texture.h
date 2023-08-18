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
	void Reference() { references++; };
	void Unreference() { references--; };
	void Bind(unsigned int slot);

	void CreateSSAONoiseTexture(glm::vec3* noiseTexData);

public:
	bool loaded = false;
	int references = 0;
	string name;
	GLuint texID;
};

