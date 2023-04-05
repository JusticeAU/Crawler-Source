#include "MaterialManager.h"
#include "TextureManager.h"
#include "LogUtils.h"
#include <filesystem>
#include <fstream>
#include <sstream>

using std::vector;
namespace fs = std::filesystem;

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
	ImGui::Begin("Material Manager");
	
	if (ImGui::Button("New"))
	{
		// Write a mtl file to disk and then load it
		Material* material = new Material();
		s_instance->materials.emplace(s_instance->newFileName, material);
		material->filePath = s_instance->newFileName;
		material->SaveToFile();
	}

	string newName = s_instance->newFileName;
	if (ImGui::InputText("Filename", &newName))
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
	material->filePath = filename;
	
	std::fstream file(filename, std::ios::in);
	std::string line;
	std::string header;
	char buffer[256];

	// get the path part of the fileName for use with relative paths for maps later
	std::string directory(filename);
	int index = (int)directory.rfind('/');
	if (index != -1)
		directory = directory.substr(0, index + 1);

	while (!file.eof())
	{
		file.getline(buffer, 256);
		line = buffer;
		std::stringstream ss(line,
			std::stringstream::in | std::stringstream::out);
		if (line.find("Ka") == 0)
			ss >> header >> material->Ka.x >> material->Ka.y >> material->Ka.z;
		else if (line.find("Ks") == 0)
			ss >> header >> material->Ks.x >> material->Ks.y >> material->Ks.z;
		else if (line.find("Kd") == 0)
			ss >> header >> material->Kd.x >> material->Kd.y >> material->Kd.z;
		else if (line.find("Ns") == 0)
			ss >> header >> material->specularPower;
		else if (line.find("map_Kd") == 0)
		{
			std::string mapFileName;
			ss >> header >> mapFileName;
			material->mapKd = TextureManager::GetTexture(directory + mapFileName);
			material->mapKdName = directory + mapFileName;
		}
		else if (line.find("map_Ks") == 0)
		{
			std::string mapFileName;
			ss >> header >> mapFileName;
			material->mapKs = TextureManager::GetTexture(directory + mapFileName);
			material->mapKsName = directory + mapFileName;
		}
		else if (line.find("bump") == 0)
		{
			std::string mapFileName;
			ss >> header >> mapFileName;
			material->mapBump = TextureManager::GetTexture(directory + mapFileName);
			material->mapBumpName = directory + mapFileName;
		}
	}
	materials.emplace(filename, material);
}

void MaterialManager::LoadAllFiles()
{
	LogUtils::Log("Loading Materials");
	for (auto d : fs::recursive_directory_iterator("models"))
	{
		if (d.path().extension() == ".mtl")
		{
			string output = "Loading: " + d.path().generic_string();
			LogUtils::Log(output.c_str());
			LoadFromFile(d.path().generic_string().c_str());
		}

	}
}

MaterialManager* MaterialManager::s_instance = nullptr;