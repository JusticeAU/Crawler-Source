#include "TextureManager.h"
#include "Graphics.h"
#include <filesystem>
#include "LogUtils.h"

using std::vector;
namespace fs = std::filesystem;

TextureManager::TextureManager()
{
	LoadAllFiles();
}

void TextureManager::Init()
{
	if (!s_instance) s_instance = new TextureManager();
	else LogUtils::Log("Tried to Init MeshManager when it was already initilised");
}

Texture* TextureManager::GetTexture(string name)
{
	auto meshIt = s_instance->textures.find(name);
	if (meshIt == s_instance->textures.end())
		return nullptr;
	else
		return meshIt->second;
}

void TextureManager::DrawGUI()
{
	ImGui::Begin("Texture Manager");
	ImGui::BeginDisabled();
	int meshCount = s_instance->textures.size();
	ImGui::DragInt("Texture Count", &meshCount);
	for (auto m : s_instance->textures)
	{
		ImGui::Text(m.first.c_str());
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

void TextureManager::LoadAllFiles()
{
	LogUtils::Log("Loading Textures");
	for (auto d : fs::recursive_directory_iterator("models"))
	{
		if (d.path().extension() == ".tga" || d.path().extension() == ".png")
		{
			string output = "Loading: " + d.path().generic_string();
			LogUtils::Log(output.c_str());
			LoadFromFile(d.path().generic_string().c_str());
		}

	}
}

TextureManager* TextureManager::s_instance = nullptr;