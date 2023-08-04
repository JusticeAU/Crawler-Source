#include "MaterialManager.h"
#include "TextureManager.h"
#include "ShaderManager.h"
#include "LogUtils.h"
#include <filesystem>
#include <fstream>
#include <sstream>
#include "serialisation.h"

using std::vector;
namespace fs = std::filesystem;

MaterialManager::~MaterialManager()
{
	for (auto material : materials)
		delete material.second;
}

void MaterialManager::Init()
{
	if (!s_instance) s_instance = new MaterialManager();
	else LogUtils::Log("Tried to Init MaterialManager when it was already initialised");
}

Material* MaterialManager::GetMaterial(string name)
{
	auto matIt = s_instance->materials.find(name);
	if (matIt == s_instance->materials.end())
		return nullptr;
	else
		return matIt->second;
}

void MaterialManager::DrawGUI()
{
	ImGui::SetNextWindowPos({ 800, 0 }, ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowSize({ 400, 900 }, ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowCollapsed(true, ImGuiCond_FirstUseEver);
	ImGui::Begin("Materials", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize);
	
	if (ImGui::Button("New"))
	{
		// Write a mtl file to disk and then load it
		Material* material = new Material();
		s_instance->materials.emplace(s_instance->newFileName + s_instance->fileExtension, material);
		material->filePath = s_instance->newFileName + s_instance->fileExtension;
		material->name = s_instance->newFileName + s_instance->fileExtension;
		material->SaveToFile();
	}
	ImGui::SameLine();
	string newName = s_instance->newFileName;
	if (ImGui::InputText(s_instance->fileExtension.c_str(), &newName))
	{
		s_instance->newFileName = newName;
	}


	ImGui::BeginDisabled();
	int materialCount = (int)s_instance->materials.size();
	ImGui::DragInt("Material Count", &materialCount);
	ImGui::EndDisabled();
	for (auto m : s_instance->materials)
	{
		if (ImGui::Selectable(m.first.c_str()))
			MaterialSelected = m.second;
	}

	ImGui::End();
}

void MaterialManager::RemoveMaterial(string name)
{
	auto old = s_instance->materials.find(name);
	if (old != s_instance->materials.end())
	{
		s_instance->materials.erase(old);
	}
}

MaterialManager::MaterialManager()
{
	materials.emplace("_null", nullptr);
}

void MaterialManager::LoadFromFile(const char* filename)
{
	Material* material = new Material();
	material->name = filename;
	material->filePath = filename;
	
	auto input = ReadJSONFromDisk(filename);
	input.get_to<Material>(*material);

	materials.emplace(filename, material);
}
void MaterialManager::LoadAllFiles(string folder)
{
	LogUtils::Log("Loading Materials");
	for (auto d : fs::recursive_directory_iterator(folder))
	{
		if (d.path().extension() == ".material")
		{
			string output = "Loading: " + d.path().generic_string();
			LogUtils::Log(output.c_str());
			s_instance->LoadFromFile(d.path().generic_string().c_str());
		}

	}
}

MaterialManager* MaterialManager::s_instance = nullptr;
Material* MaterialManager::MaterialSelected = nullptr;