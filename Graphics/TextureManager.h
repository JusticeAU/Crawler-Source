#pragma once
#include <string>
#include <map>
#include "Texture.h"
#include <vector>
#include "GraphicsQuality.h"

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
	static bool DrawGUITextureSelector(const string& label, Texture** TexturePtr = nullptr, string* stringPtr = nullptr);
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
	void CreateTextureFromFile(const char* filename, bool inferChannelsFromName = false);
	int InferChannelsFromName(string name);
	
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

	GraphicsQuality::Quality m_quality = GraphicsQuality::Quality::High;
};

