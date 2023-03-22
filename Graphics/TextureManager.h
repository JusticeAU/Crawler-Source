#pragma once
#include <string>
#include <map>
#include "Texture.h"

using std::string;
using std::map;

class TextureManager
{
public:
	static void Init();

	static Texture* GetTexture(string name);
	static void DrawGUI();
	static TextureManager* s_instance;
	static const map<string, Texture*>* Textures() { return &s_instance->textures; }
protected:
	TextureManager();
	map<string, Texture*> textures;

	void LoadFromFile(const char* filename);

	void LoadAllFiles();
};

