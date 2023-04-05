#include "Material.h"
#include "TextureManager.h"
#include <fstream>

void Material::DrawGUI()
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

	if (ImGui::Button("Save"))
		SaveToFile();
}

void Material::SaveToFile()
{
	std::fstream file(filePath, std::ios::out);
	file << "Ka " << Ka.x << " " << Ka.z << " " << Ka.z << std::endl;
	file << "Kd " << Kd.x << " " << Kd.z << " " << Kd.z << std::endl;
	file << "Ks " << Ks.x << " " << Ks.z << " " << Ks.z << std::endl;
	file << "Ns " << specularPower << std::endl;

	// need to clean filepaths out of filenames here.
	// Currently .mtl files save the filenames as relative paths to themselves
	int from = (int)mapKdName.find_last_of('/')+1;
	string filename = mapKdName.substr(from, mapKdName.length() - from);
	file << "map_Kd " << filename << std::endl;

	from = (int)mapKsName.find_last_of('/') + 1;
	filename = mapKsName.substr(from, mapKsName.length() - from);
	file << "map_Ks " << filename << std::endl;

	from = (int)mapBumpName.find_last_of('/') + 1;
	filename = mapBumpName.substr(from, mapBumpName.length() - from);
	file << "bump " << filename << std::endl;
	file.close();
}
