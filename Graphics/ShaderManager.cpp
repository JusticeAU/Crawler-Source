#include "ShaderManager.h"
#include "Graphics.h"
#include <filesystem>
#include "LogUtils.h"

using std::vector;
namespace fs = std::filesystem;

ShaderManager::ShaderManager()
{
	shaderPrograms.emplace("_null", nullptr);
	LoadAllFiles();
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
	// right now shaderes are hardcoded to be a vertex and fragment shader, with assumed matching names.
	ShaderProgram* SHAD = new ShaderProgram();
	SHAD->LoadFromFiles(filename + ".VERT", filename + ".FRAG");
	shaderPrograms.emplace(filename, SHAD);

}

void ShaderManager::LoadAllFiles()
{
	LogUtils::Log("Loading Shaders");
	string postProcessFolder = "postProcess";
	for (auto d : fs::recursive_directory_iterator("shaders"))
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
	for (auto shaderprogram : shaderPrograms)
	{
		if(shaderprogram.second)
			shaderprogram.second->Reload();
	}
}

ShaderManager* ShaderManager::s_instance = nullptr;