#include "DungeonEditor.h"
#include "Object.h"
#include "Input.h"
#include "Camera.h"
#include "Scene.h"
#include <filesystem>
namespace fs = std::filesystem;

#include "ComponentModel.h";
#include "ComponentRenderer.h";


Crawl::DungeonEditor::DungeonEditor()
{
	
}

void Crawl::DungeonEditor::DrawGUI()
{
	ImGui::SetNextWindowPos({ 0,0 }, ImGuiCond_Always);
	ImGui::Begin("Dungeon Edit", 0, ImGuiWindowFlags_NoMove);
	DrawGUIFileOperations();
	//DrawGUICursorInformation();
	DrawGUIMode();
	DrawGUIModeTileBrush();
	ImGui::End();
}
void Crawl::DungeonEditor::DrawGUIFileOperations()
{
	ImGui::BeginDisabled();
	ImGui::Text(dungeonFileName.c_str());
	ImGui::EndDisabled();

	if (dungeonFilePath == "")
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

	// Save As prompt
	ImGui::SetNextWindowSize({ 300, 100 });
	ImGui::SetNextWindowPos({ 0,0 }, ImGuiCond_Always);
	if (ImGui::BeginPopupModal("Save As", 0, ImGuiWindowFlags_NoCollapse & ImGuiWindowFlags_NoResize & ImGuiWindowFlags_NoMove))
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
					dungeonFileName = foundDungeonName;
					dungeonFileNameSaveAs = foundDungeonName;
					dungeonFilePath = foundDungeonPath;
					dungeon->Load(dungeonFilePath);
				}
			}
		}
		ImGui::EndPopup();
	}
}
void Crawl::DungeonEditor::DrawGUICursorInformation()
{
	ImGui::BeginDisabled();
	ImGui::DragInt2("Grid Selected", &gridSelected.x);
	ImGui::EndDisabled();
}
void Crawl::DungeonEditor::DrawGUIMode()
{
	string mode = "Tile Brush";
	if (ImGui::BeginCombo("Mode", mode.c_str()))
	{
		// Options here!
		ImGui::Selectable("Tile Brush");
		if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
			ImGui::SetTooltip("Carve new hallways into the level.");
		ImGui::Selectable("Tile Editor");
		if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
			ImGui::SetTooltip("Edit specific configuration items on a tile.");
		ImGui::Selectable("Entity Editor");
		if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
			ImGui::SetTooltip("Add and edit entities in the world");
		ImGui::EndCombo();
	}
}
void Crawl::DungeonEditor::DrawGUIModeTileBrush()
{
	ImGui::Text("Auto Tile");
	ImGui::Indent();
		ImGui::Checkbox("Enabled", &brush_AutoTileEnabled);
		if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
			ImGui::SetTooltip("Auto Tiling intelligently places the correctly configured tile down to connect to its neighbors.\nIt will ignore the below Tile Configuration");

		if (!brush_AutoTileEnabled) ImGui::BeginDisabled();
		ImGui::Checkbox("Update Neighbors", &brush_AutoTileSurround);
		if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
			ImGui::SetTooltip("This will cause surrounding tiles to be updated to connect to the new tile.");

		if (!brush_AutoTileEnabled) ImGui::EndDisabled();
	ImGui::Unindent();
	ImGui::NewLine();
	
	ImGui::Text("Tile Configuration");
	ImGui::Indent();
		if (brush_AutoTileEnabled) ImGui::BeginDisabled();
		ImGui::Checkbox("North Wall", &brush_TileNorth);
		ImGui::Checkbox("South Wall", &brush_TileSouth);
		ImGui::Checkbox("East Wall", &brush_TileEast);
		ImGui::Checkbox("West Wall", &brush_TileWest);
		if (brush_AutoTileEnabled) ImGui::EndDisabled();
	ImGui::Unindent();



}
void Crawl::DungeonEditor::Update()
{
	UpdateMousePosOnGrid();

	if (Input::Mouse(0).Pressed())
	{
		Crawl::DungeonTile* hall = dungeon->AddHall(gridSelected.x, gridSelected.y);
		if (hall != nullptr)
		{
			dungeon->SetHallMask(gridSelected.x, gridSelected.y, brush_tileMask);

			if (brush_AutoTileEnabled)
			{
				if (brush_AutoTileSurround)
					UpdateSurroundingTiles(gridSelected.x, gridSelected.y);
				else
					UpdateTile(gridSelected.x, gridSelected.y);
			}
			else
			{
				hall->mask = brush_tileMask;
				dungeon->CreateTileObject(hall);
			}
		}
	}

	if (Input::Mouse(2).Pressed())
	{
		if (dungeon->DeleteHall(gridSelected.x, gridSelected.y))
		{
			if (brush_AutoTileEnabled)
			{
				if (brush_AutoTileSurround)
					UpdateSurroundingTiles(gridSelected.x, gridSelected.y);
				else
					UpdateTile(gridSelected.x, gridSelected.y);
			}
		}
	}
}

/// <summary>
/// Updates the highlight grid unit based on the mouse window pos.
/// </summary>
void Crawl::DungeonEditor::UpdateMousePosOnGrid()
{
	// Do the math to figure out where we're pointing
	vec2 NDC = Input::GetMousePosNDC();
	vec3 rayStart = Camera::s_instance->position;
	vec3 rayDir = Camera::s_instance->GetRayFromNDC(NDC);
	float scale = rayStart.z / rayDir.z;
	vec3 groundPos = rayStart - (rayDir * scale);

	// Update our data
	gridSelected.x = glm::round(groundPos.x / DUNGEON_GRID_SCALE);
	gridSelected.y = glm::round(groundPos.y / DUNGEON_GRID_SCALE);

	// Update our visual
	// Build a temporary tile mask based on our mode
	brush_tileMask = 0;
	if (brush_AutoTileEnabled)
		brush_tileMask = dungeon->GetAutoTileMask(gridSelected.x, gridSelected.y);
	else
	{
		if (!brush_TileNorth)
			brush_tileMask += 1;
		// test west
		if (!brush_TileWest)
			brush_tileMask += 2;
		// test east
		if (!brush_TileEast)
			brush_tileMask += 4;
		// test south
		if (!brush_TileSouth)
			brush_tileMask += 8;
	}

	Object* tileTemplate = dungeon->GetTileTemplate(brush_tileMask);
	ComponentModel* templateModel = (ComponentModel*)tileTemplate->GetComponent(Component_Model);
	ComponentModel* cursorModel = (ComponentModel*)Scene::s_instance->objects[0]->GetComponent(Component_Model);
	cursorModel->model = templateModel->model;
	ComponentRenderer* cursorRenderer = (ComponentRenderer*)Scene::s_instance->objects[0]->GetComponent(Component_Renderer);
	cursorRenderer->model = templateModel->model;
	Scene::s_instance->objects[0]->SetLocalPosition({ gridSelected.x * DUNGEON_GRID_SCALE, gridSelected.y * DUNGEON_GRID_SCALE, 0 });
	Scene::s_instance->objects[0]->SetLocalRotation(tileTemplate->localRotation);
}

void Crawl::DungeonEditor::UpdateTile(int x, int y)
{
	DungeonTile* hall = dungeon->GetHall(x, y);

	if (hall != nullptr)
	{
		if (hall->object != nullptr)
			hall->object->markedForDeletion = true;

		hall->mask = dungeon->GetAutoTileMask(hall->position.x, hall->position.y);
		dungeon->CreateTileObject(hall);
	}
}

void Crawl::DungeonEditor::UpdateSurroundingTiles(int x, int y)
{
	for (int xDelta = x - 1; xDelta <= x + 1; xDelta++)
	{
		for (int yDelta = y - 1; yDelta <= y + 1; yDelta++)
			UpdateTile(xDelta, yDelta);
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
