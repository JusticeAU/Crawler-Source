#include "TextureManager.h"
#include "Graphics.h"
#include <filesystem>
#include "LogUtils.h"
#include "FrameBuffer.h"
#include "StringUtils.h"
#include "serialisation.h"

using std::vector;
namespace fs = std::filesystem;

TextureManager::TextureManager()
{
	textures.emplace("_null", nullptr);
	frameBuffers.emplace("_null", nullptr);
}

void TextureManager::Init()
{
	if (!s_instance) s_instance = new TextureManager();
	else LogUtils::Log("Tried to Init MeshManager when it was already initilised");
}

Texture* TextureManager::GetTexture(string name)
{
	auto texIt = s_instance->textures.find(StringUtils::ToLower(name));
	if (texIt == s_instance->textures.end())
	{
		LogUtils::Log("Missing Texture: " + name);
		return nullptr;
	}
	else
	{
		if (texIt->second != nullptr && !texIt->second->loaded)
			texIt->second->Load();
		return texIt->second;
	}
}

FrameBuffer* TextureManager::GetFrameBuffer(string name)
{
	auto fbIt = s_instance->frameBuffers.find(name);
	if (fbIt == s_instance->frameBuffers.end())
		return nullptr;
	else
		return fbIt->second;
}

void TextureManager::DrawGUI()
{
	ImGui::SetNextWindowPos({ 800, 40 }, ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowSize({ 400, 860 }, ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowCollapsed(true, ImGuiCond_FirstUseEver);
	ImGui::Begin("Textures", nullptr);
	if (ImGui::Button("Scan for Unreferenced Textures"))
	{
		s_instance->Audit_ScanFolderForTextureReferences("crawler/material");
		s_instance->Audit_ListAllUnreferencedTextures();
	}
	/*if (ImGui::Button("Make All RLE"))
	{
		s_instance->MakeAllTGAsRLE("crawler/texture");
	}*/
	ImGui::BeginDisabled();
	int texCount = (int)s_instance->textures.size();
	ImGui::DragInt("Texture Count", &texCount);
	ImGui::EndDisabled();
	for (auto t : s_instance->textures)
	{
		string name = t.first;
		if (ImGui::Selectable(name.c_str()))
		{
			s_instance->selectedTexture = GetTexture(t.first); // Run it through the getter to ensure its loaded.
			s_instance->selectedTextureWindowOpen = true;
		}
	}
	ImGui::End();

	// buffers
	ImGui::SetNextWindowPos({ 800, 60 }, ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowSize({ 400, 840 }, ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowCollapsed(true, ImGuiCond_FirstUseEver);
	ImGui::Begin("Frame Buffers", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize);
	ImGui::BeginDisabled();
	int fbCount = (int)s_instance->frameBuffers.size();
	ImGui::DragInt("Buffer Count", &fbCount);
	for (auto fb : s_instance->frameBuffers)
	{
		ImGui::Text(fb.first.c_str());
	}
	ImGui::EndDisabled();
	ImGui::End();
}

void TextureManager::DrawTexturePreview()
{
	if (s_instance->selectedTextureWindowOpen)
	{
		ImGui::Begin("Texture preview", &s_instance->selectedTextureWindowOpen);
		ImGui::Image((ImTextureID)(s_instance->selectedTexture->texID), { 1600,900 }, { 0,1 }, { 1,0 });
	}
}

void TextureManager::SetPreviewTexture(string textureName)
{
	s_instance->selectedTexture = GetTexture(textureName);
	if (s_instance->selectedTexture != nullptr)
	{
		s_instance->selectedTextureWindowOpen = true;
	}
}

void TextureManager::CreateTextureFromFile(const char* filename, bool inferChannelsFromName)
{
	Texture* texture = new Texture(filename);
	textures.emplace(StringUtils::ToLower(filename), texture);
	if (inferChannelsFromName) texture->channels = InferChannelsFromName(filename);
	else texture->channels = -1;
}

int TextureManager::InferChannelsFromName(string name)
{
	string mapNames[] = { "_albedo", "_normal", "_metallic", "_roughness", "_ao", "_emissive", };
	int mapChannels[] = { 4, 3, 1, 1, 1, 4 };
	int mapQuantity = 6;
	for (int i = 0; i < mapQuantity; i++)
	{
		size_t index = name.find(mapNames[i]);
		if (index != string::npos) return mapChannels[i];
	}

	return -1; // unable to detect
}

void TextureManager::PreloadAllFiles()
{
	for (auto& texture : s_instance->textures)
	{
		if (texture.second != nullptr && !texture.second->loaded)
		{
			texture.second->Load();
			s_instance->m_preloadCount += 1;
		}
	}
	s_instance->m_preloadComplete = true;
}

void TextureManager::PreloadNextFile()
{
	for (auto& texture : s_instance->textures)
	{
		if (texture.second != nullptr && !texture.second->loaded)
		{
			texture.second->Load();
			s_instance->m_preloadCount += 1;
			return;
		}
	}
	s_instance->m_preloadComplete = true;
}

float TextureManager::GetPreloadPercentage()
{
	return (float)s_instance->m_preloadCount / (float)s_instance->textures.size();
}

void TextureManager::PreloadAllFilesContaining(string contains)
{
	for (auto& texture : s_instance->textures)
	{
		if (texture.second != nullptr && !texture.second->loaded && texture.first.find(contains) != string::npos)
			texture.second->Load();
	}
}

void TextureManager::AddFrameBuffer(const char* name, FrameBuffer* fb)
{
	string texName = "framebuffers/";
	texName += name;

	auto existingFB = frameBuffers.find(name);
	if (existingFB != frameBuffers.end())
	{
		LogUtils::Log("Trying to add existing FrameBuffer - clobbering over it");
		LogUtils::Log(existingFB->first.c_str());
		frameBuffers.erase(existingFB);
		textures.erase(texName);
	}

	frameBuffers.emplace(name, fb);
	textures.emplace(StringUtils::ToLower(texName), fb->GetTexture());
}

void TextureManager::RemoveFrameBuffer(const char* name)
{
	string texName = "framebuffers/";
	texName += name;

	auto existingFB = frameBuffers.find(name);
	if (existingFB != frameBuffers.end())
	{
		frameBuffers.erase(existingFB);
		textures.erase(texName);
	}
}

void TextureManager::RefreshFrameBuffers()
{
	for (auto& fb : s_instance->frameBuffers)
	{
		if (fb.second != nullptr && fb.second->isScreenBuffer())
			fb.second->Resize();
	}
}

void TextureManager::Audit_ScanFolderForTextureReferences(string folder)
{
	LogUtils::Log("Scanning for materials in: " + folder);
	for (auto d : fs::recursive_directory_iterator(folder))
	{
		if (d.path().extension() == ".material")
		{
			ordered_json object = ReadJSONFromDisk(d.path().generic_string());
			if (object.contains("albedoMap")) TextureManager::s_instance->Audit_ReferenceTexture(object["albedoMap"]);
			if(object.contains("normalMap")) TextureManager::s_instance->Audit_ReferenceTexture(object["normalMap"]);
			if(object.contains("metallicMap")) TextureManager::s_instance->Audit_ReferenceTexture(object["metallicMap"]);
			if(object.contains("roughnessMap")) TextureManager::s_instance->Audit_ReferenceTexture(object["roughnessMap"]);
			if (object.contains("aoMap")) TextureManager::s_instance->Audit_ReferenceTexture(object["aoMap"]);
			if (object.contains("emissiveMap")) TextureManager::s_instance->Audit_ReferenceTexture(object["emissiveMap"]);
		}
	}
}

void TextureManager::Audit_ReferenceTexture(string name)
{
	if (name == "") return;
	string nameLower = StringUtils::ToLower(name);
	auto texIt = s_instance->textures.find(nameLower);
	if (texIt == s_instance->textures.end()) s_instance->Audit_missingTextures.push_back(nameLower);
	else if (texIt->second) texIt->second->Audit_referenced = true;
	return;
}

void TextureManager::Audit_ListAllUnreferencedTextures()
{
	LogUtils::Log("Unreferenced Textures");
	for (auto& texture : s_instance->textures)
	{
		if (texture.second && !texture.second->Audit_referenced)
			LogUtils::Log(texture.first);
	}
	LogUtils::Log("Missing Textures");
	for (auto& material : s_instance->Audit_missingTextures)
	{
		LogUtils::Log(material);
	}
}

void TextureManager::MakeAllTGAsRLE(string folder)
{
	for (auto d : fs::recursive_directory_iterator(folder))
	{
		if (d.path().extension() == ".tga" )
		{
			Texture::RewriteTGAwithRLE(d.path().generic_string(), d.path().generic_string());
		}

	}
}

void TextureManager::FindAllFiles(string folder)
{
	LogUtils::Log("Loading Textures");
	for (auto d : fs::recursive_directory_iterator(folder))
	{
		if (d.path().extension() == ".tga" || d.path().extension() == ".png" || d.path().extension() == ".jpg" || d.path().extension() == ".jpeg" || d.path().extension() == ".bmp")
		{
			s_instance->CreateTextureFromFile(d.path().generic_string().c_str(), true);
		}

	}
}

TextureManager* TextureManager::s_instance = nullptr;