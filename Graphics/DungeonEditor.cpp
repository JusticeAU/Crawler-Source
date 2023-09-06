#include "DungeonEditor.h"
#include "DungeonDoor.h"
#include "DungeonInteractableLever.h"
#include "DungeonActivatorPlate.h"
#include "DungeonTransporter.h"
#include "DungeonHelpers.h"
#include "DungeonPlayer.h"
#include "DungeonSpikes.h"
#include "DungeonPushableBlock.h"
#include "DungeonShootLaser.h"
#include "DungeonEnemyBlocker.h"
#include "DungeonEnemyChase.h"
#include "DungeonEnemySwitcher.h"
#include "DungeonCheckpoint.h"
#include "DungeonMirror.h"
#include "DungeonEnemySlug.h"
#include "DungeonDecoration.h"
#include "DungeonStairs.h"

#include "Object.h"
#include "Input.h"
#include "Scene.h"
#include "SceneEditorCamera.h"
#include <filesystem>
namespace fs = std::filesystem;

#include "ComponentModel.h"
#include "ComponentRenderer.h"
#include "ComponentCamera.h"

#include "LogUtils.h"
#include "LineRenderer.h"


Crawl::DungeonEditor::DungeonEditor()
{
	brushObject = Scene::CreateObject("Tile Brush");
	brushObject->LoadFromJSON(ReadJSONFromDisk("crawler/model/tile_wood.object"));
}

void Crawl::DungeonEditor::Activate()
{
	Scene::ChangeScene("Dungeon");
	Scene::SetCameraByName();
	RefreshAvailableDecorations();

	// It's possible that gameplay took us in to another dungeon, and this, that is the one that should now be loaded.
	RefreshDungeonFileNames();
}

void Crawl::DungeonEditor::Deactivate()
{

}

void Crawl::DungeonEditor::SetDungeon(Dungeon* dungeonPtr)
{
	dungeon = dungeonPtr;
	wallVarientShortNames.clear();
	for (auto varient : dungeon->wallVariantPaths)
	{
		int lastSlash = varient.find_last_of('/');
		int extension = varient.find_last_of('.');
		wallVarientShortNames.push_back(varient.substr(lastSlash + 1, extension - lastSlash -1));
	}
}

void Crawl::DungeonEditor::DrawGUI()
{
	ImGui::SetNextWindowPos({ 0,0 }, ImGuiCond_Always);
	ImGui::SetNextWindowSize({ 320, 600 }, ImGuiCond_FirstUseEver);
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
		case Mode::DungeonProperties:
		{
			DrawGUIModeDungeonProperties();
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

	// Gameplay
	if (unsavedChanges)
		ImGui::BeginDisabled();

	if (ImGui::Button(!dirtyGameplayScene ? "Play" : "Resume"))
	{
		TileEditUnselectAll();
		requestedGameMode = true;
		if (!dirtyGameplayScene)
			dungeon->player->Respawn();
		dirtyGameplayScene = true;
	}

	if (unsavedChanges)
		ImGui::EndDisabled();


	ImGui::SameLine();
	if (ImGui::Button("Reset Dungeon"))
	{
		TileEditUnselectAll();
		dungeon->ResetDungeon();
		dungeon->player->ClearRespawn();
		dungeon->player->Teleport(dungeon->defaultPlayerStartPosition);
		dungeon->player->Orient(dungeon->defaultPlayerStartOrientation);
		dirtyGameplayScene = false;
		UnMarkUnsavedChanges();
	}

	// File Manipulation
	if (dirtyGameplayScene)
		ImGui::BeginDisabled();

	if (dungeonFilePath == "" || !unsavedChanges)
		ImGui::BeginDisabled();

	if (ImGui::Button("Save"))
		Save();
	else if (dungeonFilePath == "" || !unsavedChanges)
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

	if (dirtyGameplayScene)
	{
		ImGui::SameLine();
		ImGui::Text("Scene is Dirty");
		ImGui::EndDisabled();
	}
	else
	{
		ImGui::SameLine();
		if (ImGui::Button("New Dungeon"))
		{
			dungeon->ClearDungeon();
			dungeonFileName = "";
			dungeonFileNameSaveAs = "";
			dungeonFilePath = "";
		}
	}


	// Save As prompt
	ImGui::SetNextWindowSize({ 300, 100 });
	ImGui::SetNextWindowPos({ 0,0 }, ImGuiCond_Always);
	if (ImGui::BeginPopupModal("Save As", 0, ImGuiWindowFlags_NoCollapse & ImGuiWindowFlags_NoResize & ImGuiWindowFlags_NoMove))
	{
		ImGui::PushID("save_popup");
		ImGui::InputText(dungeon->dungeonFileExtension.c_str(), &dungeonFileNameSaveAs);
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
		for (auto d : fs::recursive_directory_iterator(dungeon->dungeonFileLocation))
		{
			if (d.path().has_extension() && d.path().extension() == dungeon->dungeonFileExtension)
			{
				string foundDungeonPath = d.path().relative_path().string();
				std::replace(foundDungeonPath.begin(), foundDungeonPath.end(), '\\', '/');
				if (ImGui::Selectable(foundDungeonPath.c_str()))
				{
					if (unsavedChanges)
					{
						dungeonWantLoad = foundDungeonPath;
						shouldConfirmSaveBeforeLoad = true;
					}
					else
						Load(foundDungeonPath);
				}
			}
		}
		ImGui::EndPopup();
	}
	if (shouldConfirmSaveBeforeLoad)
	{
		ImGui::OpenPopup("Unsaved Changes");
		shouldConfirmSaveBeforeLoad = false;
	}

	ImGui::SetNextWindowSize({ 300, 100 });
	ImGui::SetNextWindowPos({ 0,0 }, ImGuiCond_Always);
	if(ImGui::BeginPopupModal("Unsaved Changes"))
	{
		ImGui::Text("You have unsaved changes.\nAre you sure you want to load?", 0, ImGuiWindowFlags_NoCollapse & ImGuiWindowFlags_NoResize & ImGuiWindowFlags_NoMove);

		if (ImGui::Button("Load Anyway"))
		{
			Load(dungeonWantLoad);
			ImGui::CloseCurrentPopup();
		}
		ImGui::SameLine();
		if (ImGui::Button("Cancel"))
		{
			dungeonWantLoad = "";
			ImGui::CloseCurrentPopup();
		}
		ImGui::EndPopup();
	}
}
void Crawl::DungeonEditor::DrawGUICursorInformation()
{
	ImGui::BeginDisabled();
	ImGui::DragInt2("Grid Selected", &gridSelected.x);
	ImGui::EndDisabled();

	/*if (ImGui::BeginCombo("Path Start Orientation", orientationNames[facingTest].c_str()))
	{
		int oldOrientation = facingTest;
		for (int i = 0; i < 4; i++)
		{
			if (ImGui::Selectable(orientationNames[i].c_str()))
				facingTest = i;
		}
		ImGui::EndCombo();
	}*/
}
void Crawl::DungeonEditor::DrawGUIModeSelect()
{
	string mode = "Tile Brush";
	if (ImGui::BeginCombo("Mode", editModeNames[(int)editMode].c_str()))
	{
		// Options here!
		if (ImGui::Selectable(editModeNames[0].c_str()))
		{
			editMode = Mode::TileBrush;
			TileEditUnselectAll();
		}
		if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
			ImGui::SetTooltip("Carve new tiles into the level.");
		
		if (ImGui::Selectable(editModeNames[1].c_str()))
			editMode = Mode::TileEdit;
		if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
			ImGui::SetTooltip("Edit specific configuration items on a tile.");
		
		/*ImGui::Selectable("Entity Editor");
		if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
			ImGui::SetTooltip("Add and edit entities in the world");*/

		if (ImGui::Selectable(editModeNames[3].c_str()))
			editMode = Mode::DungeonProperties;
		if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
			ImGui::SetTooltip("Edit specific configuration items about this particular dungeon");

		if (ImGui::Selectable(editModeNames[4].c_str()))
			editMode = Mode::SlugPathEditor;
		if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
			ImGui::SetTooltip("Build paths for the slugs");
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
	ImGui::Checkbox("Occupied", &selectedTile->occupied);
	ImGui::EndDisabled();

	// Cardinal traversable/see yes/no
	unsigned int oldMaskTraverse = selectedTile->maskTraverse;
	ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(0, 255, 0, 255));
	if (ImGui::BeginCombo("Can Walk", maskToString[oldMaskTraverse].c_str()))
	{
		if (ImGui::Checkbox("Can Walk North", &selectedTileUntraversableWalls[0]))
			selectedTile->maskTraverse += selectedTileUntraversableWalls[0] ? NORTH_MASK : -NORTH_MASK;
		if (ImGui::Checkbox("Can Walk South", &selectedTileUntraversableWalls[1]))
			selectedTile->maskTraverse += selectedTileUntraversableWalls[1] ? SOUTH_MASK : -SOUTH_MASK;
		if (ImGui::Checkbox("Can Walk East", &selectedTileUntraversableWalls[2]))
			selectedTile->maskTraverse += selectedTileUntraversableWalls[2] ? EAST_MASK : -EAST_MASK;
		if (ImGui::Checkbox("Can Walk West", &selectedTileUntraversableWalls[3]))
			selectedTile->maskTraverse += selectedTileUntraversableWalls[3] ? WEST_MASK : -WEST_MASK;
		ImGui::EndCombo();
	}
	ImGui::PopStyleColor();
	if (oldMaskTraverse != selectedTile->maskTraverse) MarkUnsavedChanges();

	unsigned int oldMaskSee = selectedTile->maskSee;
	ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(123, 123, 255, 255));
	if (ImGui::BeginCombo("Can See", maskToString[oldMaskSee].c_str()))
	{
		if (ImGui::Checkbox("Can See North", &selectedTileSeeThroughWalls[0]))
			selectedTile->maskSee += selectedTileSeeThroughWalls[0] ? NORTH_MASK : -NORTH_MASK;
		if (ImGui::Checkbox("Can See South", &selectedTileSeeThroughWalls[1]))
			selectedTile->maskSee += selectedTileSeeThroughWalls[1] ? SOUTH_MASK : -SOUTH_MASK;
		if (ImGui::Checkbox("Can See East", &selectedTileSeeThroughWalls[2]))
			selectedTile->maskSee += selectedTileSeeThroughWalls[2] ? EAST_MASK : -EAST_MASK;
		if (ImGui::Checkbox("Can See West", &selectedTileSeeThroughWalls[3]))
			selectedTile->maskSee += selectedTileSeeThroughWalls[3] ? WEST_MASK : -WEST_MASK;
		ImGui::EndCombo();
	}
	ImGui::PopStyleColor();
	if (oldMaskSee != selectedTile->maskSee) MarkUnsavedChanges();

	// Wall Variants
	ImGui::Text("Wall Variants");
	for (int cardinal = 0; cardinal < 4; cardinal++)
	{
		ImGui::PushID(cardinal);
		if (ImGui::BeginCombo(orientationNames[cardinal].c_str(), selectedTile->wallVariants[cardinal] == -1 ? "None" : wallVarientShortNames[selectedTile->wallVariants[cardinal]].c_str())) // lmao yuck
		{
			if (ImGui::Selectable("None", -1 == selectedTile->wallVariants[cardinal]))
			{
				selectedTile->wallVariants[cardinal] = -1;
				dungeon->CreateTileObject(selectedTile);
				MarkUnsavedChanges();
			}

			for (int i = 0; i < dungeon->wallVariantPaths.size(); i++)
			{
				if (ImGui::Selectable(wallVarientShortNames[i].c_str(), i == selectedTile->wallVariants[cardinal]))
				{
					selectedTile->wallVariants[cardinal] = i;
					dungeon->CreateTileObject(selectedTile);
					MarkUnsavedChanges();
				}
			}
			ImGui::EndCombo();
		}
		ImGui::PopID();
	}

	// Path finding dev stuff.
	/*int distance = dungeon->goodPath.size();
	ImGui::InputInt("Distance", &distance);
	ImGui::InputInt("Cost", &selectedTile->cost);
	for (int i = 0; i < 4; i++)
	{
		if(selectedTile->neighbors[i])
			ImGui::Text("neighbor");
		else
			ImGui::Text("nah");
	}*/

	// if yes, what wall variant - hidden for now, non-functional
	/*ImGui::BeginDisabled();
	if (!selectedTileUntraversableWalls[0])
		ImGui::Text(dungeon->wallVariantPaths[selectedTile->wallVariants[0] - 1].c_str());
	else
		ImGui::Text("Open");

	if (!selectedTileUntraversableWalls[1])
		ImGui::Text(dungeon->wallVariantPaths[selectedTile->wallVariants[1] - 1].c_str());
	else
		ImGui::Text("Open");

	if (!selectedTileUntraversableWalls[2])
		ImGui::Text(dungeon->wallVariantPaths[selectedTile->wallVariants[2] - 1].c_str());
	else
		ImGui::Text("Open");

	if (!selectedTileUntraversableWalls[3])
		ImGui::Text(dungeon->wallVariantPaths[selectedTile->wallVariants[3] - 1].c_str());
	else
		ImGui::Text("Open");

	ImGui::EndDisabled();*/

	// Buttons
	ImGui::Text("Add");
	ImGui::PushID("Add");
	if (ImGui::Button("Door"))
	{
		MarkUnsavedChanges();
		selectedDoor = dungeon->CreateDoor(selectedTile->position, 0, GetNextAvailableDoorID(), false);
		selectedTileDoors.push_back(selectedDoor);
		selectedDoorWindowOpen = true;
	}
	ImGui::SameLine();
	if (ImGui::Button("Lever"))
	{
		MarkUnsavedChanges();
		selectedLever = dungeon->CreateLever(selectedTile->position, 0, GetNextAvailableLeverID(), 0, false);
		selectedTileLevers.push_back(selectedLever);
		selectedLeverWindowOpen = true;
	}
	ImGui::SameLine();
	if (ImGui::Button("Shooter"))
	{
		MarkUnsavedChanges();
		selectedTileShootLaser = dungeon->CreateShootLaser(selectedTile->position, (FACING_INDEX)0, GetNextAvailableDoorID());
		selectedTileShootLasers.push_back(selectedTileShootLaser);
		selectedShootLaserWindowOpen = true;
	}
	bool tileOccupied = selectedTileOccupied;
	if (tileOccupied) ImGui::BeginDisabled();
	if (ImGui::Button("Activator Plate"))
	{
		MarkUnsavedChanges();
		selectedActivatorPlate = dungeon->CreatePlate(selectedTile->position, 0);
		selectedActivatorPlateWindowOpen = true;
		selectedTileOccupied = true;
	}
	ImGui::SameLine();
	if (ImGui::Button("Spikes"))
	{
		MarkUnsavedChanges();
		dungeon->CreateSpikes(selectedTile->position);
		selectedHasSpikes = true;
		selectedTileOccupied = true;
	}
	ImGui::SameLine();
	if (ImGui::Button("Box"))
	{
		MarkUnsavedChanges();
		dungeon->CreatePushableBlock(selectedTile->position);
		selectedHasBlock = true;
		selectedTileOccupied = true;
	}
	ImGui::SameLine();
	if (ImGui::Button("Mirror"))
	{
		MarkUnsavedChanges();
		selectedMirror = dungeon->CreateMirror(selectedTile->position, NORTH_INDEX);
		selectedMirrorWindowOpen = true;
		selectedTileOccupied = true;
	}

	if (ImGui::Button("Transporter"))
	{
		MarkUnsavedChanges();
		selectedTransporter = dungeon->CreateTransporter(selectedTile->position);
		selectedTransporterWindowOpen = true;
		selectedTileOccupied = true;
	}
	ImGui::SameLine();
	if (ImGui::Button("Checkpoint"))
	{
		MarkUnsavedChanges();
		selectedCheckpoint = dungeon->CreateCheckpoint(selectedTile->position, NORTH_INDEX);
		selectedCheckpointWindowOpen = true;
		selectedTileOccupied = true;
	}
	if (ImGui::Button("Blocker"))
	{
		MarkUnsavedChanges();
		selectedBlockerEnemy = dungeon->CreateEnemyBlocker(selectedTile->position, NORTH_INDEX);
		selectedBlockerEnemyWindowOpen = true;
		selectedTileOccupied = true;
	}
	ImGui::SameLine();
	bool maxChasers = (dungeon->chasers.size() > 1);
	if (maxChasers) ImGui::BeginDisabled();
	if (ImGui::Button("Chaser"))
	{
		MarkUnsavedChanges();
		selectedChaseEnemy = dungeon->CreateEnemyChase(selectedTile->position, NORTH_INDEX);
		selectedChaseEnemyWindowOpen = true;
		selectedTileOccupied = true;
	}
	if (maxChasers) ImGui::EndDisabled();
	ImGui::SameLine();
	if (ImGui::Button("Slug"))
	{
		MarkUnsavedChanges();
		selectedSlugEnemy = dungeon->CreateSlug(selectedTile->position, NORTH_INDEX);
		selectedSlugEnemyWindowOpen = true;
		selectedTileOccupied = true;
	}
	ImGui::SameLine();
	if (ImGui::Button("Switcher"))
	{
		MarkUnsavedChanges();
		selectedSwitcherEnemy = dungeon->CreateEnemySwitcher(selectedTile->position, NORTH_INDEX);
		selectedSwitcherEnemyWindowOpen = true;
		selectedTileOccupied = true;
	}
	if (tileOccupied) ImGui::EndDisabled();
	if (ImGui::Button("Decoration"))
	{
		MarkUnsavedChanges();
		selectedTileDecoration = dungeon->CreateDecoration(selectedTile->position, NORTH_INDEX);
		selectedTileDecorations.push_back(selectedTileDecoration);
		selectedDecorationWindowOpen = true;
	}
	
	bool cantMakeStairs = selectedStairs != nullptr;
	if (cantMakeStairs) ImGui::BeginDisabled();
	if (ImGui::Button("Stairs (Justin Only!)"))
	{
		MarkUnsavedChanges();
		selectedStairs = dungeon->CreateStairs(selectedTile->position);
		selectedStairsWindowOpen = true;
	}
	if (cantMakeStairs) ImGui::EndDisabled();

	ImGui::PopID();
	// Entity List
	ImGui::Text("Modify/Remove");
	ImGui::PushID("Modify");
	ImGui::Indent();
	for (int i = 0; i < selectedTileDoors.size(); i++)
	{
		ImGui::PushID(i);
		string doorName = "Door (ID: ";
		doorName += to_string(selectedTileDoors[i]->id);
		doorName += ")";
		if (ImGui::Selectable(doorName.c_str()))
		{
			selectedDoor = selectedTileDoors[i];
			selectedDoorWindowOpen = true;
		}
		ImGui::PopID();
	}
	for (int i = 0; i < selectedTileLevers.size(); i++)
	{
		ImGui::PushID(i);
		string leverName = "Lever (ID: ";
		leverName += to_string(selectedTileLevers[i]->id);
		leverName += ", Activates ID: ";
		leverName += to_string(selectedTileLevers[i]->activateID);
		leverName += ")";
		if (ImGui::Selectable(leverName.c_str()))
		{
			selectedLever = selectedTileLevers[i];
			selectedLeverWindowOpen = true;
		}
		ImGui::PopID();
	}
	for (int i = 0; i < selectedTileShootLasers.size(); i++)
	{
		ImGui::PushID(i);
		string shootLaserName = "shootLaser ID (";
		shootLaserName += to_string(selectedTileShootLasers[i]->id);
		shootLaserName += ")";
		if (ImGui::Selectable(shootLaserName.c_str()))
		{
			selectedTileShootLaser = selectedTileShootLasers[i];
			selectedShootLaserWindowOpen = true;
		}
		ImGui::PopID();
	}
	if (selectedActivatorPlate)
	{
		string plateName = "Plate (Activates ID: ";
		plateName += to_string(selectedActivatorPlate->activateID);
		plateName += ")";
		if (ImGui::Selectable(plateName.c_str())) selectedActivatorPlateWindowOpen = true;
	}
	if (selectedTransporter)
	{
		string transporterName = "Transporter (Name: ";
		transporterName += selectedTransporter->name;
		transporterName += ")";
		if (ImGui::Selectable(transporterName.c_str())) selectedTransporterWindowOpen = true;
	}
	if (selectedCheckpoint)
	{
		string checkpointName = "Checkpoint (Facing: ";
		checkpointName += orientationNames[selectedCheckpoint->facing];
		checkpointName += ")";
		if (ImGui::Selectable(checkpointName.c_str())) selectedCheckpointWindowOpen = true;
	}
	if (selectedHasSpikes)
	{	
		if (ImGui::Button("Delete Spikes"))
		{
			MarkUnsavedChanges();
			dungeon->RemoveSpikes(selectedTile->position);
			selectedHasSpikes = false;
		}
	}
	if (selectedBlockerEnemy)
	{
		if (ImGui::Selectable("Blocker")) selectedBlockerEnemyWindowOpen = true;
		ImGui::SameLine();
	}
	if (selectedChaseEnemy)
	{
		if (ImGui::Selectable("Chaser")) selectedChaseEnemyWindowOpen = true;
	}
	if (selectedSlugEnemy)
	{
		if (ImGui::Selectable("Slug")) selectedSlugEnemyWindowOpen = true;
	}
	if (selectedSwitcherEnemy)
	{
		if (ImGui::Selectable("Switcher")) selectedSwitcherEnemyWindowOpen = true;
	}
	if (selectedHasBlock)
	{
		if (ImGui::Button("Delete Block"))
		{
			MarkUnsavedChanges();
			dungeon->RemovePushableBlock(selectedTile->position);
			selectedHasBlock = false;
		}
	}
	if (selectedMirror)
	{
		if (ImGui::Selectable("Mirror")) selectedMirrorWindowOpen = true;
	}
	for (int i = 0; i < selectedTileDecorations.size(); i++)
	{
		ImGui::PushID(i);
		string decoName = "Decoration (";
		decoName += selectedTileDecorations[i]->modelName;
		decoName += ")";
		if (ImGui::Selectable(decoName.c_str()))
		{
			selectedTileDecoration = selectedTileDecorations[i];
			selectedDecorationWindowOpen = true;
		}
		ImGui::PopID();
	}

	if (selectedStairs)
	{
		if (ImGui::Selectable("Stairs")) selectedStairsWindowOpen = true;
	}

	ImGui::Unindent();
	ImGui::PopID();

	ImGui::PushID("EntityProperties");
	
	if (selectedDoorWindowOpen)
		DrawGUIModeTileEditDoor();
	if (selectedLeverWindowOpen)
		DrawGUIModeTileEditLever();
	if (selectedActivatorPlateWindowOpen)
		DrawGUIModeTileEditPlate();
	if (selectedTransporterWindowOpen)
		DrawGUIModeTileEditTransporter();
	if (selectedCheckpointWindowOpen)
		DrawGUIModeTileEditCheckpoint();
	if (selectedShootLaserWindowOpen)
		DrawGUIModeTileEditShootLaser();
	if (selectedBlockerEnemyWindowOpen)
		DrawGUIModeTileEditBlocker();
	if (selectedChaseEnemyWindowOpen)
		DrawGUIModeTileEditChase();
	if (selectedSlugEnemyWindowOpen)
		DrawGUIModeTileEditSlug();

	if (selectedSwitcherEnemyWindowOpen)
		DrawGUIModeTileEditSwitcher();
	if (selectedMirrorWindowOpen)
		DrawGUIModeTileEditMirror();
	if (selectedDecorationWindowOpen)
		DrawGUIModeTileEditDecoration();
	if (selectedStairsWindowOpen)
		DrawGUIModeTileEditStairs();

	ImGui::PopID();
}

void Crawl::DungeonEditor::DrawGUIModeTileEditDoor()
{
	ImGui::SetNextWindowPos({ 400,0 }, ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowSize({ 300, 150 }, ImGuiCond_FirstUseEver);
	ImGui::Begin("Edit Door", &selectedDoorWindowOpen);

	if(ImGui::InputInt("ID", (int*)&selectedDoor->id))
		MarkUnsavedChanges();

	if (ImGui::BeginCombo("Orientation", orientationNames[selectedDoor->orientation].c_str()))
	{
		int oldOrientation = selectedDoor->orientation;
		for (int i = 0; i < 4; i++)
			if (ImGui::Selectable(orientationNames[i].c_str()))
			{
				selectedDoor->orientation = i;
				if (selectedDoor->orientation != oldOrientation)
				{
					MarkUnsavedChanges();
					selectedDoor->object->SetLocalRotationZ(orientationEulers[i]);
				}
			}
		ImGui::EndCombo();
	}

	bool open = selectedDoor->open;
	if (ImGui::Checkbox("Is Open", &open))
	{
		MarkUnsavedChanges();
		selectedDoor->Toggle(); // this could go out of sync.
	}

	if (ImGui::Button("Delete"))
		ImGui::OpenPopup("delete_door_confirm");


	if (ImGui::BeginPopupModal("delete_door_confirm"))
	{
		ImGui::Text("Are you sure you want to delete the door?");
		if (ImGui::Button("Yes"))
		{
			MarkUnsavedChanges();
			dungeon->RemoveDoor(selectedDoor);
			selectedDoor = nullptr;
			selectedDoorWindowOpen = false;
			RefreshSelectedTile();
		}
		if (ImGui::Button("Cancel"))
		{
			ImGui::CloseCurrentPopup();
		}
		ImGui::EndPopup();
	}

	ImGui::End();
}
void Crawl::DungeonEditor::DrawGUIModeTileEditLever()
{
	ImGui::SetNextWindowPos({ 400,0 }, ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowSize({ 300, 170 }, ImGuiCond_FirstUseEver);
	ImGui::Begin("Edit Lever", &selectedLeverWindowOpen);

	if (ImGui::InputInt("ID", (int*)&selectedLever->id))
	{
		MarkUnsavedChanges();
		selectedLever->SetID(selectedLever->id);
	}

	if (ImGui::BeginCombo("Orientation", orientationNames[selectedLever->orientation].c_str()))
	{
		int oldOrientation = selectedLever->orientation;
		for (int i = 0; i < 4; i++)
		{
			if (ImGui::Selectable(orientationNames[i].c_str()))
			{
				if (selectedLever->orientation != i)
				{
					MarkUnsavedChanges();
					selectedLever->orientation = i;
					selectedLever->object->SetLocalRotationZ(orientationEulers[i]);
				}
			}
		}
		ImGui::EndCombo();
	}

	if (ImGui::Checkbox("Status", &selectedLever->status))
	{
		selectedLever->status = !selectedLever->status;
		selectedLever->Toggle(); // this could go out of sync.
	}

	if (ImGui::InputInt("Activate ID", (int*)&selectedLever->activateID))
		MarkUnsavedChanges();

	if (ImGui::Button("Delete"))
		ImGui::OpenPopup("delete_lever_confirm");


	if (ImGui::BeginPopupModal("delete_lever_confirm"))
	{
		ImGui::Text("Are you sure you want to delete the lever?");
		if (ImGui::Button("Yes"))
		{
			MarkUnsavedChanges();
			dungeon->RemoveLever(selectedLever);
			// remove from selected
			selectedLever = nullptr;
			selectedLeverWindowOpen = false;
			RefreshSelectedTile();
		}
		if (ImGui::Button("Cancel"))
		{
			ImGui::CloseCurrentPopup();
		}
		ImGui::EndPopup();
	}
	ImGui::End();
}
void Crawl::DungeonEditor::DrawGUIModeTileEditShootLaser()
{
	ImGui::SetNextWindowPos({ 400,0 }, ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowSize({ 350, 195 }, ImGuiCond_FirstUseEver);
	ImGui::Begin("Edit Shoot Laser", &selectedShootLaserWindowOpen);

	if (ImGui::InputInt("ID", (int*)&selectedTileShootLaser->id))
		MarkUnsavedChanges();

	if (ImGui::BeginCombo("Look Direction", orientationNames[selectedTileShootLaser->facing].c_str()))
	{
		int oldOrientation = selectedTileShootLaser->facing;
		for (int i = 0; i < 4; i++)
			if (ImGui::Selectable(orientationNames[i].c_str()))
			{
				selectedTileShootLaser->facing = (FACING_INDEX)i;
				if (selectedTileShootLaser->facing != oldOrientation)
				{
					MarkUnsavedChanges();
					selectedTileShootLaser->object->SetLocalRotationZ(orientationEulersReversed[i]);
				}
			}
		ImGui::EndCombo();
	}
	if (ImGui::Checkbox("Activates on Line of Sight", &selectedTileShootLaser->detectsLineOfSight))
		MarkUnsavedChanges();

	if (ImGui::Checkbox("Fires Projectile", &selectedTileShootLaser->firesProjectile))
		MarkUnsavedChanges();

	if (ImGui::Checkbox("Fires Immediately", &selectedTileShootLaser->firesImmediately))
		MarkUnsavedChanges();

	if (ImGui::InputInt("Activate ID", (int*)&selectedTileShootLaser->activateID))
		MarkUnsavedChanges();

	if (ImGui::Button("Delete"))
		ImGui::OpenPopup("delete_lever_confirm");


	if (ImGui::BeginPopupModal("delete_lever_confirm"))
	{
		ImGui::Text("Are you sure you want to delete the Shooter?");
		if (ImGui::Button("Yes"))
		{
			MarkUnsavedChanges();
			dungeon->RemoveDungeonShootLaser(selectedTileShootLaser);
			// remove from selected
			selectedTileShootLaser = nullptr;
			selectedShootLaserWindowOpen = false;
			RefreshSelectedTile();
		}
		if (ImGui::Button("Cancel"))
		{
			ImGui::CloseCurrentPopup();
		}
		ImGui::EndPopup();
	}

	ImGui::End();
}
void Crawl::DungeonEditor::DrawGUIModeTileEditPlate()
{
	ImGui::SetNextWindowPos({ 400,0 }, ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowSize({ 300, 100 }, ImGuiCond_FirstUseEver);
	ImGui::Begin("Edit Plate", &selectedActivatorPlateWindowOpen, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize);

	ImGui::InputInt("Activate ID", (int*)&selectedActivatorPlate->activateID);
	
	if (ImGui::Button("Delete"))
		ImGui::OpenPopup("delete_plate_confirm");


	if (ImGui::BeginPopupModal("delete_plate_confirm"))
	{
		ImGui::Text("Are you sure you want to delete the Plate?");
		if (ImGui::Button("Yes"))
		{
			MarkUnsavedChanges();
			dungeon->RemovePlate(selectedActivatorPlate);
			selectedActivatorPlate = nullptr;
			selectedActivatorPlateWindowOpen = false;
			RefreshSelectedTile();
		}
		if (ImGui::Button("Cancel"))
		{
			ImGui::CloseCurrentPopup();
		}
		ImGui::EndPopup();
	}

	ImGui::End();
}
void Crawl::DungeonEditor::DrawGUIModeTileEditTransporter()
{
	ImGui::SetNextWindowPos({ 400,0 }, ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowSize({ 500, 165 }, ImGuiCond_FirstUseEver);
	ImGui::Begin("Edit Transporter", &selectedTransporterWindowOpen, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize);
	if (ImGui::InputText("Name", &selectedTransporter->name))
		MarkUnsavedChanges();
	if (ImGui::BeginCombo("Orientation", orientationNames[selectedTransporter->fromOrientation].c_str()))
	{
		int oldOrientation = selectedTransporter->fromOrientation;
		for (int i = 0; i < 4; i++)
			if (ImGui::Selectable(orientationNames[i].c_str()))
			{
				selectedTransporter->fromOrientation = i;
				if (selectedTransporter->fromOrientation != oldOrientation)
				{
					MarkUnsavedChanges();
					selectedTransporter->object->SetLocalRotationZ(orientationEulers[i]);
				}
			}
		ImGui::EndCombo();
	}

	if (ImGui::BeginCombo("To Dungeon", selectedTransporter->toDungeon.c_str()))
	{
		for (auto d : fs::recursive_directory_iterator(dungeon->dungeonFileLocation))
		{
			bool selected = false;
			string folder = d.path().relative_path().parent_path().string();
			if (d.path().has_extension() && d.path().extension() == dungeon->dungeonFileExtension)
			{
				string foundDungeonPath = folder + "/" + d.path().stem().string();
				if (foundDungeonPath == selectedTransporter->toDungeon) selected = true;
				if (ImGui::Selectable(foundDungeonPath.c_str(), selected))
				{
					if (!selected)
					{
						selectedTransporter->toDungeon = foundDungeonPath;
						RefreshSelectedTransporterData(foundDungeonPath + dungeon->dungeonFileExtension);
						MarkUnsavedChanges();
					}
				}
				if (selected) ImGui::SetItemDefaultFocus();
			}
		}
		ImGui::EndCombo();
	}

	if (!selectedTransporterToDungeonLoaded) ImGui::BeginDisabled();
	if (ImGui::BeginCombo("To Transporter", selectedTransporter->toTransporter.c_str()))
	{
		for (auto& transporter : selectedTransporterToDungeonJSON["transporters"])
		{
			bool selected = false;
			string transporterName;
			transporter["name"].get_to(transporterName);
			if (selectedTransporter->toTransporter == transporterName) selected = true;
			if (ImGui::Selectable(transporterName.c_str(), selected))
			{
				if (!selected)
				{
					selectedTransporter->toTransporter = transporterName;
					MarkUnsavedChanges();
				}
			}
			if (selected) ImGui::SetItemDefaultFocus();
		}
		ImGui::EndCombo();
	}
	if (!selectedTransporterToDungeonLoaded) ImGui::EndDisabled();

	if (ImGui::Button("Delete"))
		ImGui::OpenPopup("delete_transporter_confirm");
	if (ImGui::BeginPopupModal("delete_transporter_confirm"))
	{
		ImGui::Text("Are you sure you want to delete the Transporter?");
		if (ImGui::Button("Yes"))
		{
			MarkUnsavedChanges();
			dungeon->RemoveTransporter(selectedTransporter);
			selectedTransporter = nullptr;
			selectedTransporterWindowOpen = false;
			RefreshSelectedTile();
		}
		if (ImGui::Button("Cancel"))
		{
			ImGui::CloseCurrentPopup();
		}
		ImGui::EndPopup();
	}

	ImGui::End();
}
void Crawl::DungeonEditor::DrawGUIModeTileEditCheckpoint()
{
	ImGui::SetNextWindowPos({ 400,0 }, ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowSize({ 300, 100 }, ImGuiCond_FirstUseEver);
	ImGui::Begin("Edit Checkpoint", &selectedActivatorPlateWindowOpen, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize);

	if (ImGui::BeginCombo("Spawn Direction", orientationNames[selectedCheckpoint->facing].c_str()))
	{
		int oldOrientation = selectedCheckpoint->facing;
		for (int i = 0; i < 4; i++)
			if (ImGui::Selectable(orientationNames[i].c_str()))
			{
				selectedCheckpoint->facing = (FACING_INDEX)i;
				if (selectedCheckpoint->facing != oldOrientation)
				{
					MarkUnsavedChanges();
					selectedCheckpoint->object->SetLocalRotationZ(orientationEulers[i]);
				}
			}
		ImGui::EndCombo();
	}

	if (ImGui::Button("Delete"))
		ImGui::OpenPopup("Delete Checkpoint?");


	if (ImGui::BeginPopupModal("Delete Checkpoint?"))
	{
		ImGui::Text("Are you sure you want to delete the Checkpoint?");
		if (ImGui::Button("Yes"))
		{
			MarkUnsavedChanges();
			dungeon->RemoveCheckpoint(selectedCheckpoint);
			selectedCheckpoint = nullptr;
			selectedCheckpointWindowOpen = false;
		}
		if (ImGui::Button("Cancel"))
		{
			ImGui::CloseCurrentPopup();
		}
		ImGui::EndPopup();
	}

	ImGui::End();
}
void Crawl::DungeonEditor::DrawGUIModeTileEditBlocker()
{
	ImGui::SetNextWindowPos({ 400,0 }, ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowSize({ 300, 150 }, ImGuiCond_FirstUseEver);
	ImGui::Begin("Edit Blocker Enemy", &selectedBlockerEnemyWindowOpen);

	if (ImGui::BeginCombo("Look Direction", orientationNames[selectedBlockerEnemy->facing].c_str()))
	{
		int oldOrientation = selectedBlockerEnemy->facing;
		for (int i = 0; i < 4; i++)
			if (ImGui::Selectable(orientationNames[i].c_str()))
			{
				selectedBlockerEnemy->facing = (FACING_INDEX)i;
				if (selectedBlockerEnemy->facing != oldOrientation)
				{
					MarkUnsavedChanges();
					selectedBlockerEnemy->object->SetLocalRotationZ(orientationEulersReversed[i]);
				}
			}
		ImGui::EndCombo();
	}
	if (ImGui::Checkbox("Rapid Attack", &selectedBlockerEnemy->rapidAttack))
		MarkUnsavedChanges();

	if (ImGui::Button("Delete"))
		ImGui::OpenPopup("delete_blocker_confirm");
	if (ImGui::BeginPopupModal("delete_blocker_confirm"))
	{
ImGui::Text("Are you sure you want to delete the Blocker?");
if (ImGui::Button("Yes"))
{
	MarkUnsavedChanges();
	dungeon->RemoveEnemyBlocker(selectedBlockerEnemy);
	selectedBlockerEnemy = nullptr;
	selectedBlockerEnemyWindowOpen = false;
	RefreshSelectedTile();
}
if (ImGui::Button("Cancel"))
{
	ImGui::CloseCurrentPopup();
}
ImGui::EndPopup();
	}

	ImGui::End();
}
void Crawl::DungeonEditor::DrawGUIModeTileEditChase()
{
	ImGui::SetNextWindowPos({ 400,0 }, ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowSize({ 300, 150 }, ImGuiCond_FirstUseEver);
	ImGui::Begin("Edit Chase Enemy", &selectedChaseEnemyWindowOpen);

	if (ImGui::BeginCombo("Look Direction", orientationNames[selectedChaseEnemy->facing].c_str()))
	{
		int oldOrientation = selectedChaseEnemy->facing;
		for (int i = 0; i < 4; i++)
			if (ImGui::Selectable(orientationNames[i].c_str()))
			{
				selectedChaseEnemy->facing = (FACING_INDEX)i;
				if (selectedChaseEnemy->facing != oldOrientation)
				{
					MarkUnsavedChanges();
					selectedChaseEnemy->object->SetLocalRotationZ(orientationEulers[i]);
				}
			}
		ImGui::EndCombo();
	}

	bool startActive = selectedChaseEnemy->state == Crawl::DungeonEnemyChase::IDLE;
	if (ImGui::Checkbox("Start Active", &startActive))
	{
		MarkUnsavedChanges();
		if (startActive)
			selectedChaseEnemy->state = Crawl::DungeonEnemyChase::IDLE;
		else
			selectedChaseEnemy->state = Crawl::DungeonEnemyChase::INACTIVE;
	}

	if (ImGui::Button("Delete"))
		ImGui::OpenPopup("delete_chaser_confirm");
	if (ImGui::BeginPopupModal("delete_chaser_confirm"))
	{
		ImGui::Text("Are you sure you want to delete the Chaser?");
		if (ImGui::Button("Yes"))
		{
			MarkUnsavedChanges();
			dungeon->RemoveEnemyChase(selectedChaseEnemy);
			selectedChaseEnemy = nullptr;
			selectedChaseEnemyWindowOpen = false;
			RefreshSelectedTile();
		}
		if (ImGui::Button("Cancel"))
		{
			ImGui::CloseCurrentPopup();
		}
		ImGui::EndPopup();
	}

	ImGui::End();
}
void Crawl::DungeonEditor::DrawGUIModeTileEditSlug()
{
	ImGui::SetNextWindowPos({ 400,0 }, ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowSize({ 300, 150 }, ImGuiCond_FirstUseEver);
	ImGui::Begin("Edit Slug Enemy", &selectedSlugEnemyWindowOpen);

	if (ImGui::BeginCombo("Look Direction", orientationNames[selectedSlugEnemy->facing].c_str()))
	{
		int oldOrientation = selectedSlugEnemy->facing;
		for (int i = 0; i < 4; i++)
			if (ImGui::Selectable(orientationNames[i].c_str()))
			{
				selectedSlugEnemy->facing = (FACING_INDEX)i;
				if (selectedSlugEnemy->facing != oldOrientation)
				{
					MarkUnsavedChanges();
					selectedSlugEnemy->object->SetLocalRotationZ(orientationEulers[i]);
				}
			}
		ImGui::EndCombo();
	}

	if (ImGui::Button("Delete"))
		ImGui::OpenPopup("delete_chaser_confirm");
	if (ImGui::BeginPopupModal("delete_chaser_confirm"))
	{
		ImGui::Text("Are you sure you want to delete the Slug?");
		if (ImGui::Button("Yes"))
		{
			MarkUnsavedChanges();
			dungeon->RemoveSlug(selectedSlugEnemy);
			selectedSlugEnemy = nullptr;
			selectedSlugEnemyWindowOpen = false;
			RefreshSelectedTile();
		}
		if (ImGui::Button("Cancel"))
		{
			ImGui::CloseCurrentPopup();
		}
		ImGui::EndPopup();
	}

	ImGui::End();
}
void Crawl::DungeonEditor::DrawGUIModeTileEditDecoration()
{
	ImGui::SetNextWindowPos({ 400,0 }, ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowSize({ 300, 150 }, ImGuiCond_FirstUseEver);
	ImGui::Begin("Edit Decoration", &selectedDecorationWindowOpen);

	if (ImGui::BeginCombo("Select Decoration", selectedTileDecoration->modelName.c_str()))
	{
		for (auto& path : decorations)
		{
			bool selected = selectedTileDecoration->modelName.c_str() == path.c_str();
			if (ImGui::Selectable(path.c_str(), selected))
			{
				MarkUnsavedChanges();
				selectedTileDecoration->modelName = path;
				selectedTileDecoration->LoadDecoration();
			}
		}
		ImGui::EndCombo();
	}

	if (ImGui::BeginCombo("Forward", orientationNames[selectedTileDecoration->facing].c_str()))
	{
		int oldOrientation = selectedTileDecoration->facing;
		for (int i = 0; i < 4; i++)
			if (ImGui::Selectable(orientationNames[i].c_str()))
			{
				selectedTileDecoration->facing = (FACING_INDEX)i;
				if (selectedTileDecoration->facing != oldOrientation)
				{
					MarkUnsavedChanges();
					selectedTileDecoration->object->SetLocalRotationZ(orientationEulersReversed[i]);
				}
			}
		ImGui::EndCombo();
	}

	if (ImGui::DragFloat3("XYZ", &selectedTileDecoration->localPosition.x,0.1f, -5, 5, "%.3f", ImGuiSliderFlags_AlwaysClamp))
	{
		MarkUnsavedChanges();
		selectedTileDecoration->UpdateTransform();
	}

	if (ImGui::Button("Delete"))
		ImGui::OpenPopup("delete_decoration_confirm");


	if (ImGui::BeginPopupModal("delete_decoration_confirm"))
	{
		ImGui::Text("Are you sure you want to delete the decoration?");
		if (ImGui::Button("Yes"))
		{
			MarkUnsavedChanges();
			dungeon->RemoveDecoration(selectedTileDecoration);
			selectedTileDecoration = nullptr;
			selectedDecorationWindowOpen = false;
			RefreshSelectedTile();
		}
		if (ImGui::Button("Cancel"))
		{
			ImGui::CloseCurrentPopup();
		}
		ImGui::EndPopup();
	}

	ImGui::End();
}
void Crawl::DungeonEditor::DrawGUIModeTileEditStairs()
{
	ImGui::SetNextWindowPos({ 400,0 }, ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowSize({ 450, 300 }, ImGuiCond_FirstUseEver);
	ImGui::Begin("Edit Stairs", &selectedStairsWindowOpen);
	ImGui::BeginDisabled();
	ImGui::InputInt2("Start Position", &selectedStairs->startPosition.x);
	ImGui::EndDisabled();
	if(ImGui::InputInt2("End Position", &selectedStairs->endPosition.x)) MarkUnsavedChanges();
	if(ImGui::InputInt("Direction Start", (int*)&selectedStairs->directionStart)) MarkUnsavedChanges();
	if(ImGui::InputInt("Direction End", (int*)&selectedStairs->directionEnd)) MarkUnsavedChanges();
	if(ImGui::Checkbox("Stairs go up", &selectedStairs->up)) MarkUnsavedChanges();
	ImGui::Spacing();
	if(ImGui::DragFloat3("Start World Position", &selectedStairs->startWorldPosition.x)) MarkUnsavedChanges();
	if(ImGui::DragFloat3("Start Offset", &selectedStairs->startOffset.x)) MarkUnsavedChanges();
	if(ImGui::DragFloat3("End World Position", &selectedStairs->endWorldPosition.x)) MarkUnsavedChanges();
	if(ImGui::DragFloat3("End Offset", &selectedStairs->endOffset.x)) MarkUnsavedChanges();
	if (ImGui::Button("Generate World Positions"))
	{
		selectedStairs->BuildDefaultSpline();
		MarkUnsavedChanges();
	}
	if (ImGui::InputInt("Gizmo Edit", &selectedStairsGizmoIndex))
	{
		selectedStairsGizmoIndex = glm::clamp(selectedStairsGizmoIndex, 0, 3);
	}
	ImGui::End();

	// Send the curve to the line renderer
	for (float i = 0; i < 0.98; i += 0.02f)
	{
		vec3 a = selectedStairs->EvaluatePosition(i);
		if(!selectedStairs->up) a.z -= 3.0f;
		vec3 b = selectedStairs->EvaluatePosition(i + 0.02f);
		if (!selectedStairs->up) b.z -= 3.0f;
		LineRenderer::DrawLine(a, b, glm::vec3((i/2) + 0.5f));
	}
	// send the anchors in to the line renderer
	LineRenderer::DrawLine(selectedStairs->startWorldPosition, selectedStairs->startWorldPosition + selectedStairs->startOffset);
	LineRenderer::DrawLine(selectedStairs->endWorldPosition, selectedStairs->endWorldPosition + selectedStairs->endOffset);

	// Render Gizmo
	ImGuizmo::SetRect(0, 0, Window::GetViewPortSize().x, Window::GetViewPortSize().y);
	mat4 view = Scene::GetCurrentCamera()->GetViewMatrix();
	mat4 projection = Scene::GetCurrentCamera()->GetProjectionMatrix();
	mat4 translation;
	switch (selectedStairsGizmoIndex)
	{
	case 0:
		translation = glm::translate(mat4(1), selectedStairs->startWorldPosition);
		break;
	case 1:
		translation = glm::translate(mat4(1), selectedStairs->startWorldPosition + selectedStairs->startOffset);
		break;
	case 2:
		translation = glm::translate(mat4(1), selectedStairs->endWorldPosition);
		break;
	case 3:
		translation = glm::translate(mat4(1), selectedStairs->endWorldPosition + selectedStairs->endOffset);
		break;
	}

	if (ImGuizmo::Manipulate((float*)&view, (float*)&projection, ImGuizmo::TRANSLATE, ImGuizmo::LOCAL, (float*)&translation))
	{
		switch (selectedStairsGizmoIndex)
		{
		case 0:
			selectedStairs->startWorldPosition = translation[3];
			break;
		case 1:
			selectedStairs->startOffset = (vec3)translation[3] - selectedStairs->startWorldPosition;
			break;
		case 2:
			selectedStairs->endWorldPosition = translation[3];
			break;
		case 3:
			selectedStairs->endOffset = (vec3)translation[3] - selectedStairs->endWorldPosition;
			break;
		}
	}
}
void Crawl::DungeonEditor::DrawGUIModeTileEditSwitcher()
{
	ImGui::SetNextWindowPos({ 400,0 }, ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowSize({ 300, 150 }, ImGuiCond_FirstUseEver);
	ImGui::Begin("Edit Switcher Enemy", &selectedSwitcherEnemyWindowOpen);

	if (ImGui::BeginCombo("Look Direction", orientationNames[selectedSwitcherEnemy->facing].c_str()))
	{
		int oldOrientation = selectedSwitcherEnemy->facing;
		for (int i = 0; i < 4; i++)
			if (ImGui::Selectable(orientationNames[i].c_str()))
			{
				selectedSwitcherEnemy->facing = (FACING_INDEX)i;
				if (selectedSwitcherEnemy->facing != oldOrientation)
				{
					MarkUnsavedChanges();
					selectedSwitcherEnemy->object->SetLocalRotationZ(orientationEulers[i]);
				}
			}
		ImGui::EndCombo();
	}

	if (ImGui::Button("Delete"))
		ImGui::OpenPopup("delete_switcher_confirm");
	if (ImGui::BeginPopupModal("delete_switcher_confirm"))
	{
		ImGui::Text("Are you sure you want to delete the Switcher?");
		if (ImGui::Button("Yes"))
		{
			MarkUnsavedChanges();
			dungeon->RemoveEnemySwitcher(selectedSwitcherEnemy);
			selectedSwitcherEnemy = nullptr;
			selectedSwitcherEnemyWindowOpen = false;
			selectedTileOccupied = false;
			RefreshSelectedTile();
		}
		if (ImGui::Button("Cancel"))
		{
			ImGui::CloseCurrentPopup();
		}
		ImGui::EndPopup();
	}
}
void Crawl::DungeonEditor::DrawGUIModeTileEditMirror()
{
	ImGui::SetNextWindowPos({ 400,0 }, ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowSize({ 300, 100 }, ImGuiCond_FirstUseEver);
	ImGui::Begin("Edit Mirror", &selectedMirrorWindowOpen, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize);

	if (ImGui::BeginCombo("Spawn Direction", orientationNames[selectedMirror->facing].c_str()))
	{
		int oldOrientation = selectedMirror->facing;
		for (int i = 0; i < 4; i++)
			if (ImGui::Selectable(orientationNames[i].c_str()))
			{
				selectedMirror->facing = (FACING_INDEX)i;
				if (selectedMirror->facing != oldOrientation)
				{
					MarkUnsavedChanges();
					selectedMirror->object->SetLocalRotationZ(orientationEulers[i]);
				}
			}
		ImGui::EndCombo();
	}

	if (ImGui::Button("Delete"))
		ImGui::OpenPopup("Delete Mirror?");


	if (ImGui::BeginPopupModal("Delete Mirror?"))
	{
		ImGui::Text("Are you sure you want to delete the Mirror?");
		if (ImGui::Button("Yes"))
		{
			MarkUnsavedChanges();
			dungeon->RemoveMirror(selectedMirror);
			selectedMirror = nullptr;
			selectedMirrorWindowOpen = false;
			selectedTileOccupied = false;
		}
		if (ImGui::Button("Cancel"))
		{
			ImGui::CloseCurrentPopup();
		}
		ImGui::EndPopup();
	}

	ImGui::End();
}
void Crawl::DungeonEditor::DrawGUIModeDungeonProperties()
{
	if(ImGui::InputInt2("Default Position", &dungeon->defaultPlayerStartPosition.x))
		MarkUnsavedChanges();

	if (ImGui::BeginCombo("Default Orientation", orientationNames[dungeon->defaultPlayerStartOrientation].c_str()))
	{
		int oldOrientation = dungeon->defaultPlayerStartOrientation;
		for (int i = 0; i < 4; i++)
			if (ImGui::Selectable(orientationNames[i].c_str()))
			{
				dungeon->defaultPlayerStartOrientation = (FACING_INDEX)i;
				if (dungeon->defaultPlayerStartOrientation != oldOrientation)
					MarkUnsavedChanges();
			}
		ImGui::EndCombo();
	}

	if (ImGui::Checkbox("Player Turn is Free (Default is True)", &dungeon->playerTurnIsFree))
		MarkUnsavedChanges();

	if (ImGui::Checkbox("Player Interact is Free (Default is False)", &dungeon->playerInteractIsFree))
		MarkUnsavedChanges();

	/*if (ImGui::Checkbox("Player can Knife (Default is False)", &dungeon->playerHasKnife))
		MarkUnsavedChanges();*/

	if (ImGui::Checkbox("Player can Kick (Default is True)", &dungeon->playerCanKickBox))
		MarkUnsavedChanges();

	if (ImGui::Checkbox("Player Can Push Box (Default is False)", &dungeon->playerCanPushBox))
		MarkUnsavedChanges();

	if (ImGui::Checkbox("Switchers Must Be Looked At (Default is False)", &dungeon->switchersMustBeLookedAt))
		MarkUnsavedChanges();

	if (ImGui::Checkbox("Player Can Push Mirrors (Default is False)", &dungeon->playerCanPushMirror))
		MarkUnsavedChanges();


	if (ImGui::Button("Beauty Scene Quick Config"))
	{
		if (beautySceneLights == nullptr)
		{
			beautySceneLights = Scene::CreateObject();
			beautySceneLights->LoadFromJSON(ReadJSONFromDisk("crawler/object/beautySceneLights.object"));
		}

		Scene::s_instance->drawGizmos = false;

		Scene::s_editorCamera->object->SetLocalPosition({ -2.970f, 1.447f, 1.694f });
		Scene::s_editorCamera->object->SetLocalRotationZ(-130.2f);
		Scene::s_editorCamera->object->SetLocalRotationX(-2.7f);

		dungeon->player->Respawn();
	}
}

void Crawl::DungeonEditor::Update()
{
	// Draw Player Position
	vec3 playerWorldPosition = dungeonPosToObjectScale(dungeon->player->GetPosition());
	LineRenderer::DrawLine(playerWorldPosition, playerWorldPosition + vec3(0, 0, 2));

	switch (editMode)
	{
	case Mode::TileBrush:
		UpdateModeTileBrush();	break;
	case Mode::TileEdit:
		UpdateModeTileEdit();	break;
	case Mode::SlugPathEditor:
		UpdateModeSlugPathEdit();	break;
	case Mode::DungeonProperties:
	{
		for (auto& column : dungeon->tiles)
		{
			for (auto& dungeonTile : column.second.row)
			{
				DrawTileInformation(&dungeonTile.second);
			}
		}
		break;
	}
	default:
		break;
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
			MarkUnsavedChanges();
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
				tile->maskTraverse = brush_tileMask;
				tile->maskSee = brush_tileMask;
				if ((tile->maskTraverse & NORTH_MASK) != NORTH_MASK) // North Wall
					tile->wallVariants[0] = 0;
				if ((tile->maskTraverse & EAST_MASK) != EAST_MASK) // East Wall
					tile->wallVariants[1] = 0;
				if ((tile->maskTraverse & SOUTH_MASK) != SOUTH_MASK) // South Wall
					tile->wallVariants[2] = 0;
				if ((tile->maskTraverse & WEST_MASK) != WEST_MASK) // West Wall
					tile->wallVariants[3] = 0;
				dungeon->CreateTileObject(tile);
			}
		}
	}

	if (Input::Mouse(2).Pressed())
	{
		if (dungeon->DeleteTile(gridSelected))
		{
			MarkUnsavedChanges();
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
	if (selectedTile)
	{
		DrawTileInformation(selectedTile);

		// Highlight the tile
		vec3 tilePos = dungeonPosToObjectScale(selectedTile->position);
		for (int i = 0; i < 4; i++)
		{	
			int nextI = i+1;
			if (nextI == 4) nextI = 0;
			vec3 a = dungeonPosToObjectScale(directionsDiagonal[i]) * 0.5f;;
			vec3 b = dungeonPosToObjectScale(directionsDiagonal[nextI]) * 0.5f;;

			LineRenderer::DrawLine(tilePos + a, tilePos + b);
		}
	}


	if (Input::Mouse(0).Down())
	{
		TileEditUnselectAll();
		glm::ivec2 selectionPos = GetMousePosOnGrid();

		selectedTile = dungeon->GetTile(selectionPos);
		if (!selectedTile)
			return;
		
		RefreshSelectedTile();
	}

	// path finding dev stuff
	/*if (Input::Keyboard(GLFW_KEY_LEFT_CONTROL).Down())
	{
		from = GetMousePosOnGrid();
		to = dungeon->player->GetPosition();
		LogUtils::Log("Path finding with free turns");
		for (auto& o : pathFindObjects)
			o->markedForDeletion = true;
		pathFindObjects.clear();

		dungeon->FindPath(from, to);
		for (auto& p : dungeon->goodPath)
		{
			Object* o = Scene::CreateObject();
			o->LoadFromJSON(path_template);
			o->AddLocalPosition({ p->position.x * DUNGEON_GRID_SCALE, p->position.y * DUNGEON_GRID_SCALE, 0 });
			pathFindObjects.push_back(o);
		}
	}*/

	/*if (Input::Keyboard(GLFW_KEY_SPACE).Down())
	{
		from = GetMousePosOnGrid();
		to = dungeon->player->GetPosition();
		LogUtils::Log("Path finding with turn cost");
		for (auto& o : pathFindObjects)
			o->markedForDeletion = true;
		pathFindObjects.clear();

		dungeon->FindPath(from, to, facingTest);
		for (auto& p : dungeon->goodPath)
		{
			Object* o = Scene::CreateObject();
			o->LoadFromJSON(path_template);
			o->AddLocalPosition({ p->position.x * DUNGEON_GRID_SCALE, p->position.y * DUNGEON_GRID_SCALE, 0 });
			pathFindObjects.push_back(o);
		}
	}*/
}
void Crawl::DungeonEditor::UpdateModeSlugPathEdit()
{
	if (slugPathCursor == nullptr)
	{
		slugPathCursor = Scene::CreateObject();
		slugPathCursor->LoadFromJSON(ReadJSONFromDisk("crawler/object/prototype/slug_rail_nothing.object"));
	}
	
	gridSelected = GetMousePosOnGrid();
	slugPathCursor->SetLocalPosition(dungeonPosToObjectScale(gridSelected));

	if (Input::Mouse(0).Down())
	{
		if (dungeon->CreateSlugPath(gridSelected))
			MarkUnsavedChanges();
	}

	if (Input::Mouse(2).Down())
	{
		DungeonEnemySlugPath* toDelete = dungeon->GetSlugPath(gridSelected);
		if (toDelete)
		{
			dungeon->RemoveSlugPath(toDelete);
			MarkUnsavedChanges();
		}
	}
}

void Crawl::DungeonEditor::DrawTileInformation(DungeonTile* tile)
{
	glm::vec3 tileOrigin = dungeonPosToObjectScale(tile->position);
	for (int i = 0; i < 4; i++)
		if ((tile->maskTraverse & orientationMasksIndex[i]) == orientationMasksIndex[i]) LineRenderer::DrawLine(tileOrigin, tileOrigin + dungeonPosToObjectScale(directions[i]) * 0.5f, { 0,1,0 });

	tileOrigin.z += 0.2f;
	for (int i = 0; i < 4; i++)
		if ((tile->maskSee & orientationMasksIndex[i]) == orientationMasksIndex[i]) LineRenderer::DrawLine(tileOrigin, tileOrigin + dungeonPosToObjectScale(directions[i]) * 0.5f, { 0.5,.5,1 });
}

void Crawl::DungeonEditor::RefreshSelectedTile()
{
	selectedTileUntraversableWalls[0] = (selectedTile->maskTraverse & NORTH_MASK) == NORTH_MASK; // North Check
	selectedTileUntraversableWalls[1] = (selectedTile->maskTraverse & SOUTH_MASK) == SOUTH_MASK; // South Check
	selectedTileUntraversableWalls[2] = (selectedTile->maskTraverse & EAST_MASK) == EAST_MASK; // East Check
	selectedTileUntraversableWalls[3] = (selectedTile->maskTraverse & WEST_MASK) == WEST_MASK; // West Check

	selectedTileSeeThroughWalls[0] = (selectedTile->maskSee & NORTH_MASK) == NORTH_MASK; // North Check
	selectedTileSeeThroughWalls[1] = (selectedTile->maskSee & SOUTH_MASK) == SOUTH_MASK; // South Check
	selectedTileSeeThroughWalls[2] = (selectedTile->maskSee & EAST_MASK) == EAST_MASK; // East Check
	selectedTileSeeThroughWalls[3] = (selectedTile->maskSee & WEST_MASK) == WEST_MASK; // West Check

	selectedTileOccupied = false;

	// update list of tiles doors and levers / interactabes
	selectedTileDoors.clear();
	for (int i = 0; i < dungeon->activatable.size(); i++)
	{
		if (dungeon->activatable[i]->position == selectedTile->position)
			selectedTileDoors.push_back(dungeon->activatable[i]);
	}

	selectedTileLevers.clear();
	for (int i = 0; i < dungeon->interactables.size(); i++)
	{
		if (dungeon->interactables[i]->position == selectedTile->position)
			selectedTileLevers.push_back(dungeon->interactables[i]);
	}

	selectedTileShootLasers.clear();
	for (int i = 0; i < dungeon->shootLasers.size(); i++)
	{
		if (dungeon->shootLasers[i]->position == selectedTile->position)
			selectedTileShootLasers.push_back(dungeon->shootLasers[i]);
	}

	selectedActivatorPlate = nullptr;
	for (int i = 0; i < dungeon->activatorPlates.size(); i++)
	{
		if (dungeon->activatorPlates[i]->position == selectedTile->position)
		{
			selectedActivatorPlate = dungeon->activatorPlates[i];
			selectedTileOccupied = true;
		}
	}

	selectedTransporter = nullptr;
	for (int i = 0; i < dungeon->transporterPlates.size(); i++)
	{
		if (dungeon->transporterPlates[i]->position == selectedTile->position)
		{
			selectedTransporter = dungeon->transporterPlates[i];
			selectedTileOccupied = true;
			RefreshSelectedTransporterData(selectedTransporter->toDungeon + dungeon->dungeonFileExtension);
		}
	}

	selectedCheckpoint = dungeon->GetCheckpointAt(selectedTile->position); // holy smokes this is a little better!
	if (selectedCheckpoint)
		selectedTileOccupied = true;

	selectedHasSpikes = false;
	for (int i = 0; i < dungeon->spikesPlates.size(); i++)
	{
		if (dungeon->spikesPlates[i]->position == selectedTile->position)
		{
			selectedHasSpikes = true;
			selectedTileOccupied = true;
		}
	}

	selectedBlockerEnemy = nullptr;
	for (int i = 0; i < dungeon->blockers.size(); i++)
	{
		if (dungeon->blockers[i]->position == selectedTile->position)
		{
			selectedBlockerEnemy = dungeon->blockers[i];
			selectedTileOccupied = true;
		}
	}

	selectedChaseEnemy = nullptr;
	for (int i = 0; i < dungeon->chasers.size(); i++)
	{
		if (dungeon->chasers[i]->position == selectedTile->position)
		{
			selectedChaseEnemy = dungeon->chasers[i];
			selectedTileOccupied = true;
		}
	}

	selectedSlugEnemy = nullptr;
	for (int i = 0; i < dungeon->slugs.size(); i++)
	{
		if (dungeon->slugs[i]->position == selectedTile->position)
		{
			selectedSlugEnemy = dungeon->slugs[i];
			selectedTileOccupied = true;
		}
	}

	selectedSwitcherEnemy = nullptr;
	for (int i = 0; i < dungeon->switchers.size(); i++)
	{
		if (dungeon->switchers[i]->position == selectedTile->position)
		{
			selectedSwitcherEnemy = dungeon->switchers[i];
			selectedTileOccupied = true;
		}
	}

	selectedHasBlock = false;
	for (int i = 0; i < dungeon->pushableBlocks.size(); i++)
	{
		if (dungeon->pushableBlocks[i]->position == selectedTile->position)
		{
			selectedHasBlock = true;
			selectedTileOccupied = true;
		}
	}

	selectedMirror = nullptr;
	for (int i = 0; i < dungeon->mirrors.size(); i++)
	{
		if (dungeon->mirrors[i]->position == selectedTile->position)
		{
			selectedMirror = dungeon->mirrors[i];
			selectedTileOccupied = true;
		}
	}

	selectedTileDecorations.clear();
	for (int i = 0; i < dungeon->decorations.size(); i++)
	{
		if (dungeon->decorations[i]->position == selectedTile->position)
			selectedTileDecorations.push_back(dungeon->decorations[i]);
	}

	selectedStairs = nullptr;
	for (int i = 0; i < dungeon->stairs.size(); i++)
	{
		if (dungeon->stairs[i]->startPosition == selectedTile->position)
			selectedStairs = dungeon->stairs[i];
	}
}
void Crawl::DungeonEditor::RefreshSelectedTransporterData(string dungeonPath)
{
	selectedTransporterToDungeonJSON.clear();
	if (fs::exists(dungeonPath))
	{
		selectedTransporterToDungeonJSON = ReadJSONFromDisk(dungeonPath);
		selectedTransporterToDungeonLoaded = true;
	}
	else LogUtils::Log("Warning: ToDungeon on selected Transporter does not exist");
}
void Crawl::DungeonEditor::RefreshAvailableDecorations()
{
	decorations.clear();
	string folder = "crawler/model/";
	for (auto d : fs::recursive_directory_iterator(folder))
	{
		if (d.path().extension() == ".object")
		{
			if(d.path().string().find("decoration") != string::npos) decorations.push_back(d.path().generic_string());
		}
	}
}

void Crawl::DungeonEditor::RefreshDungeonFileNames()
{
	dungeonFileName = "";
	if ((dungeon->dungeonSubFolder.size() > 0)) dungeonFileName += dungeon->dungeonSubFolder + "/";
	dungeonFileName += dungeon->dungeonFileName;
	dungeonFileNameSaveAs = dungeonFileName;
	dungeonFilePath = dungeon->dungeonFilePath;
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
		for (int i = 0; i < dungeon->activatable.size(); i++)
		{
			if (dungeon->activatable[i]->id == nextAvail)
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
	vec3 rayStart = Scene::s_editorCamera->object->GetWorldSpacePosition();
	vec3 rayDir = Scene::s_editorCamera->camera->GetRayFromNDC(NDC);
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

	brushObject->SetLocalPosition({ gridSelected.x * DUNGEON_GRID_SCALE, gridSelected.y * DUNGEON_GRID_SCALE, 0 });
}

void Crawl::DungeonEditor::UpdateAutoTile(ivec2 position)
{
	DungeonTile* tile = dungeon->GetTile(position);

	if (tile != nullptr)
	{
		tile->maskTraverse = dungeon->GetAutoTileMask(tile->position);
		tile->maskSee = tile->maskTraverse;
		SetDefaultWallVarients(tile);
		dungeon->CreateTileObject(tile);
	}
}

void Crawl::DungeonEditor::SetDefaultWallVarients(DungeonTile* tile)
{
	tile->wallVariants[0] = (tile->maskTraverse & NORTH_MASK) == NORTH_MASK ? -1 : 0;
	tile->wallVariants[1] = (tile->maskTraverse & EAST_MASK) == EAST_MASK ? -1 : 0;
	tile->wallVariants[2] = (tile->maskTraverse & SOUTH_MASK) == SOUTH_MASK ? -1 : 0;
	tile->wallVariants[3] = (tile->maskTraverse & WEST_MASK) == WEST_MASK ? -1 : 0;
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
	dungeonFilePath = dungeon->dungeonFileLocation + dungeonFileNameSaveAs + dungeon->dungeonFileExtension;
	dungeon->Save(dungeonFilePath);
	RefreshDungeonFileNames();
	dungeonWantLoad = "";
	UnMarkUnsavedChanges();
}

void Crawl::DungeonEditor::Load(string path)
{	
	TileEditUnselectAll();
	dungeon->Load(path);
	RefreshDungeonFileNames();
	dungeonWantLoad = "";
	UnMarkUnsavedChanges();
}

std::string Crawl::DungeonEditor::GetDungeonFilePath()
{
	std::string filename;
	filename += dungeon->dungeonFileLocation;
	filename += dungeonFileNameSaveAs;
	filename += dungeon->dungeonFileExtension;
	return filename;
}

void Crawl::DungeonEditor::MarkUnsavedChanges()
{
	unsavedChanges = true;
	Window::SetWindowTitle("Crawler Editor | ***(Unsaved Changes)***");
}

void Crawl::DungeonEditor::UnMarkUnsavedChanges()
{
	unsavedChanges = false;
	Window::SetWindowTitle("Crawler Editor");
}

void Crawl::DungeonEditor::TileEditUnselectAll()
{
	selectedTile = nullptr;
	selectedLever = nullptr;
	selectedDoor = nullptr;
	selectedTransporter = nullptr;
	selectedActivatorPlate = nullptr;
	selectedTileShootLaser = nullptr;
	selectedBlockerEnemy = nullptr;
	selectedChaseEnemy = nullptr;
	selectedSwitcherEnemy = nullptr;
	selectedCheckpoint = nullptr;
	selectedMirror = nullptr;
	selectedSlugEnemy = nullptr;
	selectedTileDecoration = nullptr;
	selectedStairs = nullptr;

	selectedDoorWindowOpen = false;
	selectedLeverWindowOpen = false;
	selectedActivatorPlateWindowOpen = false;
	selectedTransporterWindowOpen = false;
	selectedShootLaserWindowOpen = false;
	selectedBlockerEnemyWindowOpen = false;
	selectedChaseEnemyWindowOpen = false;
	selectedSwitcherEnemyWindowOpen = false;
	selectedCheckpointWindowOpen = false;
	selectedMirrorWindowOpen = false;
	selectedSlugEnemyWindowOpen = false;
	selectedDecorationWindowOpen = false;
	selectedStairsWindowOpen = false;
}
