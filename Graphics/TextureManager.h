#pragma once
#include <string>
#include <map>
#include "Texture.h"

using std::string;
using std::map;

class FrameBuffer;


// Singleton class to handle all texture resources. Call Init() once and then access it through static functions.
class TextureManager
{
public:
	static void Init();

	static Texture* GetTexture(string name);
	static void DrawGUI();
	static TextureManager* s_instance;
	static const map<string, Texture*>* Textures() { return &s_instance->textures; }
	void AddFrameBufferTexture(const char* name, FrameBuffer* fb);
protected:
	TextureManager();
	map<string, Texture*> textures;

	void LoadFromFile(const char* filename);
	void LoadAllFiles();
};

