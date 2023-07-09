#include "TextureManager.h"
#include "Graphics.h"
#include <filesystem>
#include "LogUtils.h"
#include "FrameBuffer.h"

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
	auto texIt = s_instance->textures.find(name);
	if (texIt == s_instance->textures.end())
		return nullptr;
	else
		return texIt->second;
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
	for (auto t : s_instance->textures)
	{
		ImGui::Text(t.first.c_str());
	}
	ImGui::EndDisabled();
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

void TextureManager::LoadFromFile(const char* filename)
{
	Texture* texture = new Texture();
	texture->LoadFromFile(filename);
	textures.emplace(filename, texture);
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
	textures.emplace(texName, fb->GetTexture());
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

void TextureManager::LoadAllFiles(string folder)
{
	LogUtils::Log("Loading Textures");
	for (auto d : fs::recursive_directory_iterator(folder))
	{
		if (d.path().extension() == ".tga" || d.path().extension() == ".png" || d.path().extension() == ".jpg" || d.path().extension() == ".jpeg" || d.path().extension() == ".bmp")
		{
			string output = "Loading: " + d.path().generic_string();
			LogUtils::Log(output.c_str());
			s_instance->LoadFromFile(d.path().generic_string().c_str());
		}

	}
}

TextureManager* TextureManager::s_instance = nullptr;