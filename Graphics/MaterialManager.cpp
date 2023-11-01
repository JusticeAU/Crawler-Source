#include "MaterialManager.h"
#include "TextureManager.h"
#include "ShaderManager.h"
#include "LogUtils.h"
#include <filesystem>
#include <fstream>
#include <sstream>
#include "serialisation.h"
#include "StringUtils.h"

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
	string nameLower = StringUtils::ToLower(name);
	auto matIt = s_instance->materials.find(nameLower);
	if (matIt == s_instance->materials.end())
	{
		LogUtils::Log("Missing Material: " + nameLower);
		return nullptr;
	}
	else
	{
		if (!matIt->second->loaded) matIt->second->LoadTextures();

		return matIt->second;
	}
}

void MaterialManager::DrawGUI()
{
	ImGui::SetNextWindowPos({ 800, 0 }, ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowSize({ 400, 900 }, ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowCollapsed(true, ImGuiCond_FirstUseEver);
	ImGui::Begin("Materials", nullptr);
	if (ImGui::Button("Scan For Unreferenced Materials"))
	{
		s_instance->Audit_ScanFolderForMaterialReferences("crawler");
		s_instance->Audit_ScanFolderForMaterialReferences("engine");
		s_instance->Audit_ListAllUnreferencedMaterials();
	}
	if (ImGui::Button("New"))
	{
		// Write a material file to disk and then load it
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
		string name = m.first;
		//if (m.second && !m.second->Audit_referenced) ImGui::BeginDisabled();
		if (ImGui::Selectable(name.c_str()))
		{
			s_instance->selectedMaterial = m.second;
			s_instance->selectedMaterialWindowOpen = true;
		}
		//if (m.second && !m.second->Audit_referenced) ImGui::EndDisabled();
	}

	ImGui::End();

	if (s_instance->selectedMaterialWindowOpen)
	{
		ImGui::SetNextWindowPos({ 550, 10 }, ImGuiCond_FirstUseEver);
		ImGui::SetNextWindowSize({ 600, 400 }, ImGuiCond_FirstUseEver);
		ImGui::Begin("Material", &s_instance->selectedMaterialWindowOpen);
		s_instance->selectedMaterial->DrawGUI();
		ImGui::End();
	}
}

void MaterialManager::RemoveMaterial(string name)
{
	auto old = s_instance->materials.find(name);
	if (old != s_instance->materials.end())
	{
		s_instance->materials.erase(old);
	}
}

void MaterialManager::Audit_ScanFolderForMaterialReferences(string folder)
{
	LogUtils::Log("Scanning for objects in: " + folder);
	for (auto d : fs::recursive_directory_iterator(folder))
	{
		if (d.path().extension() == ".object")
		{
			ordered_json object = ReadJSONFromDisk(d.path().generic_string());
			PerformFunctionOnMatchingKeysRecursive(object, "materials", [](ordered_json j) {
				for (auto& mat : j)
				{
					string materialName = mat;
					MaterialManager::s_instance->Audit_ReferenceMaterial(materialName);
				}
			});
		}
	}
}

void MaterialManager::Audit_ReferenceMaterial(string name)
{
	string nameLower = StringUtils::ToLower(name);
	auto matIt = s_instance->materials.find(nameLower);
	if (matIt == s_instance->materials.end()) s_instance->Audit_missingMaterials.push_back(nameLower);
	else if (matIt->second) matIt->second->Audit_referenced = true;
	return;
}

void MaterialManager::Audit_ListAllUnreferencedMaterials()
{
	LogUtils::Log("Unreferenced Materials");
	for (auto& material : s_instance->materials)
	{
		if (material.second && !material.second->Audit_referenced)
			LogUtils::Log(material.first);
	}
	LogUtils::Log("Missing Materials");
	for (auto& material : s_instance->Audit_missingMaterials)
	{
		LogUtils::Log(material);
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
	material->isInGameData = true;
	
	auto input = ReadJSONFromDisk(filename);
	input.get_to<Material>(*material);
	materials.emplace(StringUtils::ToLower(filename), material);
}
void MaterialManager::FindAllFiles(string folder)
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

void MaterialManager::PreloadAllFiles()
{
	for (auto& material : s_instance->materials)
	{
		if (material.second != nullptr && !material.second->loaded)
			material.second->LoadTextures();
	}
}

MaterialManager* MaterialManager::s_instance = nullptr;