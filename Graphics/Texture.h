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

	void CreateSSAONoiseTexture(glm::vec3* noiseTexData);

	static void RewriteTGAwithRLE(string from, string to);

public:
	bool loaded = false;
	string name;
	GLuint texID;

	bool Audit_referenced = false;
};

