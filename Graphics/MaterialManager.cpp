#include "MaterialManager.h"
#include <filesystem>
#include "LogUtils.h"
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
	ImGui::BeginDisabled();
	int materialCount = s_instance->materials.size();
	ImGui::DragInt("Texture Count", &materialCount);
	for (auto m : s_instance->materials)
	{
		ImGui::Text(m.first.c_str());
	}
	ImGui::EndDisabled();
	ImGui::End();
}

MaterialManager::MaterialManager()
{
	LoadAllFiles();
}

void MaterialManager::LoadFromFile(const char* filename)
{
	Material* material = new Material();
	std::fstream file(filename, std::ios::in);
	std::string line;
	std::string header;
	char buffer[256];
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