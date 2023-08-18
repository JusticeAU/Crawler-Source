#include "TextureManager.h"
#include "Graphics.h"
#include <filesystem>
#include "LogUtils.h"
#include "FrameBuffer.h"
#include "StringUtils.h"

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
		return nullptr;
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
	ImGui::Begin("Textures", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize);
	ImGui::BeginDisabled();
	int texCount = (int)s_instance->textures.size();
	ImGui::DragInt("Texture Count", &texCount);
	ImGui::EndDisabled();
	for (auto t : s_instance->textures)
	{
		if (t.second != nullptr && !t.second->loaded)
			ImGui::BeginDisabled();
		if (ImGui::Selectable(t.first.c_str()))
		{
			s_instance->selectedTexture = t.second;
			s_instance->selectedTextureWindowOpen = true;
		}
		if (t.second != nullptr && !t.second->loaded)
			ImGui::EndDisabled();
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

void TextureManager::CreateTextureFromFile(const char* filename)
{
	Texture* texture = new Texture(filename);
	textures.emplace(StringUtils::ToLower(filename), texture);
}

void TextureManager::PreloadAllFiles()
{
	for (auto& texture : s_instance->textures)
	{
		if (texture.second != nullptr && !texture.second->loaded)
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

void TextureManager::FindAllFiles(string folder)
{
	LogUtils::Log("Loading Textures");
	for (auto d : fs::recursive_directory_iterator(folder))
	{
		if (d.path().extension() == ".tga" || d.path().extension() == ".png" || d.path().extension() == ".jpg" || d.path().extension() == ".jpeg" || d.path().extension() == ".bmp")
		{
			s_instance->CreateTextureFromFile(d.path().generic_string().c_str());
		}

	}
}

TextureManager* TextureManager::s_instance = nullptr;