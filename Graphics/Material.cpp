#include "Material.h"
#include "TextureManager.h"
#include "ShaderManager.h"
#include "MaterialManager.h"
#include <fstream>

void Material::DrawGUI()
{
	// The ImGui begin headers for this are at the end of the Material Manager Draw GUI function.. for laziness..
	if (isInGameData)
		ImGui::BeginDisabled();

	string newName = name;
	if (ImGui::InputText("Name", &newName, ImGuiInputTextFlags_EnterReturnsTrue))
	{
		// Material Manager change name
		MaterialManager::RemoveMaterial(name);
		MaterialManager::PushMaterial(newName, this);
		// change name
		name = newName;
	}

	if (isInGameData)
		ImGui::EndDisabled();

	// Shader
	string shaderStr = "Static Shader";
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

	shaderStr = "Skinned Shader";
	if (ImGui::BeginCombo(shaderStr.c_str(), shaderSkinned != nullptr ? shaderSkinned->name.c_str() : "NULL"))
	{
		for (auto s : *ShaderManager::ShaderPrograms())
		{
			const bool is_selected = (s.second == shaderSkinned);
			if (ImGui::Selectable(s.first.c_str(), is_selected))
				shaderSkinned = ShaderManager::GetShaderProgram(s.first);

			// Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
			if (is_selected)
				ImGui::SetItemDefaultFocus();
		}
		ImGui::EndCombo();
	}

	ImGui::Checkbox("Is PBR", &isPBR);

	if (!isPBR)
	{
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
	}
	else
	{
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
		string aoMapStr = "PBR AO##";
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
		// Emissive
		string emissiveMapStr = "PBR Emissive##";
		if (ImGui::BeginCombo(emissiveMapStr.c_str(), emissiveMapName.c_str()))
		{
			for (auto t : *TextureManager::Textures())
			{
				const bool is_selected = (t.second == aoMap);
				if (ImGui::Selectable(t.first.c_str(), is_selected))
				{
					emissiveMap = TextureManager::GetTexture(t.first);
					emissiveMapName = t.first;
				}

				// Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
				if (is_selected)
					ImGui::SetItemDefaultFocus();
			}
			ImGui::EndCombo();
		}
	}
	if (ImGui::BeginCombo("Blend Mode", Material::blendModeStrings[(int)blendMode].c_str()))
	{
		for (int i = 0; i < (int)Material::BlendMode::Count; i++)
		{
			bool selected = false;
			if ((int)blendMode == i) selected = true;
			if (ImGui::Selectable(Material::blendModeStrings[i].c_str(), selected))
			{
				blendMode = (Material::BlendMode)i;
			}
		}
	}
	ImGui::Spacing();
	if (!isInGameData) ImGui::BeginDisabled();
	if (ImGui::Button("Write changes to Game Data"))
		SaveToFile();
	if (!isInGameData) ImGui::EndDisabled();
}

void Material::ReferenceTextures()
{
	if (!isPBR)
	{
		if (mapKdName != "") TextureManager::ReferenceTexture(mapKdName);
		if (mapKsName != "") TextureManager::ReferenceTexture(mapKsName);
		if (mapBumpName != "") TextureManager::ReferenceTexture(mapBumpName);
	}
	else
	{
		if (albedoMapName != "") TextureManager::ReferenceTexture(albedoMapName);
		if (normalMapName != "") TextureManager::ReferenceTexture(normalMapName);
		if (metallicMapName != "") TextureManager::ReferenceTexture(metallicMapName);
		if (roughnessMapName != "") TextureManager::ReferenceTexture(roughnessMapName);
		if (aoMapName != "") TextureManager::ReferenceTexture(aoMapName);
		if (emissiveMapName != "") TextureManager::ReferenceTexture(emissiveMapName);
	}
}

void Material::UnreferenceTextures()
{
	if (!isPBR)
	{
		if (mapKdName != "") TextureManager::UnreferenceTexture(mapKdName);
		if (mapKsName != "") TextureManager::UnreferenceTexture(mapKsName);
		if (mapBumpName != "") TextureManager::UnreferenceTexture(mapBumpName);
	}
	else
	{
		if (albedoMapName != "") TextureManager::UnreferenceTexture(albedoMapName);
		if (normalMapName != "") TextureManager::UnreferenceTexture(normalMapName);
		if (metallicMapName != "") TextureManager::UnreferenceTexture(metallicMapName);
		if (roughnessMapName != "") TextureManager::UnreferenceTexture(roughnessMapName);
		if (aoMapName != "") TextureManager::UnreferenceTexture(aoMapName);
		if (emissiveMapName != "") TextureManager::UnreferenceTexture(emissiveMapName);
	}
}

void Material::LoadTextures()
{
	if (!isPBR)
	{
		if (mapKdName != "") mapKd = TextureManager::GetTexture(mapKdName);
		if (mapKsName != "") mapKs = TextureManager::GetTexture(mapKsName);
		if (mapBumpName != "") mapBump = TextureManager::GetTexture(mapBumpName);
	}
	else
	{
		if (albedoMapName != "") albedoMap = TextureManager::GetTexture(albedoMapName);
		if (normalMapName != "") normalMap = TextureManager::GetTexture(normalMapName);
		if (metallicMapName != "") metallicMap = TextureManager::GetTexture(metallicMapName);
		if (roughnessMapName != "") roughnessMap = TextureManager::GetTexture(roughnessMapName);
		if (aoMapName != "") aoMap = TextureManager::GetTexture(aoMapName);
		if (emissiveMapName != "") emissiveMap = TextureManager::GetTexture(emissiveMapName);
	}
	loaded = true;
}

void Material::Reference()
{
	references++;
	ReferenceTextures();
}

void Material::Unreference()
{
	references--;
	UnreferenceTextures();
}

void Material::SaveToFile()
{
	ordered_json output = *this;
	WriteJSONToDisk(filePath, output);
}

void to_json(nlohmann::ordered_json& j, const Material& mat)
{
	j["type"] = "material";
	j["version"] = 1;
	
	j["shader"] = mat.shader ? mat.shader->name : "NULL";
	j["shaderSkinned"] = mat.shaderSkinned ? mat.shaderSkinned->name : "NULL";

	j["isPBR"] = mat.isPBR;
	
	j["Ka"] = mat.Ka;
	j["Kd"] = mat.Kd;
	j["Ks"] = mat.Ks;
	j["Ns"] = mat.specularPower;
	j["map_Kd"] = mat.mapKdName;
	j["map_Ks"] = mat.mapKsName;
	j["bump"] = mat.mapBumpName;
	
	j["albedoMap"] = mat.albedoMapName;
	j["normalMap"] = mat.normalMapName;
	j["metallicMap"] = mat.metallicMapName;
	j["roughnessMap"] = mat.roughnessMapName;
	j["aoMap"] = mat.aoMapName;
	j["emissiveMap"] = mat.emissiveMapName;

	if (mat.blendMode != Material::BlendMode::Opaque)
		j["blendMode"] = mat.blendMode;
}
void from_json(const nlohmann::ordered_json& j, Material& mat)
{
	if (j.contains("shader"))			j.at("shader").get_to(mat.shaderName);
	if (j.contains("shaderSkinned"))	j.at("shaderSkinned").get_to(mat.shaderSkinnedName);

	if (j.contains("isPBR"))			j.at("isPBR").get_to(mat.isPBR);

	if (j.contains("Ka"))				j.at("Ka").get_to(mat.Ka);
	if (j.contains("Kd"))				j.at("Kd").get_to(mat.Kd);
	if (j.contains("Ks"))				j.at("Ks").get_to(mat.Ks);
	if (j.contains("Ns"))				j.at("Ns").get_to(mat.specularPower);

	if (j.contains("map_Kd"))			j.at("map_Kd").get_to(mat.mapKdName);
	if (j.contains("map_Ks"))			j.at("map_Ks").get_to(mat.mapKsName);
	if (j.contains("bump"))				j.at("bump").get_to(mat.mapBumpName);

	if (j.contains("albedoMap"))		j.at("albedoMap").get_to(mat.albedoMapName);
	if (j.contains("normalMap"))		j.at("normalMap").get_to(mat.normalMapName);
	if (j.contains("metallicMap"))		j.at("metallicMap").get_to(mat.metallicMapName);
	if (j.contains("roughnessMap"))		j.at("roughnessMap").get_to(mat.roughnessMapName);
	if (j.contains("aoMap"))			j.at("aoMap").get_to(mat.aoMapName);
	if (j.contains("emissiveMap"))		j.at("emissiveMap").get_to(mat.emissiveMapName);

	if (j.contains("blendMode"))		j.at("blendMode").get_to(mat.blendMode);

	if (mat.shaderName != "") mat.shader = ShaderManager::GetShaderProgram(mat.shaderName);
	if (mat.shaderSkinnedName != "") mat.shaderSkinned = ShaderManager::GetShaderProgram(mat.shaderSkinnedName);
}

string Material::blendModeStrings[3] = {
		"Opaque",
		"Opaque with Alpha Cutoff",
		"Transparent"
};
