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
	dungeon = new Dungeon();
}

void Crawl::DungeonEditor::DrawGUI()
{
	ImGui::Begin("Dungeon Edit");
	ImGui::BeginDisabled();
	ImGui::Text(dungeonFileName.c_str());
	ImGui::EndDisabled();
	if (ImGui::Button("Save"))
		dungeon->Save(dungeonFileName);
	ImGui::SameLine();
	if (ImGui::Button("Save As"))
	{
		didSaveAs = false;
		ImGui::OpenPopup("dungeon_save_as");
		dungeonFileNameSaveAs = dungeonFileName;
	}

	ImGui::SameLine();
	if (ImGui::Button("Load"))
		ImGui::OpenPopup("popup_load_dungeon");

	ImGui::BeginDisabled();
	ImGui::DragInt2("Grid Selected", &gridSelected.x);
	ImGui::EndDisabled();

	// Save As prompt
	if (ImGui::BeginPopupModal("dungeon_save_as"))
	{
		ImGui::PushID("save_popup");
		ImGui::InputText("File Name", &dungeonFileNameSaveAs);
		if (ImGui::Button("Save"))
		{
			if (FileUtils::CheckFileExists(dungeonFileNameSaveAs))
				ImGui::OpenPopup("dungeon_save_as_file_exists");
			else
			{
				dungeonFileName = dungeonFileNameSaveAs;
				dungeon->Save(dungeonFileName);
			}

		}
		ImGui::SameLine();
		if (ImGui::Button("Cancel"))
			ImGui::CloseCurrentPopup();

		if (ImGui::BeginPopupModal("dungeon_save_as_file_exists"))
		{
			ImGui::PushID("save_popup_exists");
			ImGui::Text("File already exists. Are you sure?");
			if (ImGui::Button("Save"))
			{
				dungeonFileName = dungeonFileNameSaveAs;
				dungeon->Save(dungeonFileName);
				ImGui::CloseCurrentPopup();
				didSaveAs = true;
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

	// Draw scene file list if requested
	if (ImGui::BeginPopup("popup_load_dungeon"))
	{
		ImGui::SameLine();
		ImGui::SeparatorText("Dungeon Name");
		for (auto d : fs::recursive_directory_iterator("crawl/dungeons"))
		{
			if (d.path().has_extension() && d.path().extension() == ".dungeon")
			{
				string foundSceneName = d.path().filename().string();
				string foundScenePath = d.path().relative_path().string();
				if (ImGui::Selectable(foundScenePath.c_str()))
				{
					dungeonFileName = foundScenePath;
					dungeon->Load(dungeonFileName);
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