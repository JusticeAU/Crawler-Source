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
	for (auto m : s_instance->materials)
	{
		ImGui::Text(m.first.c_str());
	}
	ImGui::EndDisabled();
	ImGui::End();
}

MaterialManager::MaterialManager()
{
	materials.emplace("_null", nullptr);
	LoadAllFiles();
}

void MaterialManager::LoadFromFile(const char* filename)
{
	Material* material = new Material();
	material->name = filename;
	material->filePath = filename;
	

	auto input = ReadJSONFromDisk(filename);
	std::cout << input.size();
	input.at("shader").get_to(material->shaderName);
	material->shader = ShaderManager::GetShaderProgram(material->shaderName);
	
	input.at("Ka").get_to(material->Ka);
	input.at("Kd").get_to(material->Kd);
	input.at("Ks").get_to(material->Ks);
	input.at("Ns").get_to(material->specularPower);

	input.at("map_Kd").get_to(material->mapKdName);
	material->mapKd = TextureManager::GetTexture(material->mapKdName);
	input.at("map_Ks").get_to(material->mapKsName);
	material->mapKs = TextureManager::GetTexture(material->mapKsName);
	input.at("bump").get_to(material->mapBumpName);
	material->mapBump = TextureManager::GetTexture(material->mapBumpName);
	
	input.at("albedoMap").get_to(material->albedoMapName);
	material->albedoMap = TextureManager::GetTexture(material->albedoMapName);
	input.at("normalMap").get_to(material->normalMapName);
	material->normalMap = TextureManager::GetTexture(material->normalMapName);
	input.at("metallicMap").get_to(material->metallicMapName);
	material->metallicMap = TextureManager::GetTexture(material->metallicMapName);
	input.at("roughnessMap").get_to(material->roughnessMapName);
	material->roughnessMap = TextureManager::GetTexture(material->roughnessMapName);
	input.at("aoMap").get_to(material->aoMapName);
	material->aoMap = TextureManager::GetTexture(material->aoMapName);

	materials.emplace(filename, material);
}

void MaterialManager::LoadAllFiles()
{
	LogUtils::Log("Loading Materials");
	for (auto d : fs::recursive_directory_iterator("models"))
	{
		if (d.path().extension() == ".material")
		{
			string output = "Loading: " + d.path().generic_string();
			LogUtils::Log(output.c_str());
			LoadFromFile(d.path().generic_string().c_str());
		}

	}
}

MaterialManager* MaterialManager::s_instance = nullptr;