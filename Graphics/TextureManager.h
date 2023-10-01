#pragma once
#include <string>
#include <map>
#include "Texture.h"
#include <vector>

using std::string;
using std::map;

class FrameBuffer;

// Singleton class to handle all texture resources. Call Init() once and then access it through static functions.
class TextureManager
{
public:
	static void Init();

	static Texture* GetTexture(string name);

	static FrameBuffer* GetFrameBuffer(string name);

	static void DrawGUI();
	static void DrawTexturePreview();
	static void SetPreviewTexture(string textureName);
	static TextureManager* s_instance;
	static void AddTexture(Texture* texture) { s_instance->textures.emplace(texture->name, texture); }
	
	static const map<string, Texture*>* Textures() { return &s_instance->textures; }
	static const map<string, FrameBuffer*>* FrameBuffers() { return &s_instance->frameBuffers; }

	static void FindAllFiles(string folder);
	static void PreloadAllFiles();
	static void PreloadNextFile();
	static bool IsPreloadComplete() { return s_instance->m_preloadComplete; };
	static float GetPreloadPercentage();
	static void PreloadAllFilesContaining(string contains);
	void CreateTextureFromFile(const char* filename);
	
	void AddFrameBuffer(const char* name, FrameBuffer* fb);
	void RemoveFrameBuffer(const char* name);

	static void RefreshFrameBuffers();

	void Audit_ScanFolderForTextureReferences(string folder);
	void Audit_ReferenceTexture(string name);
	void Audit_ListAllUnreferencedTextures();
	std::vector<string> Audit_missingTextures;

	void MakeAllTGAsRLE(string folder);

protected:
	TextureManager();
	map<string, Texture*> textures;
	map<string, FrameBuffer*> frameBuffers;
	bool m_preloadComplete = false;
	int m_preloadCount = 0;

	Texture* selectedTexture = nullptr;
	bool selectedTextureWindowOpen = false;

};

