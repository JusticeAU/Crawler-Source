#include "DungeonEditor.h"
#include "Object.h"
#include "Input.h"
#include "Camera.h"
#include "Scene.h"
#include <filesystem>
namespace fs = std::filesystem;

#include "LogUtils.h"

Crawl::DungeonEditor::DungeonEditor()
{
	
}

void Crawl::DungeonEditor::DrawGUI()
{
	ImGui::SetNextWindowPos({ 0,0 }, ImGuiCond_Always);
	ImGui::Begin("Dungeon Edit", 0, ImGuiWindowFlags_NoMove);
	ImGui::BeginDisabled();
	ImGui::Text(dungeonFileName.c_str());
	ImGui::EndDisabled();
	
	if(dungeonFilePath == "")
		ImGui::BeginDisabled();

	if (ImGui::Button("Save"))
		Save();

	if (dungeonFilePath == "")
		ImGui::EndDisabled();

	ImGui::SameLine();
	if (ImGui::Button("Save As"))
	{
		didSaveAs = false;
		ImGui::OpenPopup("Save As");
		dungeonFileNameSaveAs = dungeonFileName;
	}

	ImGui::SameLine();
	if (ImGui::Button("Load"))
		ImGui::OpenPopup("Load Dungeon");

	ImGui::BeginDisabled();
	ImGui::DragInt2("Grid Selected", &gridSelected.x);
	ImGui::EndDisabled();

	// Save As prompt
	ImGui::SetNextWindowSize({ 300, 100 });
	ImGui::SetNextWindowPos({ 0,0 }, ImGuiCond_Always);
	if (ImGui::BeginPopupModal("Save As",0, ImGuiWindowFlags_NoCollapse & ImGuiWindowFlags_NoResize & ImGuiWindowFlags_NoMove))
	{
		ImGui::PushID("save_popup");
		ImGui::InputText(extension.c_str(), &dungeonFileNameSaveAs);
		if (ImGui::Button("Save"))
		{
			if (FileUtils::CheckFileExists(GetDungeonFilePath()))
				ImGui::OpenPopup("Overwrite Existing File");
			else
			{
				dungeonFileName = dungeonFileNameSaveAs;
				didSaveAs = true;
				Save();
			}

		}
		ImGui::SameLine();
		if (ImGui::Button("Cancel"))
			ImGui::CloseCurrentPopup();

		ImGui::SetNextWindowSize({ 300, 100 });
		ImGui::SetNextWindowPos({ 0,0 }, ImGuiCond_Always);
		if (ImGui::BeginPopupModal("Overwrite Existing File", 0, ImGuiWindowFlags_NoCollapse & ImGuiWindowFlags_NoResize & ImGuiWindowFlags_NoMove))
		{
			ImGui::PushID("save_popup_exists");
			ImGui::Text("File already exists. Are you sure?");
			if (ImGui::Button("Save"))
			{
				dungeonFileName = dungeonFileNameSaveAs;
				ImGui::CloseCurrentPopup();
				didSaveAs = true;
				Save();
			}
			ImGui::SameLine();
			if (ImGui::Button("Cancel"))
				ImGui::CloseCurrentPopup();
			ImGui::PopID();
			ImGui::EndPopup();
		}
		if (didSaveAs)
			ImGui::CloseCurrentPopup();

		ImGui::PopID();
		ImGui::EndPopup();
	}

	// Draw dungeon file list if requested
	if (ImGui::BeginPopup("Load Dungeon"))
	{
		ImGui::SameLine();
		ImGui::SeparatorText("Dungeon Name");
		for (auto d : fs::recursive_directory_iterator(subfolder))
		{
			if (d.path().has_extension() && d.path().extension() == extension)
			{
				string foundDungeonName = d.path().stem().string();
				string foundDungeonPath = d.path().relative_path().string();
				if (ImGui::Selectable(foundDungeonPath.c_str()))
				{
					dungeonFilePath = foundDungeonPath;
					dungeonFileName = foundDungeonName;
					dungeon->Load(dungeonFilePath);
				}
			}
		}
		ImGui::EndPopup();
	}
	ImGui::End();
}

void Crawl::DungeonEditor::Update()
{
	UpdateMousePosOnGrid();

	if (Input::Mouse(0).Pressed())
	{
		Crawl::Hall* hall = dungeon->AddHall(gridSelected.x, gridSelected.y);
		if (hall != nullptr)
			UpdateSurroundingTiles(hall->column, hall->row);
	}

	if (Input::Mouse(2).Pressed())
	{
		if(dungeon->DeleteHall(gridSelected.x, gridSelected.y))
			UpdateSurroundingTiles(gridSelected.x, gridSelected.y);
	}
}

/// <summary>
/// Updates the highlight grid unit based on the mouse window pos.
/// </summary>
void Crawl::DungeonEditor::UpdateMousePosOnGrid()
{
	vec2 NDC = Input::GetMousePosNDC();

	vec3 rayStart = Camera::s_instance->position;
	vec3 rayDir = Camera::s_instance->GetRayFromNDC(NDC);

	float scale = rayStart.y / rayDir.y;
	vec3 groundPos = rayStart - (rayDir * scale);
	gridSelected.x = glm::round(groundPos.x / DUNGEON_GRID_SCALE);
	gridSelected.y = glm::round(groundPos.z / DUNGEON_GRID_SCALE);

	Scene::s_instance->objects[0]->localTransform[3][0] = gridSelected.x * DUNGEON_GRID_SCALE;
	Scene::s_instance->objects[0]->localTransform[3][1] = 0;
	Scene::s_instance->objects[0]->localTransform[3][2] = gridSelected.y * DUNGEON_GRID_SCALE;
	Scene::s_instance->objects[0]->dirtyTransform = true;
}

void Crawl::DungeonEditor::UpdateSurroundingTiles(int column, int row)
{
	for (int co = column - 1; co <= column + 1; co++)
	{
		for (int ro = row - 1; ro <= row + 1; ro++)
		{
			Hall* hall = dungeon->GetHall(co, ro);

			if(hall != nullptr)
			{
				if(hall->object != nullptr)
					hall->object->markedForDeletion = true;

				dungeon->CreateTileObject(hall);
			}
		}
	}
}

void Crawl::DungeonEditor::Save()
{
	
	dungeon->Save(GetDungeonFilePath());
}

std::string Crawl::DungeonEditor::GetDungeonFilePath()
{
	std::string filename;
	filename += subfolder;
	filename += dungeonFileNameSaveAs;
	filename += extension;
	return filename;
}
