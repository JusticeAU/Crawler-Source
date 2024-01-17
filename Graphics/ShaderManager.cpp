#include "ShaderManager.h"
#include "Graphics.h"
#include <filesystem>
#include "LogUtils.h"
#include "FileUtils.h"
#include "StringUtils.h"

using std::vector;
namespace fs = std::filesystem;

ShaderManager::ShaderManager()
{
	shaderPrograms.emplace("_null", nullptr);
	LoadAllFiles();
	//ShaderProgram* particleSystem = new ShaderProgram();
	//particleSystem->LoadParticleSystemShader();
	//shaderPrograms.emplace("ParticleSystem", particleSystem);

	//particleSystem = new ShaderProgram();
	//particleSystem->LoadDustParticleSystemShader();
	//shaderPrograms.emplace("DustParticleSystem", particleSystem);
}

void ShaderManager::Init()
{
	if (!s_instance) s_instance = new ShaderManager();
	else LogUtils::Log("Tried to Init ShaderManager when it was already initilised");
}

ShaderProgram* ShaderManager::GetShaderProgram(string name)
{
	auto meshIt = s_instance->shaderPrograms.find(name);
	if (meshIt == s_instance->shaderPrograms.end())
		return nullptr;
	else
		return meshIt->second;
}

void ShaderManager::DrawGUI()
{
	ImGui::SetNextWindowPos({ 800, 20 }, ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowSize({ 400, 880 }, ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowCollapsed(true, ImGuiCond_FirstUseEver);
	ImGui::Begin("Shaders", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize);
	if (ImGui::Button("Reload"))
		s_instance->RecompileAllShaderPrograms();
	ImGui::BeginDisabled();
	int shaderCount = (int)s_instance->shaderPrograms.size();
	ImGui::DragInt("Shader Count", &shaderCount);
	for (auto s : s_instance->shaderPrograms)
	{
		ImGui::Text(s.first.c_str());
	}
	ImGui::EndDisabled();
	ImGui::End();
}


void ShaderManager::LoadFromFile(string filename)
{
	// Read the vert and frag shader from the file
 	string SHAD = FileUtils::LoadFileAsString(filename + ".SHAD");
	int count = 0;
	string* SHADs = StringUtils::Split(SHAD, "\n", &count);
	string subfolder = filename.substr(0, filename.find_last_of('/') + 1);

	// Build the shader from the serilised names that are assumed to be paths relative to the current sub folder
	ShaderProgram* shader = new ShaderProgram();
	if (count < 4)
		shader->LoadFromFiles(subfolder + SHADs[0], subfolder + SHADs[1]);
	else
		shader->LoadFromFiles(subfolder + SHADs[0], subfolder + SHADs[1], subfolder + SHADs[2]);
	shader->name = filename;
	shaderPrograms.emplace(filename, shader);
	delete[] SHADs;
}

void ShaderManager::LoadAllFiles()
{
	LogUtils::Log("Loading Shaders");
	string postProcessFolder = "postProcess";
	for (auto d : fs::recursive_directory_iterator("engine/shader"))
	{
		if (d.path().extension() == ".SHAD")
		{
			string output = "Loading: " + d.path().generic_string();
			LogUtils::Log(output.c_str());
			string shaderPath = d.path().parent_path().generic_string() + "/" + d.path().stem().generic_string();
			LoadFromFile(shaderPath);
			
			// check if its a post process shader and if so, add to that list.
			if (shaderPath.find(postProcessFolder) != std::string::npos)
				m_postProcessShaderNames.push_back(shaderPath);
		}
	}
}

void ShaderManager::RecompileAllShaderPrograms()
{
	for (auto shaderprogram : s_instance->shaderPrograms)
	{
		if(shaderprogram.second)
			shaderprogram.second->Reload();
	}
}

ShaderManager* ShaderManager::s_instance = nullptr;