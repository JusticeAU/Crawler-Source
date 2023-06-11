#include "DungeonEditor.h"
#include "Object.h"
#include "Input.h"
#include "Camera.h"
#include "Scene.h"
#include <filesystem>
namespace fs = std::filesystem;

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
					// now need to process the dungeon and attach assets to each Hall
					BuildSceneFromDungeonLayout();
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
	if (Input::Mouse(0).Down())
	{
		Crawl::Hall* hall = dungeon->AddHall(gridSelected.x, gridSelected.y);
		if (hall != nullptr)
		{
			Object* obj = Scene::s_instance->DuplicateObject(Scene::s_instance->objects[1]);
			obj->localTransform[3][0] = gridSelected.x * GRID_SCALE;
			obj->localTransform[3][2] = gridSelected.y * GRID_SCALE;

			hall->object = obj;
		}
	}
	if (Input::Mouse(2).Down())
	{
		dungeon->DeleteHall(gridSelected.x, gridSelected.y);
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
	gridSelected.x = glm::round(groundPos.x / GRID_SCALE);
	gridSelected.y = glm::round(groundPos.z / GRID_SCALE);

	Scene::s_instance->objects[0]->localTransform[3][0] = gridSelected.x * GRID_SCALE;
	Scene::s_instance->objects[0]->localTransform[3][1] = 0;
	Scene::s_instance->objects[0]->localTransform[3][2] = gridSelected.y * GRID_SCALE;
	Scene::s_instance->objects[0]->dirtyTransform = true;
}

/// <summary>
/// After loading a dungeon, this will build it in the scene graph based on object templates.
/// This logic should probably move somewhere else. We'll also need to use this when just loading the game - so it's not a DungeonEditor specific function.
/// </summary>
void Crawl::DungeonEditor::BuildSceneFromDungeonLayout()
{
	while (Scene::s_instance->objects.size() > 2) // this is shit
		Scene::s_instance->objects.erase(Scene::s_instance->objects.end()-1);


	for (auto& column : dungeon->halls)
	{
		for (auto& row : column.second.row)
		{
			Crawl::Hall* hall = &row.second;
			Object* obj = Scene::s_instance->DuplicateObject(Scene::s_instance->objects[1]);
			obj->localTransform[3][0] = hall->column * GRID_SCALE;
			obj->localTransform[3][2] = hall->row * GRID_SCALE;

			hall->object = obj;
		}
	}
}
