#include "Material.h"
#include "TextureManager.h"
#include "ShaderManager.h"
#include <fstream>
#include "serialisation.h"

void Material::DrawGUI()
{
	// Shader
	string shaderStr = "Shader";
	if (ImGui::BeginCombo(shaderStr.c_str(), shader != nullptr ? shader->name.c_str() : "NULL"))
	{
		for (auto s : *ShaderManager::ShaderPrograms())
		{
			const bool is_selected = (s.second == shader);
			if (ImGui::Selectable(s.first.c_str(), is_selected))
				shader = ShaderManager::GetShaderProgram(s.first);

			// Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
			if (is_selected)
				ImGui::SetItemDefaultFocus();
		}
		ImGui::EndCombo();
	}

	// Ka
	float KaGUI[3] = { Ka.r, Ka.g, Ka.b, };
	if (ImGui::ColorEdit3("Ambient Colour", KaGUI))
		Ka = { KaGUI[0], KaGUI[1], KaGUI[2] };

	// Kd
	float KdGUI[3] = { Kd.r, Kd.g, Kd.b, };
	if (ImGui::ColorEdit3("Diffuse Colour", KdGUI))
		Kd = { KdGUI[0], KdGUI[1], KdGUI[2] };

	// Ks
	float KsGUI[3] = { Ks.r, Ks.g, Ks.b, };
	if (ImGui::ColorEdit3("Specular Colour", KsGUI))
		Ks = { KsGUI[0], KsGUI[1], KsGUI[2] };

	// Specular Power
	ImGui::DragFloat("Specular Power", &specularPower);

	// Diffuse Texture
	string diffuseStr = "Diffuse##";
	if (ImGui::BeginCombo(diffuseStr.c_str(), mapKdName.c_str()))
	{
		for (auto t : *TextureManager::Textures())
		{
			const bool is_selected = (t.second == mapKd);
			if (ImGui::Selectable(t.first.c_str(), is_selected))
			{
				mapKd = TextureManager::GetTexture(t.first);
				mapKdName = t.first;
			}

			// Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
			if (is_selected)
				ImGui::SetItemDefaultFocus();
		}
		ImGui::EndCombo();
	}

	// Specular Texture
	string specularStr = "Specular##";
	if (ImGui::BeginCombo(specularStr.c_str(), mapKsName.c_str()))
	{
		for (auto t : *TextureManager::Textures())
		{
			const bool is_selected = (t.second == mapKs);
			if (ImGui::Selectable(t.first.c_str(), is_selected))
			{
				mapKs = TextureManager::GetTexture(t.first);
				mapKsName = t.first;
			}

			// Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
			if (is_selected)
				ImGui::SetItemDefaultFocus();
		}
		ImGui::EndCombo();
	}

	// Normal Texture
	string normalStr = "Normal##";
	if (ImGui::BeginCombo(normalStr.c_str(), mapBumpName.c_str()))
	{
		for (auto t : *TextureManager::Textures())
		{
			const bool is_selected = (t.second == mapBump);
			if (ImGui::Selectable(t.first.c_str(), is_selected))
			{
				mapBump = TextureManager::GetTexture(t.first);
				mapBumpName = t.first;
			}

			// Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
			if (is_selected)
				ImGui::SetItemDefaultFocus();
		}
		ImGui::EndCombo();
	}

	// PBR Textures
	// Albedo
	string albedoMapStr = "PBR Albedo##";
	if (ImGui::BeginCombo(albedoMapStr.c_str(), albedoMapName.c_str()))
	{
		for (auto t : *TextureManager::Textures())
		{
			const bool is_selected = (t.second == albedoMap);
			if (ImGui::Selectable(t.first.c_str(), is_selected))
			{
				albedoMap = TextureManager::GetTexture(t.first);
				albedoMapName = t.first;
			}

			// Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
			if (is_selected)
				ImGui::SetItemDefaultFocus();
		}
		ImGui::EndCombo();
	}
	// Normal
	string normalMapStr = "PBR Normal##";
	if (ImGui::BeginCombo(normalMapStr.c_str(), normalMapName.c_str()))
	{
		for (auto t : *TextureManager::Textures())
		{
			const bool is_selected = (t.second == normalMap);
			if (ImGui::Selectable(t.first.c_str(), is_selected))
			{
				normalMap = TextureManager::GetTexture(t.first);
				normalMapName = t.first;
			}

			// Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
			if (is_selected)
				ImGui::SetItemDefaultFocus();
		}
		ImGui::EndCombo();
	}
	// Metallic
	string metallicMapStr = "PBR Metallic##";
	if (ImGui::BeginCombo(metallicMapStr.c_str(), metallicMapName.c_str()))
	{
		for (auto t : *TextureManager::Textures())
		{
			const bool is_selected = (t.second == metallicMap);
			if (ImGui::Selectable(t.first.c_str(), is_selected))
			{
				metallicMap = TextureManager::GetTexture(t.first);
				metallicMapName = t.first;
			}

			// Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
			if (is_selected)
				ImGui::SetItemDefaultFocus();
		}
		ImGui::EndCombo();
	}
	// Roughness
	string roughnessMapStr = "PBR Roughness##";
	if (ImGui::BeginCombo(roughnessMapStr.c_str(), roughnessMapName.c_str()))
	{
		for (auto t : *TextureManager::Textures())
		{
			const bool is_selected = (t.second == roughnessMap);
			if (ImGui::Selectable(t.first.c_str(), is_selected))
			{
				roughnessMap = TextureManager::GetTexture(t.first);
				roughnessMapName = t.first;
			}

			// Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
			if (is_selected)
				ImGui::SetItemDefaultFocus();
		}
		ImGui::EndCombo();
	}
	// Ambient Occlusion
	string aoMapStr = "PBR Ambient Occlusion##";
	if (ImGui::BeginCombo(aoMapStr.c_str(), aoMapName.c_str()))
	{
		for (auto t : *TextureManager::Textures())
		{
			const bool is_selected = (t.second == aoMap);
			if (ImGui::Selectable(t.first.c_str(), is_selected))
			{
				aoMap = TextureManager::GetTexture(t.first);
				aoMapName = t.first;
			}

			// Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
			if (is_selected)
				ImGui::SetItemDefaultFocus();
		}
		ImGui::EndCombo();
	}


	if (ImGui::Button("Save"))
		SaveToFile();
}

void Material::SaveToFile()
{
	nlohmann::ordered_json output;
	output["type"] = "material";
	output["version"] = 1;
	
	output["shader"] = shader ? shader->name : "NULL";
	
	output["Ka"] = Ka;
	output["Kd"] = Kd;
	output["Ks"] = Ks;
	output["Ns"] = specularPower;
	output["map_Kd"] = mapKdName;
	output["map_Ks"] = mapKsName;
	output["bump"] = mapBumpName;

	output["albedoMap"] = albedoMapName;
	output["normalMap"] = normalMapName;
	output["metallicMap"] = metallicMapName;
	output["roughnessMap"] = roughnessMapName;
	output["aoMap"] = aoMapName;
	
	WriteJSONToDisk(filePath, output);
}