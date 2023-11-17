#include "DungeonEditor.h"
#include "DungeonGameManager.h"
#include "DungeonGameManagerEvent.h"
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
#include "DungeonLight.h"
#include "DungeonEventTrigger.h"
#include "DungeonEnemySlugPath.h"
#include "DungeonCollectableKey.h"
#include "DungeonGameManager.h"
#include "DungeonMenu.h"

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

#include "ComponentLightPoint.h"

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

	floorVarientShortNames.clear();
	for (auto varient : dungeon->floorVariantPaths)
	{
		int lastSlash = varient.find_last_of('/');
		int extension = varient.find_last_of('.');
		floorVarientShortNames.push_back(varient.substr(lastSlash + 1, extension - lastSlash - 1));
	}


}

void Crawl::DungeonEditor::DrawGUI()
{
	ImGui::SetNextWindowPos({ 0,0 }, ImGuiCond_Always);
	ImGui::SetNextWindowSize({ 320, 600 }, ImGuiCond_FirstUseEver);
	ImGui::Begin("Dungeon Edit", 0, ImGuiWindowFlags_NoMove);
	DrawGUIFileOperations();
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
		case Mode::MurderinaPathBrush:
		{
			DrawGUIModeRailBrush();
			break;
		}
		case Mode::MurderinaPathEdit:
		{
			DrawGUIModeRailEdit();
			break;
		}
		case Mode::DungeonProperties:
		{
			DrawGUIModeDungeonProperties();
			break;
		}
		case Mode::GameManager:
		{
			DrawGUIModeGameManager();
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
		requestedGameMode = true;
		if (!dirtyGameplayScene)
			dungeon->player->Respawn();
		dirtyGameplayScene = true;
		dungeon->player->usingLevelEditor = true;
		TileEditUnselectAll();
	}
	if (unsavedChanges)
	{
		ImGui::EndDisabled();
		if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
			ImGui::SetTooltip("You cannot Play with unsaved changes.");
	}

	ImGui::SameLine();
	bool canReset = dungeon->dungeonFileName != "" && (dirtyGameplayScene || unsavedChanges);
	if (!canReset) ImGui::BeginDisabled();
	if (ImGui::Button("Reset Dungeon"))
	{
		TileEditUnselectAll();
		dungeon->ResetDungeon();
		dungeon->player->SetLevel2(false);
		dungeon->player->ClearRespawn();
		dungeon->player->Teleport(dungeon->defaultPlayerStartPosition);
		dungeon->player->Orient(dungeon->defaultPlayerStartOrientation);

		dirtyGameplayScene = false;
		UnMarkUnsavedChanges();
	}
	if (!canReset) ImGui::EndDisabled();
	if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
		ImGui::SetTooltip("Resets the gameplay changes to the dungeon back to initial dungeon state.");

	ImGui::SameLine();
	if (ImGui::Button("Back to Menu"))
	{
		DungeonGameManager::Get()->GetMenu()->ExecuteReturnToMainMenuButton();
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
			NewDungeon();
			MarkUnsavedChanges();
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
void Crawl::DungeonEditor::DrawGUIModeSelect()
{
	if (ImGui::BeginCombo("Mode", editModeNames[(int)editMode].c_str()))
	{
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

		if (ImGui::Selectable(editModeNames[2].c_str()))
		{
			editMode = Mode::MurderinaPathBrush;
		}
		if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
			ImGui::SetTooltip("Build paths for the Murderina");

		if (ImGui::Selectable(editModeNames[3].c_str()))
		{
			editMode = Mode::MurderinaPathEdit;
			murderinaPathSelected = nullptr;
		}
		if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
			ImGui::SetTooltip("Tweak existing paths for the Murderina");

		if (ImGui::Selectable(editModeNames[4].c_str()))
			editMode = Mode::DungeonProperties;
		if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
			ImGui::SetTooltip("Edit specific configuration items about this particular dungeon");

		if (ImGui::Selectable(editModeNames[5].c_str()))
			editMode = Mode::GameManager;
		if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
			ImGui::SetTooltip("The GameManager contains configuration items specific to the lobby state. These values change over time, but you can mess with them here for testing purposes.");

		ImGui::EndCombo();
	}
}
void Crawl::DungeonEditor::DrawGUIModeTileBrush()
{
	ImGui::BeginDisabled();
	ImGui::DragInt2("Brush Position", &gridSelected.x);
	ImGui::EndDisabled();
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
		ImGui::Text("No Tile Here");
		ImGui::BeginDisabled();
		ImGui::DragInt2("Location", &selectedTilePosition.x);
		ImGui::EndDisabled();
	}
	else
	{
		// selected tile coordinates
		ImGui::Text("Selected Tile");
		ImGui::BeginDisabled();
		ImGui::DragInt2("Location", &selectedTile->position.x);
		ImGui::EndDisabled();

		// Cardinal traversable/see yes/no
		ImGui::Spacing();
		ImGui::Text("Pathing");
		ImGui::SameLine();
		if (ImGui::Checkbox("Untraversable", &selectedTile->permanentlyOccupied))
			MarkUnsavedChanges();
		if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
			ImGui::SetTooltip("Marks the tile as completely untraversable. Takes precedence over any other settings. Previous called \"Permanently Occupied\"");
		unsigned int oldMaskTraverse = selectedTile->maskTraverse;
		ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(0, 255, 0, 255));
		if (ImGui::BeginCombo("Walk", maskToString[oldMaskTraverse].c_str()))
		{
			if (ImGui::Checkbox("Can Walk North", &selectedTileUntraversableWalls[0]))
				selectedTile->maskTraverse += selectedTileUntraversableWalls[0] ? NORTH_MASK : -NORTH_MASK;
			if (ImGui::Checkbox("Can Walk East", &selectedTileUntraversableWalls[2]))
				selectedTile->maskTraverse += selectedTileUntraversableWalls[2] ? EAST_MASK : -EAST_MASK;
			if (ImGui::Checkbox("Can Walk South", &selectedTileUntraversableWalls[1]))
				selectedTile->maskTraverse += selectedTileUntraversableWalls[1] ? SOUTH_MASK : -SOUTH_MASK;
			if (ImGui::Checkbox("Can Walk West", &selectedTileUntraversableWalls[3]))
				selectedTile->maskTraverse += selectedTileUntraversableWalls[3] ? WEST_MASK : -WEST_MASK;
			if (ImGui::Button("All"))
				selectedTile->maskTraverse = 15;
			ImGui::SameLine();
			if (ImGui::Button("None"))
				selectedTile->maskTraverse = 0;
			ImGui::EndCombo();
		}
		ImGui::PopStyleColor();
		if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
			ImGui::SetTooltip("Configure in what direction things can walk off (or onto) this tile");

		if (oldMaskTraverse != selectedTile->maskTraverse)
		{
			RefreshSelectedTile();
			MarkUnsavedChanges();
		}

		unsigned int oldMaskSee = selectedTile->maskSee;
		ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(123, 123, 255, 255));
		if (ImGui::BeginCombo("See", maskToString[oldMaskSee].c_str()))
		{
			if (ImGui::Checkbox("Can See North", &selectedTileSeeThroughWalls[0]))
				selectedTile->maskSee += selectedTileSeeThroughWalls[0] ? NORTH_MASK : -NORTH_MASK;
			if (ImGui::Checkbox("Can See South", &selectedTileSeeThroughWalls[1]))
				selectedTile->maskSee += selectedTileSeeThroughWalls[1] ? SOUTH_MASK : -SOUTH_MASK;
			if (ImGui::Checkbox("Can See East", &selectedTileSeeThroughWalls[2]))
				selectedTile->maskSee += selectedTileSeeThroughWalls[2] ? EAST_MASK : -EAST_MASK;
			if (ImGui::Checkbox("Can See West", &selectedTileSeeThroughWalls[3]))
				selectedTile->maskSee += selectedTileSeeThroughWalls[3] ? WEST_MASK : -WEST_MASK;
			if (ImGui::Button("All"))
				selectedTile->maskSee = 15;
			ImGui::SameLine();
			if (ImGui::Button("None"))
				selectedTile->maskSee = 0;
			ImGui::EndCombo();
		}
		ImGui::PopStyleColor();
		if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
			ImGui::SetTooltip("Configure in what direction things can see out (or into) this tile");
		if (oldMaskSee != selectedTile->maskSee)
		{
			RefreshSelectedTile();
			MarkUnsavedChanges();
		}

		// Wall Variants
		ImGui::Spacing();
		ImGui::Text("Wall Visuals");
		ImGui::SameLine();
		if (ImGui::Checkbox("No Pillars", &selectedTile->dontGeneratePillars))
		{
			MarkUnsavedChanges();
			dungeon->UpdatePillarsForTile(selectedTile);
		}
		if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
			ImGui::SetTooltip("Stops automatic pillar generation on the corners of this tile.");

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
			if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
				ImGui::SetTooltip("Configure the configurations of the walls for this tile. Note this doesnt affect ability to traverse or see through the tile");
			ImGui::PopID();
		}

		// Floor Tile Variants
		ImGui::Text("Floor Visual");
		if (ImGui::BeginCombo("Type", selectedTile->floorVariant == -1 ? "None" : floorVarientShortNames[selectedTile->floorVariant].c_str())) // lmao yuck
		{
			if (ImGui::Selectable("None", -1 == selectedTile->floorVariant))
			{
				selectedTile->floorVariant = -1;
				dungeon->CreateTileObject(selectedTile);
				MarkUnsavedChanges();
			}

			for (int i = 0; i < dungeon->floorVariantPaths.size(); i++)
			{
				if (ImGui::Selectable(floorVarientShortNames[i].c_str(), i == selectedTile->floorVariant))
				{
					selectedTile->floorVariant = i;
					dungeon->CreateTileObject(selectedTile);
					MarkUnsavedChanges();
				}
			}
			ImGui::EndCombo();
		}
		if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
			ImGui::SetTooltip("Configure the configurations of the walls for this tile. Note this doesnt affect ability to traverse or see through the tile");
	}

	// Buttons
	ImGui::Spacing();
	ImGui::Text("Tile Objects");
	ImGui::PushID("Add");
	if (ImGui::BeginCombo("Add", "...", ImGuiComboFlags_HeightLargest))
	{
		ImGui::Text("Edge Objects");
		ImGui::Indent();
		if (ImGui::Selectable("Door"))
		{
			MarkUnsavedChanges();
			selectedDoor = dungeon->CreateDoor(selectedTilePosition, 0, GetNextAvailableDoorID(), false);
			selectedTileDoors.push_back(selectedDoor);
			selectedDoorWindowOpen = true;
		}
		//ImGui::SameLine();
		if (ImGui::Selectable("Button"))
		{
			MarkUnsavedChanges();
			selectedLever = dungeon->CreateLever(selectedTilePosition, 0, GetNextAvailableLeverID(), 0, false);
			selectedTileLevers.push_back(selectedLever);
			selectedLeverWindowOpen = true;
		}
		//ImGui::SameLine();
		if (ImGui::Selectable("Shooter"))
		{
			MarkUnsavedChanges();
			selectedTileShootLaser = dungeon->CreateShootLaser(selectedTilePosition, (FACING_INDEX)0, GetNextAvailableDoorID());
			selectedTileShootLasers.push_back(selectedTileShootLaser);
			selectedShootLaserWindowOpen = true;
		}
		if (ImGui::Selectable("Key"))
		{
			MarkUnsavedChanges();
			selectedTileKey = dungeon->CreateKey(selectedTilePosition);
			selectedKeyWindowOpen = true;
		}
		ImGui::Unindent();
		ImGui::Text("Basic");
		ImGui::Indent();
		bool tileOccupied = selectedTileOccupied;
		if (tileOccupied) ImGui::BeginDisabled();
		if (ImGui::Selectable("Activator Plate"))
		{
			MarkUnsavedChanges();
			selectedActivatorPlate = dungeon->CreatePlate(selectedTilePosition, 0);
			selectedActivatorPlateWindowOpen = true;
			selectedTileOccupied = true;
		}
		//ImGui::SameLine();
		if (ImGui::Selectable("Spikes"))
		{
			MarkUnsavedChanges();
			dungeon->CreateSpikes(selectedTilePosition);
			selectedHasSpikes = true;
			selectedTileOccupied = true;
		}
		//ImGui::SameLine();
		if (ImGui::Selectable("Box"))
		{
			MarkUnsavedChanges();
			dungeon->CreatePushableBlock(selectedTilePosition);
			selectedHasBlock = true;
			selectedTileOccupied = true;
		}
		//ImGui::SameLine();
		if (ImGui::Selectable("Mirror"))
		{
			MarkUnsavedChanges();
			selectedMirror = dungeon->CreateMirror(selectedTilePosition, NORTH_INDEX);
			selectedMirrorWindowOpen = true;
			selectedTileOccupied = true;
		}

		if (ImGui::Selectable("Transporter"))
		{
			MarkUnsavedChanges();
			selectedTransporter = dungeon->CreateTransporter(selectedTilePosition);
			selectedTransporterWindowOpen = true;
			selectedTileOccupied = true;
		}
		//ImGui::SameLine();
		if (ImGui::Selectable("Checkpoint"))
		{
			MarkUnsavedChanges();
			selectedCheckpoint = dungeon->CreateCheckpoint(selectedTilePosition, NORTH_INDEX);
			selectedCheckpointWindowOpen = true;
			selectedTileOccupied = true;
		}
		ImGui::Unindent();
		ImGui::Text("Enemies");
		ImGui::Indent();
		if (ImGui::Selectable("Blocker"))
		{
			MarkUnsavedChanges();
			selectedBlockerEnemy = dungeon->CreateEnemyBlocker(selectedTilePosition, NORTH_INDEX);
			selectedBlockerEnemyWindowOpen = true;
			selectedTileOccupied = true;
		}
		//ImGui::SameLine();
		bool maxChasers = (dungeon->chasers.size() > 1);
		if (maxChasers) ImGui::BeginDisabled();
		if (ImGui::Selectable("Chaser"))
		{
			MarkUnsavedChanges();
			selectedChaseEnemy = dungeon->CreateEnemyChase(selectedTilePosition, NORTH_INDEX);
			selectedChaseEnemyWindowOpen = true;
			selectedTileOccupied = true;
		}
		if (maxChasers) ImGui::EndDisabled();
		//ImGui::SameLine();
		if (ImGui::Selectable("Murderina"))
		{
			MarkUnsavedChanges();
			selectedMurderinaEnemy = dungeon->CreateMurderina(selectedTilePosition, NORTH_INDEX);
			selectedMurdurinaWindowOpen = true;
			selectedTileOccupied = true;
		}
		//ImGui::SameLine();
		if (ImGui::Selectable("Switcher"))
		{
			MarkUnsavedChanges();
			selectedSwitcherEnemy = dungeon->CreateEnemySwitcher(selectedTilePosition, NORTH_INDEX);
			selectedSwitcherEnemyWindowOpen = true;
			selectedTileOccupied = true;
		}
		if (tileOccupied) ImGui::EndDisabled();
		ImGui::Unindent();
		ImGui::Text("Miscellaneous");
		ImGui::Indent();
		if (ImGui::Selectable("Decoration"))
		{
			MarkUnsavedChanges();
			selectedTileDecoration = dungeon->CreateDecoration(selectedTilePosition, NORTH_INDEX);
			selectedTileDecorations.push_back(selectedTileDecoration);
			selectedDecorationWindowOpen = true;
		}
		bool cantMakeLight = selectedLight != nullptr;
		if (cantMakeLight) ImGui::BeginDisabled();
		if (ImGui::Selectable("Light"))
		{
			MarkUnsavedChanges();
			int newID = GetNextAvailableLightID();
			selectedLight = dungeon->CreateLight(selectedTilePosition);
			selectedLight->id = newID;
			selectedLight->Init();
			selectedLightWindowOpen = true;
		}
		if (cantMakeLight) ImGui::EndDisabled();

		bool cantMakeStairs = selectedStairs != nullptr;
		if (cantMakeStairs) ImGui::BeginDisabled();
		if (ImGui::Selectable("Stairs"))
		{
			MarkUnsavedChanges();
			selectedStairs = dungeon->CreateStairs(selectedTilePosition);
			selectedStairsWindowOpen = true;
		}
		if (cantMakeStairs) ImGui::EndDisabled();

		//ImGui::SameLine();
		if (ImGui::Selectable("Event Trigger"))
		{
			MarkUnsavedChanges();
			selectedEventTrigger = dungeon->CreateEventTrigger(selectedTilePosition);
			selectedEventTriggerWindowOpen = true;
		}
		ImGui::Unindent();
		ImGui::EndCombo();
	}
	ImGui::PopID();
	// Entity List
	ImGui::PushID("Objects");
	// Delete All
	if (ImGui::Button("Delete All"))
		ImGui::OpenPopup("confirm_delete_all");
	if (ImGui::BeginPopupModal("confirm_delete_all"))
	{
		ImGui::Text("Are you sure you want to delete ALL objects on this tile?");
		if (ImGui::Button("Yes"))
		{
			TileEditDeleteAllObjectsOnTile();
			ImGui::CloseCurrentPopup();
		}
		ImGui::SameLine();
		if (ImGui::Button("No"))
			ImGui::CloseCurrentPopup();
		ImGui::EndPopup();
	}

	// Move All
	ImGui::SameLine();
	if (ImGui::Button("Move All"))
		ImGui::OpenPopup("confirm_move_all");
	if (ImGui::BeginPopupModal("confirm_move_all"))
	{
		ImGui::Text("Where do you want to move all objects to?");
		ImGui::InputInt2("Coordinate", &selectedTileMoveObjectsTo.x);
		if (ImGui::Button("Move"))
		{
			TileEditMoveAllObjectsOnTile(selectedTileMoveObjectsTo);
			MarkUnsavedChanges();
			ImGui::CloseCurrentPopup();
		}
		ImGui::SameLine();
		if (ImGui::Button("Cancel"))
			ImGui::CloseCurrentPopup();
		
		selectedTileMoveFlash += 0.01f; // This should use deltaTime but delta isn't yet globally accessable and really ought to be at this point.
		LineRenderer::DrawFlatBox(dungeonPosToObjectScale(selectedTileMoveObjectsTo), 1.0f, { 0,selectedTileMoveFlash*2.0f,0 });
		if (selectedTileMoveFlash > 0.5f) selectedTileMoveFlash = 0.0f;
		
		ImGui::EndPopup();
	}

	// Copy Decorations
	ImGui::SameLine();
	if (ImGui::Button("Copy Decorations"))
		ImGui::OpenPopup("confirm_copy_all");
	if (ImGui::BeginPopupModal("confirm_copy_all"))
	{
		ImGui::Text("Where do you want to copy all decorations to?");
		ImGui::InputInt2("Coordinate", &selectedTileMoveObjectsTo.x);
		if (ImGui::Button("Copy"))
		{
			TileEditCopyDecorationsOnTile(selectedTileMoveObjectsTo);
			MarkUnsavedChanges();
			ImGui::CloseCurrentPopup();
		}
		ImGui::SameLine();
		if (ImGui::Button("Cancel"))
			ImGui::CloseCurrentPopup();

		selectedTileMoveFlash += 0.01f; // This should use deltaTime but delta isn't yet globally accessable and really ought to be at this point.
		LineRenderer::DrawFlatBox(dungeonPosToObjectScale(selectedTileMoveObjectsTo), 1.0f, { 0,selectedTileMoveFlash * 2.0f,0 });
		if (selectedTileMoveFlash > 0.5f) selectedTileMoveFlash = 0.0f;

		ImGui::EndPopup();
	}

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
		string leverName = "Button (ID: ";
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
	if (selectedTileKey)
	{
		string keyName = "Key (Lock: ";
		keyName += to_string(selectedTileKey->lockReleaseID);
		keyName += ", Activates: ";
		keyName += to_string(selectedTileKey->doorActivateID);
		keyName += ")";
		if (ImGui::Selectable(keyName.c_str())) selectedKeyWindowOpen = true;
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
	if (selectedMurderinaEnemy)
	{
		if (ImGui::Selectable("Slug")) selectedMurdurinaWindowOpen = true;
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
		decoName += selectedTileDecorations[i]->modelNameShort;
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

	if (selectedLight)
	{
		if (ImGui::Selectable("Light")) selectedLightWindowOpen = true;
	}

	if (selectedEventTrigger)
	{
		if (ImGui::Selectable("Event Trigger")) selectedEventTriggerWindowOpen = true;
	}

	ImGui::Unindent();
	ImGui::PopID();

	ImGui::PushID("EntityProperties");
	
	if (selectedDoorWindowOpen)
		DrawGUIModeTileEditDoor();
	if (selectedLeverWindowOpen)
		DrawGUIModeTileEditLever();
	if (selectedKeyWindowOpen)
		DrawGUIModeTileEditKey();
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
	if (selectedMurdurinaWindowOpen)
		DrawGUIModeTileEditMurderina();

	if (selectedSwitcherEnemyWindowOpen)
		DrawGUIModeTileEditSwitcher();
	if (selectedMirrorWindowOpen)
		DrawGUIModeTileEditMirror();
	if (selectedDecorationWindowOpen)
		DrawGUIModeTileEditDecoration();
	if (selectedStairsWindowOpen)
		DrawGUIModeTileEditStairs();
	if (selectedLightWindowOpen)
		DrawGUIModeTileEditLight();
	if (selectedEventTriggerWindowOpen)
		DrawGUIModeTileEditEventTrigger();
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
		selectedDoor->Toggle();
	}

	bool barricaded = selectedDoor->isBarricaded;
	if (ImGui::Checkbox("Is Barricaded", &barricaded))
	{
		if (barricaded)
			selectedDoor->MakeBarricaded();
		else
			selectedDoor->RemoveBarricaded();

		MarkUnsavedChanges();
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
	ImGui::Begin("Edit Button", &selectedLeverWindowOpen);

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
		ImGui::OpenPopup("delete_button_confirm");


	if (ImGui::BeginPopupModal("delete_button_confirm"))
	{
		ImGui::Text("Are you sure you want to delete the Button?");
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
void Crawl::DungeonEditor::DrawGUIModeTileEditKey()
{
	ImGui::SetNextWindowPos({ 400,0 }, ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowSize({ 300, 150 }, ImGuiCond_FirstUseEver);
	ImGui::Begin("Edit Key", &selectedKeyWindowOpen);
	if (ImGui::BeginCombo("Orientation", orientationNames[selectedTileKey->facing].c_str()))
	{
		int oldOrientation = selectedTileKey->facing;
		for (int i = 0; i < 4; i++)
			if (ImGui::Selectable(orientationNames[i].c_str()))
			{
				selectedTileKey->facing = (FACING_INDEX)i;
				if (selectedTileKey->facing != oldOrientation)
				{
					MarkUnsavedChanges();
					selectedTileKey->object->SetLocalRotationZ(orientationEulers[i]);
				}
			}
		ImGui::EndCombo();
	}

	if (ImGui::InputInt("Key ID", (int*)&selectedTileKey->lockReleaseID))
		MarkUnsavedChanges();

	if (ImGui::InputInt("Door Open ID", (int*)&selectedTileKey->doorActivateID))
		MarkUnsavedChanges();

	if (ImGui::Button("Delete"))
		ImGui::OpenPopup("delete_key_confirm");


	if (ImGui::BeginPopupModal("delete_key_confirm"))
	{
		ImGui::Text("Are you sure you want to delete the key?");
		if (ImGui::Button("Yes"))
		{
			MarkUnsavedChanges();
			dungeon->RemoveKey(selectedTileKey);
			selectedTileKey = nullptr;
			selectedKeyWindowOpen = false;
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

	if (ImGui::InputInt("Activate ID", (int*)&selectedActivatorPlate->activateID))
		MarkUnsavedChanges();
	
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
	
	// Should this window be expanded to fit the gameManager stuff.
	if(!selectedTransporter->gameManagerInteraction)
		ImGui::SetNextWindowSize({ 500, 170 }, ImGuiCond_Always);
	else
		ImGui::SetNextWindowSize({ 500, 310 }, ImGuiCond_Always);
	ImGui::Begin("Edit Transporter", &selectedTransporterWindowOpen, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize);
	if (ImGui::InputText("Name", &selectedTransporter->name))
		MarkUnsavedChanges();
	ImGui::PushItemWidth(100);
	if (ImGui::BeginCombo("Orientation", orientationNames[selectedTransporter->fromOrientation].c_str()))
	{
		int oldOrientation = selectedTransporter->fromOrientation;
		for (int i = 0; i < 4; i++)
			if (ImGui::Selectable(orientationNames[i].c_str()))
			{
				selectedTransporter->fromOrientation = i;
				if (selectedTransporter->fromOrientation != oldOrientation) MarkUnsavedChanges();
			}
		ImGui::EndCombo();
	}
	ImGui::PopItemWidth();
	ImGui::SameLine();
	if (ImGui::Checkbox("To Lobby Second Level", &selectedTransporter->toLobby2))
	{
		selectedTransporter->toDungeon = "crawler/dungeon/lobby";
		RefreshSelectedTransporterData("crawler/dungeon/lobby2" + dungeon->dungeonFileExtension);
		MarkUnsavedChanges();
	}

	if (selectedTransporter->toLobby2) ImGui::BeginDisabled();
	if (ImGui::BeginCombo("To Dungeon", selectedTransporter->toDungeon.c_str(), ImGuiComboFlags_HeightLargest))
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
	if (selectedTransporter->toLobby2) ImGui::EndDisabled();

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

	// Game Manager triggers
	if (ImGui::Checkbox("Game Manager Interactions", &selectedTransporter->gameManagerInteraction)) MarkUnsavedChanges();
	if (selectedTransporter->gameManagerInteraction)
	{
		for (int i = 0; i < selectedTransporter->gameManagerEvents.size(); i++)
		{
			ImGui::PushID(i);
			if (ImGui::Button("Del"))
			{
				selectedTransporter->gameManagerEvents.erase(selectedTransporter->gameManagerEvents.begin() + i);
				MarkUnsavedChanges();
				i--;
				if (i == selectedTransporter->gameManagerEvents.size() - 1)
				{
					ImGui::PopID();
					break;
				}
			}
			ImGui::SameLine();
			if (selectedTransporter->gameManagerEvents[i].DrawGUIInternal()) MarkUnsavedChanges();
			ImGui::PopID();
		}
		if (ImGui::Button("Add"))
		{
			selectedTransporter->gameManagerEvents.emplace_back();
			MarkUnsavedChanges();
		}
	}

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
				if (selectedCheckpoint->facing != oldOrientation) MarkUnsavedChanges();
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
void Crawl::DungeonEditor::DrawGUIModeTileEditMurderina()
{
	ImGui::SetNextWindowPos({ 400,0 }, ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowSize({ 300, 150 }, ImGuiCond_FirstUseEver);
	ImGui::Begin("Edit Murderina Enemy", &selectedMurdurinaWindowOpen);

	if (ImGui::BeginCombo("Look Direction", orientationNames[selectedMurderinaEnemy->facing].c_str()))
	{
		int oldOrientation = selectedMurderinaEnemy->facing;
		for (int i = 0; i < 4; i++)
			if (ImGui::Selectable(orientationNames[i].c_str()))
			{
				selectedMurderinaEnemy->facing = (FACING_INDEX)i;
				if (selectedMurderinaEnemy->facing != oldOrientation)
				{
					MarkUnsavedChanges();
					selectedMurderinaEnemy->object->SetLocalRotationZ(orientationEulers[i]);
				}
			}
		ImGui::EndCombo();
	}

	if (ImGui::Button("Delete"))
		ImGui::OpenPopup("delete_murderina_confirm");
	if (ImGui::BeginPopupModal("delete_murderina_confirm"))
	{
		ImGui::Text("Are you sure you want to delete the Murderina?");
		if (ImGui::Button("Yes"))
		{
			MarkUnsavedChanges();
			dungeon->RemoveSlug(selectedMurderinaEnemy);
			selectedMurderinaEnemy = nullptr;
			selectedMurdurinaWindowOpen = false;
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
	// Handle 3D Gizmo
	ImGuizmo::SetRect(0, 0, Window::GetViewPortSize().x, Window::GetViewPortSize().y);
	mat4 view = Scene::GetCurrentCamera()->GetViewMatrix();
	mat4 projection = Scene::GetCurrentCamera()->GetProjectionMatrix();
	mat4 position = selectedTileDecoration->object->children[0]->transform;
	mat4 delta;
	if (ImGuizmo::Manipulate((float*)&view, (float*)&projection, ImGuizmo::TRANSLATE, ImGuizmo::WORLD, (float*)&position, (float*)&delta, 0, 0, 0))
	{
		// This is the most bullshit hacked thing that I don't have time to really clean up at the moment!
		// I think the co-ordinate spaces are wrong here, but this check flips it and makes it work and we're cool move on nothing to see here.
		if (selectedTileDecoration->facing == 0 || selectedTileDecoration->facing == 2)
			delta = glm::rotate(glm::mat4(1), glm::radians(orientationEulersReversed[selectedTileDecoration->facing]), { 0,0,1 }) * delta;
		else
			delta = glm::rotate(glm::mat4(1), glm::radians(orientationEulers[selectedTileDecoration->facing]), { 0,0,1 }) * delta;
		vec3 moveDelta = delta[3];
		selectedTileDecoration->localPosition += moveDelta;
		selectedTileDecoration->UpdateTransform();
		MarkUnsavedChanges();
	}

	ImGui::SetNextWindowPos({ 400,0 }, ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowSize({ 600, 170 }, ImGuiCond_FirstUseEver);
	ImGui::Begin("Edit Decoration", &selectedDecorationWindowOpen, ImGuiWindowFlags_NoResize);

	if (ImGui::BeginCombo("Decoration", selectedTileDecoration->modelName.c_str(), ImGuiComboFlags_HeightLargest))
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
	ImGui::SameLine();
	if (ImGui::Checkbox("Show All ", &decorationsShowAllModels)) RefreshAvailableDecorations();

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
	
	if (ImGui::DragFloat3("Position", &selectedTileDecoration->localPosition.x, 0.1f, -5, 5, "%.3f", ImGuiSliderFlags_AlwaysClamp))
	{
		MarkUnsavedChanges();
		selectedTileDecoration->UpdateTransform();
	}
	if (ImGui::DragFloat3("Rotation", &selectedTileDecoration->localRotation.x, 0.1f, -180.0f, 180.0f, "%.3f", ImGuiSliderFlags_AlwaysClamp))
	{
		MarkUnsavedChanges();
		selectedTileDecoration->UpdateTransform();
	}

	if (ImGui::Checkbox("Casts Shadows", &selectedTileDecoration->castsShadows))
	{
		MarkUnsavedChanges();
		selectedTileDecoration->UpdateShadowCasting();
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
void Crawl::DungeonEditor::DrawGUIModeTileEditLight()
{
	ImGui::SetNextWindowPos({ 400,0 }, ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowSize({ 450, 350 }, ImGuiCond_FirstUseEver);
	ImGui::Begin("Edit Point Light", &selectedLightWindowOpen);
	if (ImGui::ColorEdit3("Colour", &selectedLight->colour.x))
	{
		MarkUnsavedChanges();
		selectedLight->UpdateLight();
	}
	if (ImGui::DragFloat("Intensity", &selectedLight->intensity))
	{
		selectedLight->intensityCurrent = selectedLight->intensity;
		selectedLight->UpdateLight();
		MarkUnsavedChanges();
	}
	if (ImGui::DragFloat3("Position", &selectedLight->localPosition.x, 0.1f, -2.0f, 10.0f))
	{
		MarkUnsavedChanges();
		selectedLight->UpdateTransform();
	}

	// Decoration stuff
	string decorationName = "None";
	if (selectedLight->lightDecorationID != -1) decorationName = DungeonLight::lightDecorations[selectedLight->lightDecorationID];

	if (ImGui::BeginCombo("Decoration", decorationName.c_str()))
	{
		bool selectedOne = false;
		if (ImGui::Selectable("None", false))
		{
			selectedOne = true;
			selectedLight->lightDecorationID = -1;
		}

		for (int i = 0; i < DungeonLight::lightDecorationsQuantity; i++)
		{
			if (ImGui::Selectable(DungeonLight::lightDecorations[i].c_str(), false))
			{
				selectedOne = true;
				selectedLight->lightDecorationID = i;
			}
		}
		if (selectedOne) selectedLight->LoadDecoration();

		ImGui::EndCombo();
	}
	if (selectedLight->lightDecorationID != -1)
	{
		decorationName = DungeonLight::lightDecorations[selectedLight->lightDecorationID];
		if (ImGui::BeginCombo("Forward", orientationNames[selectedLight->lightDecorationDirection].c_str()))
		{
			int oldOrientation = selectedLight->lightDecorationDirection;
			for (int i = 0; i < 4; i++)
				if (ImGui::Selectable(orientationNames[i].c_str()))
				{
					selectedLight->lightDecorationDirection = (FACING_INDEX)i;
					if (selectedLight->lightDecorationDirection != oldOrientation)
					{
						MarkUnsavedChanges();
						selectedLight->object->SetLocalRotationZ(orientationEulersReversed[i]);
					}
				}
			ImGui::EndCombo();
		}

	}

	if (ImGui::InputInt("ID", &selectedLight->id)) MarkUnsavedChanges();
	if (ImGui::Checkbox("Ignore Global Flicker", &selectedLight->flickerIgnoreGlobal))	MarkUnsavedChanges();
	if (ImGui::Checkbox("Starts Disabled", &selectedLight->startDisabled)) MarkUnsavedChanges();
	if (ImGui::Checkbox("Flickers Randomly", &selectedLight->flickerRepeat))
	{
		MarkUnsavedChanges();
		selectedLight->flickerEnabled = selectedLight->flickerRepeat;
	}
	if (selectedLight->flickerRepeat)
	{
		if (ImGui::InputFloat("Min Repeat Delay", &selectedLight->flickerRepeatMin))
			MarkUnsavedChanges();
		if (ImGui::InputFloat("Max Repeat Delay", &selectedLight->flickerRepeatMax))
			MarkUnsavedChanges();
	}
	
	if (ImGui::Checkbox("Is Lobby trigger light", &selectedLight->isLobbyLight))
		MarkUnsavedChanges();

	glm::vec3 worldPos = dungeonPosToObjectScale(selectedLight->position) + selectedLight->localPosition;
	glm::vec3 offset(0, 0, 0.1f);
	LineRenderer::DrawLine(worldPos, worldPos + offset, selectedLight->colour);


	if (ImGui::Button("Apply Room Settings"))
	{
		selectedLight->MakeRoomLight();
		MarkUnsavedChanges();
	}
	ImGui::SameLine();
	if (ImGui::Button("Apply Hallway Settings"))
	{
		selectedLight->MakeHallwayLight();
		MarkUnsavedChanges();
	}

	if (ImGui::Button("Delete"))
		ImGui::OpenPopup("delete_light_confirm");


	if (ImGui::BeginPopupModal("delete_light_confirm"))
	{
		ImGui::Text("Are you sure you want to delete the light?");
		if (ImGui::Button("Yes"))
		{
			MarkUnsavedChanges();
			dungeon->RemoveLight(selectedLight);
			selectedLight = nullptr;
			selectedLightWindowOpen = false;
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
void Crawl::DungeonEditor::DrawGUIModeTileEditEventTrigger()
{
	ImGui::SetNextWindowPos({ 400,0 }, ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowSize({ 300, 200 }, ImGuiCond_FirstUseEver);
	ImGui::Begin("EditEvent Trigger", &selectedSwitcherEnemyWindowOpen);
	
	if (ImGui::Checkbox("Repeats", &selectedEventTrigger->repeats))
		MarkUnsavedChanges();

	// Event Type Selection
	if (ImGui::BeginCombo("Event Type", DrawGUIModeTileEditEventTriggerGetEventTypeString(selectedEventTrigger).c_str()))
	{
		if (ImGui::Selectable("Global Event"))
		{
			MarkUnsavedChanges();
			selectedEventTrigger->type = DungeonEventTrigger::Type::GlobalEvent;
		}
		if (ImGui::Selectable("Light Flicker"))
		{
			MarkUnsavedChanges();
			selectedEventTrigger->type = DungeonEventTrigger::Type::LightFlicker;
		}
		ImGui::EndCombo();
	}


	string eventTypeString;
	switch (selectedEventTrigger->type)
	{
	case DungeonEventTrigger::Type::GlobalEvent:
	{
		eventTypeString = "Global Event ID";
		break;
	}
	case DungeonEventTrigger::Type::LightFlicker:
	{
		eventTypeString = "Light ID";
		break;
	}
	case DungeonEventTrigger::Type::FTUEPrompt:
	{
		eventTypeString = "FTUE ID";
		break;
	}

	}
	if(ImGui::InputInt(eventTypeString.c_str(), &selectedEventTrigger->eventID))
		MarkUnsavedChanges();
	if (selectedEventTrigger->type == DungeonEventTrigger::Type::LightFlicker)
	{
		if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
			ImGui::SetTooltip("A Light ID of 0 will flicker all lights in the dungeon.");
	}
	if(ImGui::Checkbox("Must Be Facing", &selectedEventTrigger->mustBeFacing))
		MarkUnsavedChanges();
	if (selectedEventTrigger->mustBeFacing)
	{
		if (ImGui::BeginCombo("Facing Direction", orientationNames[selectedEventTrigger->facing].c_str()))
		{
			int oldOrientation = selectedEventTrigger->facing;
			for (int i = 0; i < 4; i++)
				if (ImGui::Selectable(orientationNames[i].c_str()))
				{
					selectedEventTrigger->facing = (FACING_INDEX)i;
					if (selectedEventTrigger->facing != oldOrientation)
					{
						MarkUnsavedChanges();
					}
				}
			ImGui::EndCombo();
		}
	}
	if (ImGui::Button("Delete (No Confirm)"))
	{
		MarkUnsavedChanges();
		dungeon->RemoveEventTrigger(selectedEventTrigger);
		selectedEventTrigger = nullptr;
		selectedEventTriggerWindowOpen = false;
		RefreshSelectedTile();
	}
	ImGui::End();
}

string Crawl::DungeonEditor::DrawGUIModeTileEditEventTriggerGetEventTypeString(DungeonEventTrigger* trigger)
{
	switch (trigger->type)
	{
	case DungeonEventTrigger::Type::GlobalEvent:
		return "Global";
	case DungeonEventTrigger::Type::LightFlicker:
		return "Light Flicker";
	case DungeonEventTrigger::Type::FTUEPrompt:
		return "FTUE Prompt";
	default:
		return "ERROR";
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

	if (ImGui::Checkbox("No Roof", &dungeon->noRoof))
		MarkUnsavedChanges();

	if (ImGui::Checkbox("Is Lobby (Don't mess with this!)", &dungeon->isLobby))
		MarkUnsavedChanges();

	if (ImGui::Checkbox("Is Void (Don't mess with this!)", &dungeon->isVoid))
		MarkUnsavedChanges();

	//if (ImGui::Checkbox("Player Turn is Free (Default is True)", &dungeon->playerTurnIsFree))
	//	MarkUnsavedChanges();

	//if (ImGui::Checkbox("Player Interact is Free (Default is False)", &dungeon->playerInteractIsFree))
	//	MarkUnsavedChanges();

	///*if (ImGui::Checkbox("Player can Knife (Default is False)", &dungeon->playerHasKnife))
	//	MarkUnsavedChanges();*/

	//if (ImGui::Checkbox("Player can Kick (Default is True)", &dungeon->playerCanKickBox))
	//	MarkUnsavedChanges();

	//if (ImGui::Checkbox("Player Can Push Box (Default is False)", &dungeon->playerCanPushBox))
	//	MarkUnsavedChanges();

	//if (ImGui::Checkbox("Switchers Must Be Looked At (Default is False)", &dungeon->switchersMustBeLookedAt))
	//	MarkUnsavedChanges();

	//if (ImGui::Checkbox("Player Can Push Mirrors (Default is False)", &dungeon->playerCanPushMirror))
	//	MarkUnsavedChanges();


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

void Crawl::DungeonEditor::DrawGUIModeGameManager()
{
	if (DungeonGameManager::Get()->DrawGUIInternal()) unsavedChanges = true;
}

void Crawl::DungeonEditor::NewDungeon()
{
	dungeon->ClearDungeon();
	dungeonFileName = "";
	dungeonFileNameSaveAs = "";
	dungeonFilePath = "";
	dirtyGameplayScene = false;
	MarkUnsavedChanges();
}


void Crawl::DungeonEditor::DrawGUIModeRailBrush()
{
	ImGui::Checkbox("Auto Tile", &murderinaPathAutoTile);
	if (!murderinaPathAutoTile) ImGui::BeginDisabled();
	ImGui::Checkbox("Update Neighbors", &murderinaPathUpdateNeighbors);
	if (!murderinaPathAutoTile) ImGui::EndDisabled();

	DrawGUIModeRailLines();
}
void Crawl::DungeonEditor::DrawGUIModeRailEdit()
{
	if (!murderinaPathSelected)
	{
		ImGui::Text("No Path Piece Selected");
	}
	else
	{
		unsigned int oldMaskTraverse = murderinaPathSelected->maskTraverse;
		ImGui::Indent();
		if (ImGui::Checkbox("North", &murderinaSelectedPathTraversable[NORTH_INDEX]))
			murderinaPathSelected->maskTraverse += murderinaSelectedPathTraversable[NORTH_INDEX] ? NORTH_MASK : -NORTH_MASK;
		ImGui::Unindent();

		if (ImGui::Checkbox("West", &murderinaSelectedPathTraversable[WEST_INDEX]))
			murderinaPathSelected->maskTraverse += murderinaSelectedPathTraversable[WEST_INDEX] ? WEST_MASK : -WEST_MASK;
		ImGui::SameLine();
		if (ImGui::Checkbox("East", &murderinaSelectedPathTraversable[EAST_INDEX]))
			murderinaPathSelected->maskTraverse += murderinaSelectedPathTraversable[EAST_INDEX] ? EAST_MASK : -EAST_MASK;

		ImGui::SameLine();
		ImGui::Text("  ");
		ImGui::SameLine();
		if (murderinaPathSelected->IsValidConfiguration())
		{
			ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(0, 255, 0, 255));
			ImGui::Text("Valid!");
		}
		else
		{
			ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 0, 0, 255));
			ImGui::Text("Invalid!");
		}
		ImGui::PopStyleColor();

		ImGui::Indent();
		if (ImGui::Checkbox("South", &murderinaSelectedPathTraversable[SOUTH_INDEX]))
			murderinaPathSelected->maskTraverse += murderinaSelectedPathTraversable[SOUTH_INDEX] ? SOUTH_MASK : -SOUTH_MASK;
		ImGui::Unindent();

		if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
			ImGui::SetTooltip("Configure in what direction things can walk off this tile");
		if (oldMaskTraverse != murderinaPathSelected->maskTraverse)
		{
			MarkUnsavedChanges();
			murderinaPathSelected->RefreshObject();
		}
	}
	DrawGUIModeRailLines();
}

void Crawl::DungeonEditor::DrawGUIModeRailLines()
{
	for (auto& slugPath : dungeon->slugPaths)
	{
		vec3 position = dungeonPosToObjectScale(slugPath->position);
		vec3 colour = { 0,1,1 };
		if (!slugPath->IsValidConfiguration()) colour = { 1, 0 ,0 };
		for (int i = 0; i < 4; i++)
		{
			if ((slugPath->maskTraverse & orientationMasksIndex[i]) == orientationMasksIndex[i])
			{
				LineRenderer::DrawLine(position, position + (dungeonPosToObjectScale(directions[i]) * 0.5f), colour);
			}
		}
	}
}

void Crawl::DungeonEditor::Update()
{
	// Save hotkey
	if (Input::Keyboard(GLFW_KEY_LEFT_CONTROL).Pressed() && Input::Keyboard(GLFW_KEY_S).Down())
	{
		if (!dirtyGameplayScene && dungeonFilePath != "" && unsavedChanges) Save();
	}

	// Draw Player Position
	vec3 playerWorldPosition = dungeonPosToObjectScale(dungeon->player->GetPosition());
	LineRenderer::DrawLine(playerWorldPosition, playerWorldPosition + vec3(0, 0, 2));

	switch (editMode)
	{
	case Mode::TileBrush:
		UpdateModeTileBrush();	break;
	case Mode::TileEdit:
		UpdateModeTileEdit();	break;
	case Mode::MurderinaPathBrush:
		UpdateModeMurderinaBrush();	break;
	case Mode::MurderinaPathEdit:
		UpdateModeMurderinaEdit();	break;
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
	DrawGizmos();
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
	if (Input::Mouse(0).Down())
	{
		ivec2 selectedTilePositionPrevious = selectedTilePosition;
		selectedTilePosition = GetMousePosOnGrid();

		TileEditUnselectAll();

		selectedTile = dungeon->GetTile(selectedTilePosition);

		if (selectedTilePositionPrevious != selectedTilePosition)
			selectedTileMoveObjectsTo = selectedTilePositionPrevious;
		
		RefreshSelectedTile();
	}
}

void Crawl::DungeonEditor::TileEditDeleteAllObjectsOnTile()
{
	for(auto & lever : selectedTileLevers)	dungeon->RemoveLever(lever);
	for (auto& door : selectedTileDoors)	dungeon->RemoveDoor(door);
	dungeon->RemoveKey(selectedTileKey);
	dungeon->RemoveTransporter(selectedTransporter);
	dungeon->RemovePlate(selectedActivatorPlate);
	if (selectedHasSpikes) dungeon->RemoveSpikes(selectedTile->position);
	if (selectedHasBlock) dungeon->RemovePushableBlock(selectedTile->position);
	for(auto& laser : selectedTileShootLasers) dungeon->RemoveDungeonShootLaser(laser);
	dungeon->RemoveEnemyBlocker(selectedBlockerEnemy);
	dungeon->RemoveEnemyChase(selectedChaseEnemy);
	dungeon->RemoveEnemySwitcher(selectedSwitcherEnemy);
	dungeon->RemoveCheckpoint(selectedCheckpoint);
	dungeon->RemoveMirror(selectedMirror);
	dungeon->RemoveSlug(selectedMurderinaEnemy);
	for (auto& decoration : selectedTileDecorations)	dungeon->RemoveDecoration(decoration);
	dungeon->RemoveLight(selectedLight);
	dungeon->RemoveEventTrigger(selectedEventTrigger);
	dungeon->RemoveStairs(selectedStairs);

	ivec2 currentTile = selectedTilePosition;
	TileEditUnselectAll();
	selectedTile = dungeon->GetTile(currentTile);
	RefreshSelectedTile();
}

void Crawl::DungeonEditor::TileEditMoveAllObjectsOnTile(ivec2 position)
{
	for (auto& lever : selectedTileLevers)
	{
		lever->position = position;
		lever->UpdateTransform();
	}
	for (auto& door : selectedTileDoors)
	{
		door->position = position;
		door->UpdateTransforms();
	}
	if (selectedTileKey)
	{
		selectedTileKey->position = position;
		selectedTileKey->UpdateTransform();
	}

	if(selectedTransporter) selectedTransporter->position = position;
	if (selectedActivatorPlate)
	{
		selectedActivatorPlate->position = position;
		selectedActivatorPlate->UpdateTransforms();
	}

	if (selectedHasSpikes)
	{
		DungeonSpikes* spikes = dungeon->GetSpikesAtPosition(selectedTile->position);
		spikes->position = position;
		spikes->UpdateTransform();
	}
	if (selectedHasBlock)
	{
		DungeonPushableBlock* block = dungeon->GetPushableBlockAtPosition(selectedTile->position);
		block->position = position;
		block->UpdateTransform();
	}

	for (auto& laser : selectedTileShootLasers)
	{
		laser->position = position;
		laser->UpdateTransform();
	}

	if (selectedBlockerEnemy)
	{
		selectedBlockerEnemy->position = position;
		selectedBlockerEnemy->UpdateTransform();
	}

	if (selectedChaseEnemy)
	{
		selectedChaseEnemy->position = position;
		selectedChaseEnemy->UpdateTransform();
	}

	if (selectedSwitcherEnemy)
	{
		selectedSwitcherEnemy->position = position;
		selectedSwitcherEnemy->UpdateTransform();
	}

	if(selectedCheckpoint) selectedCheckpoint->position = position;

	if (selectedMirror)
	{

	selectedMirror->position = position;
	selectedMirror->UpdateTransform();
	}

	if (selectedMurderinaEnemy)
	{

	selectedMurderinaEnemy->position = position;
	selectedMurderinaEnemy->UpdateTransform();
	}

	for (auto& decoration : selectedTileDecorations)
	{
		decoration->position = position;
		decoration->UpdateTransform();
	}

	if (selectedLight)
	{

	selectedLight->position = position;
	selectedLight->UpdateTransform();
	}
	
	if(selectedEventTrigger)	selectedEventTrigger->position = position;


	ivec2 currentTile = selectedTilePosition;
	TileEditUnselectAll();
	selectedTile = dungeon->GetTile(currentTile);
	RefreshSelectedTile();
}

void Crawl::DungeonEditor::TileEditCopyDecorationsOnTile(ivec2 position)
{
	for (auto& decoration : selectedTileDecorations)
	{
		DungeonDecoration* copy = dungeon->CreateDecoration(position, decoration->facing);
		copy->castsShadows = decoration->castsShadows;
		copy->localPosition = decoration->localPosition;
		copy->localRotation = decoration->localRotation;
		copy->modelName = decoration->modelName;
		copy->UpdateShadowCasting();
		copy->LoadDecoration();
	}
}

void Crawl::DungeonEditor::UpdateModeMurderinaBrush()
{
	if (murderinaPathCursor == nullptr)
	{
		murderinaPathCursor = Scene::CreateObject();
		murderinaPathCursor->LoadFromJSON(ReadJSONFromDisk("crawler/object/prototype/slug_rail_nothing.object"));
	}

	gridSelected = GetMousePosOnGrid();
	murderinaPathCursor->SetLocalPosition(dungeonPosToObjectScale(gridSelected));

	if (Input::Mouse(0).Down())
	{
		DungeonEnemySlugPath* newPath = dungeon->CreateSlugPath(gridSelected);
		if (newPath)
		{
			MarkUnsavedChanges();
			if (murderinaPathAutoTile) newPath->AutoGenerateMask();
			if (murderinaPathAutoTile && murderinaPathUpdateNeighbors) newPath->RefreshNeighbors();

			newPath->RefreshObject();
		}

	}

	if (Input::Mouse(2).Down())
	{
		DungeonEnemySlugPath* toDelete = dungeon->GetSlugPath(gridSelected);
		if (toDelete)
		{
			if (murderinaPathAutoTile && murderinaPathUpdateNeighbors) toDelete->RemoveConnectionsFromNeighbors();
			dungeon->RemoveSlugPath(toDelete);
			MarkUnsavedChanges();
		}
	}
}
void Crawl::DungeonEditor::UpdateModeMurderinaEdit()
{
	if (Input::Mouse(0).Down())
	{
		murderinaPathSelected = dungeon->GetSlugPath(GetMousePosOnGrid());
		if (murderinaPathSelected)
		{
			murderinaSelectedPathTraversable[NORTH_INDEX] = (murderinaPathSelected->maskTraverse & NORTH_MASK) == NORTH_MASK;
			murderinaSelectedPathTraversable[WEST_INDEX] = (murderinaPathSelected->maskTraverse & WEST_MASK) == WEST_MASK;
			murderinaSelectedPathTraversable[EAST_INDEX] = (murderinaPathSelected->maskTraverse & EAST_MASK) == EAST_MASK;
			murderinaSelectedPathTraversable[SOUTH_INDEX] = (murderinaPathSelected->maskTraverse & SOUTH_MASK) == SOUTH_MASK;

		}
	}
}

void Crawl::DungeonEditor::DrawTileInformation(DungeonTile* tile, bool drawBorder)
{
	vec3 tilePos = dungeonPosToObjectScale(tile->position);
	if (drawBorder)
	{
		LineRenderer::DrawFlatBox(dungeonPosToObjectScale(selectedTilePosition), 1, vec3(0.5, 0.5, 0.5));
	}

	if (!tile->permanentlyOccupied)
	{
		glm::vec3 tileOrigin = dungeonPosToObjectScale(tile->position);
		for (int i = 0; i < 4; i++)
			if ((tile->maskTraverse & orientationMasksIndex[i]) == orientationMasksIndex[i]) LineRenderer::DrawLine(tileOrigin, tileOrigin + dungeonPosToObjectScale(directions[i]) * 0.5f, { 0,1,0 });


		tileOrigin.z += 0.2f;
		for (int i = 0; i < 4; i++)
			if ((tile->maskSee & orientationMasksIndex[i]) == orientationMasksIndex[i]) LineRenderer::DrawLine(tileOrigin, tileOrigin + dungeonPosToObjectScale(directions[i]) * 0.5f, { 0.5,.5,1 });
	}
	else
	{
		for (int i = 0; i < 4; i++)
		{
			int nextI = i + 1;
			if (nextI == 4) nextI = 0;
			vec3 a = dungeonPosToObjectScale(directionsDiagonal[i]) * 0.4f;;
			vec3 b = dungeonPosToObjectScale(directionsDiagonal[nextI]) * 0.4f;;

			LineRenderer::DrawLine(tilePos + a, tilePos + b, vec3(1, 0, 0));
		}
	}
}

void Crawl::DungeonEditor::RefreshSelectedTile()
{
	if (selectedTile)
	{
		selectedTileUntraversableWalls[0] = (selectedTile->maskTraverse & NORTH_MASK) == NORTH_MASK; // North Check
		selectedTileUntraversableWalls[1] = (selectedTile->maskTraverse & SOUTH_MASK) == SOUTH_MASK; // South Check
		selectedTileUntraversableWalls[2] = (selectedTile->maskTraverse & EAST_MASK) == EAST_MASK; // East Check
		selectedTileUntraversableWalls[3] = (selectedTile->maskTraverse & WEST_MASK) == WEST_MASK; // West Check

		selectedTileSeeThroughWalls[0] = (selectedTile->maskSee & NORTH_MASK) == NORTH_MASK; // North Check
		selectedTileSeeThroughWalls[1] = (selectedTile->maskSee & SOUTH_MASK) == SOUTH_MASK; // South Check
		selectedTileSeeThroughWalls[2] = (selectedTile->maskSee & EAST_MASK) == EAST_MASK; // East Check
		selectedTileSeeThroughWalls[3] = (selectedTile->maskSee & WEST_MASK) == WEST_MASK; // West Check
	}

	selectedTileOccupied = false;

	// update list of tiles doors and levers / interactabes
	selectedTileKey = dungeon->GetKeyAtPosition(selectedTilePosition);

	selectedTileDoors.clear();
	for (int i = 0; i < dungeon->activatable.size(); i++)
	{
		if (dungeon->activatable[i]->position == selectedTilePosition)
			selectedTileDoors.push_back(dungeon->activatable[i]);
	}

	selectedTileLevers.clear();
	for (int i = 0; i < dungeon->interactables.size(); i++)
	{
		if (dungeon->interactables[i]->position == selectedTilePosition)
			selectedTileLevers.push_back(dungeon->interactables[i]);
	}

	selectedTileShootLasers.clear();
	for (int i = 0; i < dungeon->shootLasers.size(); i++)
	{
		if (dungeon->shootLasers[i]->position == selectedTilePosition)
			selectedTileShootLasers.push_back(dungeon->shootLasers[i]);
	}

	selectedActivatorPlate = nullptr;
	for (int i = 0; i < dungeon->activatorPlates.size(); i++)
	{
		if (dungeon->activatorPlates[i]->position == selectedTilePosition)
		{
			selectedActivatorPlate = dungeon->activatorPlates[i];
			selectedTileOccupied = true;
		}
	}

	selectedTransporter = nullptr;
	for (int i = 0; i < dungeon->transporterPlates.size(); i++)
	{
		if (dungeon->transporterPlates[i]->position == selectedTilePosition)
		{
			selectedTransporter = dungeon->transporterPlates[i];
			selectedTileOccupied = true;
			RefreshSelectedTransporterData(selectedTransporter->toDungeon + dungeon->dungeonFileExtension);
		}
	}

	selectedCheckpoint = dungeon->GetCheckpointAt(selectedTilePosition); // holy smokes this is a little better!
	if (selectedCheckpoint)
		selectedTileOccupied = true;

	selectedHasSpikes = false;
	for (int i = 0; i < dungeon->spikesPlates.size(); i++)
	{
		if (dungeon->spikesPlates[i]->position == selectedTilePosition)
		{
			selectedHasSpikes = true;
			selectedTileOccupied = true;
		}
	}

	selectedBlockerEnemy = nullptr;
	for (int i = 0; i < dungeon->blockers.size(); i++)
	{
		if (dungeon->blockers[i]->position == selectedTilePosition)
		{
			selectedBlockerEnemy = dungeon->blockers[i];
			selectedTileOccupied = true;
		}
	}

	selectedChaseEnemy = nullptr;
	for (int i = 0; i < dungeon->chasers.size(); i++)
	{
		if (dungeon->chasers[i]->position == selectedTilePosition)
		{
			selectedChaseEnemy = dungeon->chasers[i];
			selectedTileOccupied = true;
		}
	}

	selectedMurderinaEnemy = nullptr;
	for (int i = 0; i < dungeon->slugs.size(); i++)
	{
		if (dungeon->slugs[i]->position == selectedTilePosition)
		{
			selectedMurderinaEnemy = dungeon->slugs[i];
			selectedTileOccupied = true;
		}
	}

	selectedSwitcherEnemy = nullptr;
	for (int i = 0; i < dungeon->switchers.size(); i++)
	{
		if (dungeon->switchers[i]->position == selectedTilePosition)
		{
			selectedSwitcherEnemy = dungeon->switchers[i];
			selectedTileOccupied = true;
		}
	}

	selectedHasBlock = false;
	for (int i = 0; i < dungeon->pushableBlocks.size(); i++)
	{
		if (dungeon->pushableBlocks[i]->position == selectedTilePosition)
		{
			selectedHasBlock = true;
			selectedTileOccupied = true;
		}
	}

	selectedMirror = nullptr;
	for (int i = 0; i < dungeon->mirrors.size(); i++)
	{
		if (dungeon->mirrors[i]->position == selectedTilePosition)
		{
			selectedMirror = dungeon->mirrors[i];
			selectedTileOccupied = true;
		}
	}

	selectedTileDecorations.clear();
	for (int i = 0; i < dungeon->decorations.size(); i++)
	{
		if (dungeon->decorations[i]->position == selectedTilePosition)
			selectedTileDecorations.push_back(dungeon->decorations[i]);
	}

	selectedStairs = nullptr;
	for (int i = 0; i < dungeon->stairs.size(); i++)
	{
		if (dungeon->stairs[i]->startPosition == selectedTilePosition)
			selectedStairs = dungeon->stairs[i];
	}

	selectedLight = nullptr;
	for (int i = 0; i < dungeon->pointLights.size(); i++)
	{
		if (dungeon->pointLights[i]->position == selectedTilePosition)
			selectedLight = dungeon->pointLights[i];
	}

	selectedEventTrigger = nullptr;
	for (int i = 0; i < dungeon->events.size(); i++)
	{
		if (dungeon->events[i]->position == selectedTilePosition)
			selectedEventTrigger = dungeon->events[i];
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
			if(decorationsShowAllModels) decorations.push_back(d.path().generic_string());
			else if(d.path().string().find("decoration") != string::npos) decorations.push_back(d.path().generic_string());
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

void Crawl::DungeonEditor::DrawGizmos()
{
	DrawCompassAtCoordinate(GetMousePosOnGrid());

	// All transporters
	for (auto& transporter : dungeon->transporterPlates)
	{
		vec3 position = dungeonPosToObjectScale(transporter->position) + vec3(-0.5f, -0.5f, 0.0f);
		LineRenderer::DrawFlatBox(position, 0.2f);
		LineRenderer::DrawFlatBox(position, 0.3f);
		glm::vec2 direction = directions[transporter->fromOrientation];
		LineRenderer::DrawLine(position, position + vec3(direction.x, direction.y, 0) * 0.5f);
	}

	// All checkpoints
	for (auto& checkpoint : dungeon->checkpoints)
	{
		vec3 position = dungeonPosToObjectScale(checkpoint->position) + vec3(-0.5f, 0.5f, 0.0f);
		LineRenderer::DrawFlatBox(position, 0.2f, { 0.2f,0.2f, 1 });
		LineRenderer::DrawFlatBox(position, 0.3f, { 0.2f,0.2f, 1 });
		glm::vec2 direction = directions[checkpoint->facing];
		LineRenderer::DrawLine(position, position + vec3(direction.x, direction.y, 0) * 0.5f, { 0.2f,0.2f, 1 });
	}

	// All Events
	for (auto& event : dungeon->events)
	{
		vec3 position = dungeonPosToObjectScale(event->position) + vec3(0.5f, 0.5f, 0.0f);
		LineRenderer::DrawFlatBox(position, 0.2f, { 1.0f, 0.5f, 0.5f });
		LineRenderer::DrawFlatBox(position, 0.3f, { 1.0f, 0.5f, 0.5f });
		glm::vec2 direction = directions[event->facing];
		if (event->mustBeFacing)
		{
			LineRenderer::DrawLine(position, position + vec3(direction.x, direction.y, 0) * 0.5f, { 1.0f, 0.5f, 0.5f });
		}

	}

	// All Lights
	for (auto& light : dungeon->pointLights)
	{
		vec3 positionA = dungeonPosToObjectScale(light->position) + light->localPosition;
		vec3 positionB = positionA;
		positionA.z = 0.0f;
		LineRenderer::DrawLine(positionA, positionB, light->colour);
	}

	// All decorations
	for (auto& decoration : dungeon->decorations)
	{
		vec3 position = dungeonPosToObjectScale(decoration->position) + vec3(0.5f, -0.5f, 0.0f);
		LineRenderer::DrawFlatBox(position, 0.2f, { 1.0f, 0.0f, 1.0f });
		LineRenderer::DrawFlatBox(position, 0.3f, { 1.0f, 0.0f, 1.0f });
	}

	if (murderinaPathSelected)
	{
		vec3 position = dungeonPosToObjectScale(murderinaPathSelected->position);
		vec3 colour = { 0, 1,1 };
		if (!murderinaPathSelected->IsValidConfiguration())  colour = { 1, 0, 0 };
		LineRenderer::DrawFlatBox(position, 0.2f, colour);
	}

	if (editMode == Mode::TileEdit)
	{
		if (selectedTile) DrawTileInformation(selectedTile, true);
		else LineRenderer::DrawFlatBox(dungeonPosToObjectScale(selectedTilePosition), 1, vec3(0.5, 0.5, 0.5));
	}
}

void Crawl::DungeonEditor::DrawCompassAtCoordinate(ivec2 coordinate)
{
	vec3 colourNorth = { 1,0,0 };
	vec3 colourSouth = { 0,0,1 };
	float pinLength = 0.5f;

	vec3 position = dungeonPosToObjectScale(coordinate);
	vec3 northDirection = dungeonPosToObjectScale(NORTH_COORDINATE) * pinLength;
	vec3 eastDirection = dungeonPosToObjectScale(EAST_COORDINATE) * pinLength * pinLength;

	LineRenderer::DrawLine(position, position + northDirection, colourNorth);
	LineRenderer::DrawLine(position, position - northDirection, colourSouth);
	LineRenderer::DrawLine(position, position + eastDirection);
	LineRenderer::DrawLine(position, position - eastDirection);
}

void Crawl::DungeonEditor::DrawCompassAtPosition(vec3 position)
{
	vec3 colourNorth = { 1,0,0 };
	vec3 colourSouth = { 0,0,1 };
	float pinLength = 0.5f;

	vec3 northDirection = dungeonPosToObjectScale(NORTH_COORDINATE);
	vec3 eastDirection = dungeonPosToObjectScale(EAST_COORDINATE);

	LineRenderer::DrawLine(position, position + northDirection, colourNorth);
	LineRenderer::DrawLine(position, position - northDirection, colourSouth);
	LineRenderer::DrawLine(position, position + eastDirection);
	LineRenderer::DrawLine(position, position - eastDirection);
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

int Crawl::DungeonEditor::GetNextAvailableLightID()
{
	int nextAvail = 1;
	bool unused = false;
	while (!unused)
	{
		unused = true;
		for (int i = 0; i < dungeon->pointLights.size(); i++)
		{
			if (dungeon->pointLights[i]->id == nextAvail)
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
	groundPos = rayStart - (rayDir * scale);

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
	LogUtils::Log("Saved.");
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
	selectedTileKey = nullptr;
	selectedTransporter = nullptr;
	selectedActivatorPlate = nullptr;
	selectedTileShootLaser = nullptr;
	selectedBlockerEnemy = nullptr;
	selectedChaseEnemy = nullptr;
	selectedSwitcherEnemy = nullptr;
	selectedCheckpoint = nullptr;
	selectedMirror = nullptr;
	selectedMurderinaEnemy = nullptr;
	selectedTileDecoration = nullptr;
	selectedStairs = nullptr;
	selectedLight = nullptr;
	selectedEventTrigger = nullptr;
	murderinaPathSelected = nullptr;

	selectedDoorWindowOpen = false;
	selectedKeyWindowOpen = false;
	selectedLeverWindowOpen = false;
	selectedActivatorPlateWindowOpen = false;
	selectedTransporterWindowOpen = false;
	selectedShootLaserWindowOpen = false;
	selectedBlockerEnemyWindowOpen = false;
	selectedChaseEnemyWindowOpen = false;
	selectedSwitcherEnemyWindowOpen = false;
	selectedCheckpointWindowOpen = false;
	selectedMirrorWindowOpen = false;
	selectedMurdurinaWindowOpen = false;
	selectedDecorationWindowOpen = false;
	selectedStairsWindowOpen = false;
	selectedLightWindowOpen = false;
	selectedEventTriggerWindowOpen = false;

	selectedTileDoors.clear();
	selectedTileLevers.clear();
	selectedTileShootLasers.clear();
	selectedTileDecorations.clear();
}
