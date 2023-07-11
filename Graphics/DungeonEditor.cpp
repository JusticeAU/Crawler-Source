#include "DungeonEditor.h"
#include "DungeonDoor.h"
#include "DungeonInteractableLever.h"
#include "DungeonHelpers.h"
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

void Crawl::DungeonEditor::Activate()
{
	Scene::ChangeScene("Dungeon");
	Scene::SetCameraIndex(0);
}

void Crawl::DungeonEditor::Deactivate()
{

}

void Crawl::DungeonEditor::DrawGUI()
{
	ImGui::SetNextWindowPos({ 0,0 }, ImGuiCond_Always);
	ImGui::Begin("Dungeon Edit", 0, ImGuiWindowFlags_NoMove);
	DrawGUIFileOperations();
	DrawGUICursorInformation();
	DrawGUIModeSelect();
	switch (editMode)
	{
		case Mode::TileBrush:
		{
			DrawGUIModeTileBrush();
			break;
		}
		case Mode::TileEdit:
		{
			DrawGUIModeTileEdit();
			break;
		}
	}
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
void Crawl::DungeonEditor::DrawGUIModeSelect()
{
	string mode = "Tile Brush";
	if (ImGui::BeginCombo("Mode", editModeNames[(int)editMode].c_str()))
	{
		// Options here!
		if (ImGui::Selectable(editModeNames[0].c_str()))
			editMode = Mode::TileBrush;
		if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
			ImGui::SetTooltip("Carve new tiles into the level.");
		
		if (ImGui::Selectable(editModeNames[1].c_str()))
			editMode = Mode::TileEdit;
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
void Crawl::DungeonEditor::DrawGUIModeTileEdit()
{
	if (!selectedTile)
	{
		ImGui::Text("No Tile Selected");
		return;
	}


	// selected tile coordinates
	ImGui::Text("Selected Tile");
	ImGui::BeginDisabled();
	ImGui::DragInt2("Location", &selectedTile->position.x);
	ImGui::EndDisabled();

	// Cardinal traversable yes/no
	if (ImGui::Checkbox("Open North", &selectedTileOpenWalls[0]))
	{
		selectedTile->mask += selectedTileOpenWalls[0] ? 1 : -1;
		UpdateWallVariants(selectedTile);
		dungeon->CreateTileObject(selectedTile);
	}
	if (ImGui::Checkbox("Open South", &selectedTileOpenWalls[1]))
	{
		selectedTile->mask += selectedTileOpenWalls[1] ? 8 : -8;
		UpdateWallVariants(selectedTile);
		dungeon->CreateTileObject(selectedTile);
	}
	if (ImGui::Checkbox("Open East", &selectedTileOpenWalls[2]))
	{
		selectedTile->mask += selectedTileOpenWalls[2] ? 4 : -4;
		UpdateWallVariants(selectedTile);
		dungeon->CreateTileObject(selectedTile);
	}
	if (ImGui::Checkbox("Open West", &selectedTileOpenWalls[3]))
	{
		selectedTile->mask += selectedTileOpenWalls[3] ? 2 : -2;
		UpdateWallVariants(selectedTile);
		dungeon->CreateTileObject(selectedTile);
	}


	// if yes, what wall variant
	ImGui::BeginDisabled();
	if (!selectedTileOpenWalls[0])
		ImGui::Text(dungeon->wallVariantPaths[selectedTile->wallVariants[0] - 1].c_str());
	else
		ImGui::Text("Open");

	if (!selectedTileOpenWalls[1])
		ImGui::Text(dungeon->wallVariantPaths[selectedTile->wallVariants[1] - 1].c_str());
	else
		ImGui::Text("Open");

	if (!selectedTileOpenWalls[2])
		ImGui::Text(dungeon->wallVariantPaths[selectedTile->wallVariants[2] - 1].c_str());
	else
		ImGui::Text("Open");

	if (!selectedTileOpenWalls[3])
		ImGui::Text(dungeon->wallVariantPaths[selectedTile->wallVariants[3] - 1].c_str());
	else
		ImGui::Text("Open");

	ImGui::EndDisabled();

	// entities
	for (auto& door : selectTileDoors)
	{
		string doorName = "Door ID (";
		doorName += to_string(door->id);
		doorName += ")";
		if (ImGui::Selectable(doorName.c_str()))
			selectedDoor = door;
	}
	if (ImGui::Button("Add Door"))
	{
		dungeon->CreateDoor(selectedTile->position, 0, GetNextAvailableDoorID(), false);
		RefreshSelectedTile();
	}

	for (auto& lever : selectTileLevers)
	{
		string leverName = "Lever ID (";
		leverName += to_string(lever->id);
		leverName += ")";
		if (ImGui::Selectable(leverName.c_str()))
			selectedLever = lever;
	}
	if (ImGui::Button("Add Lever"))
	{
		dungeon->CreateLever(selectedTile->position, 0, GetNextAvailableLeverID(), 0, false);
		RefreshSelectedTile();
	}

	if (selectedDoor)
		DrawGUIModeTileEditDoor();
	if (selectedLever)
		DrawGUIModeTileEditLever();
}

void Crawl::DungeonEditor::DrawGUIModeTileEditDoor()
{
	ImGui::Begin("Edit Door");
	ImGui::InputInt("ID", (int*)&selectedDoor->id);
	if (ImGui::BeginCombo("Orientation", orientationNames[selectedDoor->orientation].c_str()))
	{
		for (int i = 0; i < 4; i++)
			if (ImGui::Selectable(orientationNames[i].c_str()))
			{
				selectedDoor->orientation = i;
				selectedDoor->object->SetLocalRotationZ(orientationEulers[i]);
			}
		ImGui::EndCombo();
	}
	ImGui::Checkbox("Starts Open", &selectedDoor->startOpen);
	if (ImGui::Checkbox("Is Open", &selectedDoor->open))
	{
		selectedDoor->open = !selectedDoor->open;
		selectedDoor->Toggle(); // this could go out of sync.
	}
	ImGui::End();
}
void Crawl::DungeonEditor::DrawGUIModeTileEditLever()
{
	ImGui::Begin("Edit Lever");
	if (ImGui::InputInt("ID", (int*)&selectedLever->id))
		selectedLever->SetID(selectedLever->id);

	if (ImGui::BeginCombo("Orientation", orientationNames[selectedLever->orientation].c_str()))
	{
		for (int i = 0; i < 4; i++)
			if (ImGui::Selectable(orientationNames[i].c_str()))
			{
				selectedLever->orientation = i;
				selectedLever->object->SetLocalRotationZ(orientationEulers[i]);
			}
		ImGui::EndCombo();
	}

	ImGui::Checkbox("Starts Status", &selectedLever->startStatus);

	if (ImGui::Checkbox("Status", &selectedLever->status))
	{
		selectedLever->status = !selectedLever->status;
		selectedLever->Toggle(); // this could go out of sync.
	}
	ImGui::InputInt("Door ID", (int*)&selectedLever->doorID);
	ImGui::End();
}

void Crawl::DungeonEditor::Update()
{
	switch (editMode)
	{
	case Mode::TileBrush:
		UpdateModeTileBrush();	break;
	case Mode::TileEdit:
		UpdateModeTileEdit();	break;
	}
}
void Crawl::DungeonEditor::UpdateModeTileBrush()
{
	UpdateMousePosOnGrid();
	if (Input::Mouse(0).Pressed())
	{
		Crawl::DungeonTile* tile = dungeon->AddTile(gridSelected);
		if (tile != nullptr)
		{
			dungeon->SetTileMask(gridSelected, brush_tileMask);

			if (brush_AutoTileEnabled)
			{
				if (brush_AutoTileSurround)
					UpdateSurroundingTiles(gridSelected);
				else
					UpdateAutoTile(gridSelected);
			}
			else
			{
				tile->mask = brush_tileMask;
				if ((tile->mask & 1) != 1) // North Wall
					tile->wallVariants[0] = 1;
				if ((tile->mask & 8) != 8) // South Wall
					tile->wallVariants[1] = 1;
				if ((tile->mask & 4) != 4) // East Wall
					tile->wallVariants[2] = 1;
				if ((tile->mask & 2) != 2) // West Wall
					tile->wallVariants[3] = 1;
				dungeon->CreateTileObject(tile);
			}
		}
	}

	if (Input::Mouse(2).Pressed())
	{
		if (dungeon->DeleteTile(gridSelected))
		{
			if (brush_AutoTileEnabled)
			{
				if (brush_AutoTileSurround)
					UpdateSurroundingTiles(gridSelected);
				else
					UpdateAutoTile(gridSelected);
			}
		}
	}
}
void Crawl::DungeonEditor::UpdateModeTileEdit()
{
	if (Input::Mouse(0).Down())
	{
		selectedDoor = nullptr;
		selectedLever = nullptr;
		glm::ivec2 selectionPos = GetMousePosOnGrid();

		selectedTile = dungeon->GetTile(selectionPos);
		if (!selectedTile)
			return;
		
		RefreshSelectedTile();
	}
}

void Crawl::DungeonEditor::RefreshSelectedTile()
{
	selectedTileOpenWalls[0] = (selectedTile->mask & 1) == 1; // North Check
	selectedTileOpenWalls[1] = (selectedTile->mask & 8) == 8; // South Check
	selectedTileOpenWalls[2] = (selectedTile->mask & 4) == 4; // East Check
	selectedTileOpenWalls[3] = (selectedTile->mask & 2) == 2; // West Check

	// update list of tiles doors and levers / interactabes
	selectTileDoors.clear();
	for (int i = 0; i < dungeon->doors.size(); i++)
	{
		if (dungeon->doors[i]->position == selectedTile->position)
			selectTileDoors.push_back(dungeon->doors[i]);
	}

	selectTileLevers.clear();
	for (int i = 0; i < dungeon->interactables.size(); i++)
	{
		if (dungeon->interactables[i]->position == selectedTile->position)
			selectTileLevers.push_back(dungeon->interactables[i]);
	}
}

int Crawl::DungeonEditor::GetNextAvailableLeverID()
{
	int nextAvail = 1;
	bool unused = false;
	while (!unused)
	{
		unused = true;
		for (int i = 0; i < dungeon->interactables.size(); i++)
		{
			if (dungeon->interactables[i]->id == nextAvail)
			{
				unused = false;
				nextAvail++;
				break;
			}
		}
	}
	return nextAvail;
}

int Crawl::DungeonEditor::GetNextAvailableDoorID()
{
	int nextAvail = 1;
	bool unused = false;
	while (!unused)
	{
		unused = true;
		for (int i = 0; i < dungeon->doors.size(); i++)
		{
			if (dungeon->doors[i]->id == nextAvail)
			{
				unused = false;
				nextAvail++;
				break;
			}
		}
	}
	return nextAvail;
}

glm::ivec2 Crawl::DungeonEditor::GetMousePosOnGrid()
{
	// Do the math to figure out where we're pointing
	vec2 NDC = Input::GetMousePosNDC();
	vec3 rayStart = Camera::s_instance->position;
	vec3 rayDir = Camera::s_instance->GetRayFromNDC(NDC);
	float scale = rayStart.z / rayDir.z;
	vec3 groundPos = rayStart - (rayDir * scale);

	// Update our data
	glm::ivec2 grid;
	grid.x = glm::round(groundPos.x / DUNGEON_GRID_SCALE);
	grid.y = glm::round(groundPos.y / DUNGEON_GRID_SCALE);
	return grid;
}
void Crawl::DungeonEditor::UpdateMousePosOnGrid()
{
	gridSelected = GetMousePosOnGrid();

	// Update our visual
	// Build a temporary tile mask based on our mode
	brush_tileMask = 0;
	if (brush_AutoTileEnabled)
		brush_tileMask = dungeon->GetAutoTileMask(gridSelected);
	else
	{
		if (!brush_TileNorth)
			brush_tileMask += NORTH_MASK;
		// test west
		if (!brush_TileWest)
			brush_tileMask += WEST_MASK;
		// test east
		if (!brush_TileEast)
			brush_tileMask += EAST_MASK;
		// test south
		if (!brush_TileSouth)
			brush_tileMask += SOUTH_MASK;
	}

	Object* tileTemplate = dungeon->GetTileTemplate(brush_tileMask);
	Scene::s_instance->objects[0] = tileTemplate;
	Scene::s_instance->objects[0]->SetLocalPosition({ gridSelected.x * DUNGEON_GRID_SCALE, gridSelected.y * DUNGEON_GRID_SCALE, 0 });

}

void Crawl::DungeonEditor::UpdateAutoTile(ivec2 position)
{
	DungeonTile* tile = dungeon->GetTile(position);

	if (tile != nullptr)
	{
		tile->mask = dungeon->GetAutoTileMask(tile->position);
		UpdateWallVariants(tile);
		dungeon->CreateTileObject(tile);
	}
}

void Crawl::DungeonEditor::UpdateWallVariants(DungeonTile* tile)
{
	if ((tile->mask & 1) != 1) // North Wall
	{
		if (tile->wallVariants[0] == 0)
			tile->wallVariants[0] = (rand() % WALL_VARIANT_COUNT) + 1;
	}
	else
		tile->wallVariants[0] = 0;
	if ((tile->mask & 8) != 8) // South Wall
	{
		if (tile->wallVariants[1] == 0)
			tile->wallVariants[1] = (rand() % WALL_VARIANT_COUNT) + 1;
	}
	else
		tile->wallVariants[1] = 0;
	if ((tile->mask & 4) != 4) // East Wall
	{
		if (tile->wallVariants[2] == 0)
			tile->wallVariants[2] = (rand() % WALL_VARIANT_COUNT) + 1;
	}
	else
		tile->wallVariants[2] = 0;
	if ((tile->mask & 2) != 2) // West Wall
	{
		if (tile->wallVariants[3] == 0)
			tile->wallVariants[3] = (rand() % WALL_VARIANT_COUNT) + 1;
	}
	else
		tile->wallVariants[3] = 0;
}

void Crawl::DungeonEditor::UpdateSurroundingTiles(ivec2 position)
{
	ivec2 delta;
	for (delta.x = position.x - 1; delta.x <= position.x + 1; delta.x++)
	{
		for (delta.y = position.y - 1; delta.y <= position.y + 1; delta.y++)
			UpdateAutoTile(delta);
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
