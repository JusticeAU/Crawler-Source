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

#include "Object.h"
#include "Input.h"
#include "Camera.h"
#include "Scene.h"
#include <filesystem>
namespace fs = std::filesystem;

#include "ComponentModel.h";
#include "ComponentRenderer.h";

#include "LogUtils.h"


Crawl::DungeonEditor::DungeonEditor()
{
	
}

void Crawl::DungeonEditor::Activate()
{
	Scene::ChangeScene("Dungeon");
	Scene::SetCameraIndex(0);

	// It's possible that gameplay took us in to another dungeon, and this, that is the one that should now be loaded.
	dungeonFileName = dungeon->dungeonFileName;
	dungeonFilePath = dungeon->dungeonFilePath;
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
		dirtyGameplayScene = true;
	}

	if (unsavedChanges)
		ImGui::EndDisabled();


	ImGui::SameLine();
	if (ImGui::Button("Reset Dungeon"))
	{
		UnMarkUnsavedChanges();
		dungeon->RebuildFromSerialised();
		dungeon->player->Teleport(dungeon->defaultPlayerStartPosition);
		dungeon->player->Orient(dungeon->defaultPlayerStartOrientation);
		dirtyGameplayScene = false;
		TileEditUnselectAll();
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
				string foundDungeonPath = d.path().relative_path().string();
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

	if (ImGui::BeginCombo("Path Start Orientation", orientationNames[facingTest].c_str()))
	{
		int oldOrientation = facingTest;
		for (int i = 0; i < 4; i++)
		{
			if (ImGui::Selectable(orientationNames[i].c_str()))
				facingTest = i;
		}
		ImGui::EndCombo();
	}
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

	// Cardinal traversable yes/no
	unsigned int oldMask = selectedTile->mask;
	if (ImGui::Checkbox("Open North", &selectedTileOpenWalls[0]))
		selectedTile->mask += selectedTileOpenWalls[0] ? NORTH_MASK : -NORTH_MASK;
	if (ImGui::Checkbox("Open South", &selectedTileOpenWalls[1]))
		selectedTile->mask += selectedTileOpenWalls[1] ? SOUTH_MASK : -SOUTH_MASK;
	if (ImGui::Checkbox("Open East", &selectedTileOpenWalls[2]))
		selectedTile->mask += selectedTileOpenWalls[2] ? EAST_MASK : -EAST_MASK;
	if (ImGui::Checkbox("Open West", &selectedTileOpenWalls[3]))
		selectedTile->mask += selectedTileOpenWalls[3] ? WEST_MASK : -WEST_MASK;
	if (oldMask != selectedTile->mask)
	{
		MarkUnsavedChanges();
		UpdateWallVariants(selectedTile);
		dungeon->CreateTileObject(selectedTile);
	}
	int distance = dungeon->goodPath.size();
	ImGui::InputInt("Distance", &distance);
	ImGui::InputInt("Cost", &selectedTile->cost);
	for (int i = 0; i < 4; i++)
	{
		if(selectedTile->neighbors[i])
			ImGui::Text("neighbor");
		else
			ImGui::Text("nah");
	}

	// if yes, what wall variant - hidden for now, non-functional
	/*ImGui::BeginDisabled();
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

	ImGui::EndDisabled();*/

	// entities
	for (auto& door : selectTileDoors)
	{
		string doorName = "Door ID (";
		doorName += to_string(door->id);
		doorName += ")";
		if (ImGui::Selectable(doorName.c_str()))
		{
			selectedDoor = door;
			selectedDoorWindowOpen = true;
		}
	}
	if (ImGui::Button("Add Door"))
	{
		MarkUnsavedChanges();
		dungeon->CreateDoor(selectedTile->position, 0, GetNextAvailableDoorID(), false);
		RefreshSelectedTile();
	}

	for (auto& lever : selectTileLevers)
	{
		string leverName = "Lever ID (";
		leverName += to_string(lever->id);
		leverName += ")";
		if (ImGui::Selectable(leverName.c_str()))
		{
			selectedLever = lever;
			selectedLeverWindowOpen = true;
		}
	}
	if (ImGui::Button("Add Lever"))
	{
		MarkUnsavedChanges();
		dungeon->CreateLever(selectedTile->position, 0, GetNextAvailableLeverID(), 0, false);
		RefreshSelectedTile();
	}

	// Activator Plates - exclusive on a tile ( combine? )
	if (selectedActivatorPlate)
	{
		if (ImGui::Selectable("Plate")) selectedActivatorPlateWindowOpen = true;

	}
	else if (ImGui::Button("Add Activator Plate"))
	{
		MarkUnsavedChanges();
		dungeon->CreatePlate(selectedTile->position, 0);
		RefreshSelectedTile();
	}

	// Transporters - exclusive on a tile ( combine? )
	if (selectedTransporter)
	{
		if (ImGui::Selectable("transporter")) selectedTransporterWindowOpen = true;

	}
	else if (ImGui::Button("Add Transporter"))
	{
		MarkUnsavedChanges();
		selectedTransporter = dungeon->CreateTransporter(selectedTile->position);
		selectedTransporterWindowOpen = true;
		//RefreshSelectedTile(); // shouldn't need this?
	}

	// Spikes - exclusive on a tile ( combine? )
	if (selectedHasSpikes)
	{	
		if (ImGui::Button("Delete Spikes"))
		{
			MarkUnsavedChanges();
			dungeon->RemoveSpikes(selectedTile->position);
			selectedHasSpikes = false;
		}
	}
	else if (ImGui::Button("Add Spikes"))
	{
		MarkUnsavedChanges();
		dungeon->CreateSpikes(selectedTile->position);
		selectedHasSpikes = true;
	}

	if (selectedBlockerEnemy)
	{
		if (ImGui::Selectable("Sword Blocker")) selectedBlockerEnemyWindowOpen = true;
		if (ImGui::Button("Delete Sword Blocker"))
		{
			MarkUnsavedChanges();
			dungeon->RemoveEnemyBlocker(selectedTile->position);
			selectedBlockerEnemyWindowOpen = false;
		}
	}
	else if (ImGui::Button("Add Sword Blocker"))
	{
		MarkUnsavedChanges();
		selectedBlockerEnemy = dungeon->CreateEnemyBlocker(selectedTile->position, EAST_INDEX);
		selectedBlockerEnemyWindowOpen = true;
	}

	if (selectedChaseEnemy)
	{
		if (ImGui::Selectable("Chaser")) selectedChaseEnemyWindowOpen = true;
		if (ImGui::Button("Delete Chaser"))
		{
			MarkUnsavedChanges();
			dungeon->RemoveEnemyChase(selectedTile->position);
			selectedChaseEnemyWindowOpen = false;
		}
	}
	else if (ImGui::Button("Add Chaser"))
	{
		MarkUnsavedChanges();
		selectedChaseEnemy = dungeon->CreateEnemyChase(selectedTile->position, EAST_INDEX);
		selectedChaseEnemyWindowOpen = true;
	}

	// Pushable Blocks - similar but different to plates - they are closer to NPCs - they occupy a space.
	if (selectedHasBlock)
	{
		if (ImGui::Button("Delete Block"))
		{
			MarkUnsavedChanges();
			dungeon->RemovePushableBlock(selectedTile->position);
			selectedHasBlock = false;
		}
	}
	else if (ImGui::Button("Add Block"))
	{
		MarkUnsavedChanges();
		dungeon->CreatePushableBlock(selectedTile->position);
		selectedHasBlock = true;
	}

	for (auto& shootLaser : selectTileShootLasers)
	{
		string shootLaserName = "shootLaser ID (";
		shootLaserName += to_string(shootLaser->id);
		shootLaserName += ")";
		if (ImGui::Selectable(shootLaserName.c_str()))
		{
			selectedTileShootLaser = shootLaser;
			selectedShootLaserWindowOpen = true;
		}
	}
	if (ImGui::Button("Add Shoot Laser"))
	{
		MarkUnsavedChanges();
		dungeon->CreateShootLaser(selectedTile->position, (FACING_INDEX)0, GetNextAvailableDoorID());
		RefreshSelectedTile();
	}

	if (selectedDoorWindowOpen)
		DrawGUIModeTileEditDoor();
	if (selectedLeverWindowOpen)
		DrawGUIModeTileEditLever();
	if (selectedActivatorPlateWindowOpen)
		DrawGUIModeTileEditPlate();
	if (selectedTransporterWindowOpen)
		DrawGUIModeTileEditTransporter();
	if (selectedShootLaserWindowOpen)
		DrawGUIModeTileEditShootLaser();
	if (selectedBlockerEnemyWindowOpen)
		DrawGUIModeTileEditBlocker();
	if (selectedChaseEnemyWindowOpen)
		DrawGUIModeTileEditChase();
}

void Crawl::DungeonEditor::DrawGUIModeTileEditDoor()
{
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

	if(ImGui::Checkbox("Starts Open", &selectedDoor->startOpen))
		MarkUnsavedChanges();

	if (ImGui::Checkbox("Is Open", &selectedDoor->open))
	{
		selectedDoor->open = !selectedDoor->open;
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
			// remove from list
			for (int i = 0; i < dungeon->activatable.size(); i++)
			{
				if (selectedDoor == dungeon->activatable[i])
				{
					dungeon->activatable.erase(dungeon->activatable.begin()+i);
					break;
				}
			}
			// call delete on pointer
			delete selectedDoor;
			// remove from selected
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

	if(ImGui::Checkbox("Starts Status", &selectedLever->startStatus))
		MarkUnsavedChanges();

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
			// remove from list
			for (int i = 0; i < dungeon->interactables.size(); i++)
			{
				if (selectedLever == dungeon->interactables[i])
				{
					dungeon->interactables.erase(dungeon->interactables.begin() + i);
					break;
				}
			}
			// call delete on pointer
			delete selectedLever;
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

void Crawl::DungeonEditor::DrawGUIModeTileEditPlate()
{
	ImGui::SetNextWindowSize({ 300, 100 });
	ImGui::Begin("Edit Plate", &selectedActivatorPlateWindowOpen, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize);

	ImGui::InputInt("Activate ID", (int*)&selectedActivatorPlate->activateID);
	if (ImGui::Button("Delete"))
	{
		for (auto it = dungeon->activatorPlates.begin(); it != dungeon->activatorPlates.end(); it++)
		{
			if (selectedActivatorPlate == *it)
			{
				dungeon->activatorPlates.erase(it);
				delete selectedActivatorPlate;
				selectedActivatorPlate = nullptr;
				selectedActivatorPlateWindowOpen = false;
				break;
			}
		}
	}

	ImGui::End();
}

void Crawl::DungeonEditor::DrawGUIModeTileEditTransporter()
{
	ImGui::SetNextWindowSize({ 500, 165 });
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
	if(ImGui::InputText("To Dungeon", &selectedTransporter->toDungeon))
		MarkUnsavedChanges();
	if(ImGui::InputText("To Transporter", &selectedTransporter->toTransporter))
		MarkUnsavedChanges();

	if (ImGui::Button("Delete"))
	{
		for (auto it = dungeon->transporterPlates.begin(); it != dungeon->transporterPlates.end(); it++)
		{
			if (selectedTransporter == *it)
			{
				dungeon->transporterPlates.erase(it);
				delete selectedTransporter;
				selectedTransporter = nullptr;
				selectedTransporterWindowOpen = false;
				break;
			}
		}
		MarkUnsavedChanges();
	}

	ImGui::End();
}

void Crawl::DungeonEditor::DrawGUIModeTileEditShootLaser()
{
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

	ImGui::End();
}

void Crawl::DungeonEditor::DrawGUIModeTileEditBlocker()
{
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
}

void Crawl::DungeonEditor::DrawGUIModeTileEditChase()
{
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

	if (ImGui::Checkbox("Player Turn is Free", &dungeon->playerTurnIsFree))
		MarkUnsavedChanges();

	if (ImGui::Checkbox("Player Interact is Free", &dungeon->playerInteractIsFree))
		MarkUnsavedChanges();

	if (ImGui::Checkbox("Player can Knife", &dungeon->playerHasKnife))
		MarkUnsavedChanges();

	if (ImGui::Checkbox("Player can Kick Box ", &dungeon->playerCanKickBox))
		MarkUnsavedChanges();

	if (ImGui::Checkbox("Player Can Push Box", &dungeon->playerCanPushBox))
		MarkUnsavedChanges();


}

void Crawl::DungeonEditor::Update()
{
	switch (editMode)
	{
	case Mode::TileBrush:
		UpdateModeTileBrush();	break;
	case Mode::TileEdit:
		UpdateModeTileEdit();	break;
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
		TileEditUnselectAll();
		glm::ivec2 selectionPos = GetMousePosOnGrid();

		selectedTile = dungeon->GetTile(selectionPos);
		if (!selectedTile)
			return;
		
		RefreshSelectedTile();
	}



	if (Input::Keyboard(GLFW_KEY_LEFT_CONTROL).Down())
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
	}

	if (Input::Keyboard(GLFW_KEY_SPACE).Down())
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
	for (int i = 0; i < dungeon->activatable.size(); i++)
	{
		if (dungeon->activatable[i]->position == selectedTile->position)
			selectTileDoors.push_back(dungeon->activatable[i]);
	}

	selectTileLevers.clear();
	for (int i = 0; i < dungeon->interactables.size(); i++)
	{
		if (dungeon->interactables[i]->position == selectedTile->position)
			selectTileLevers.push_back(dungeon->interactables[i]);
	}

	selectedActivatorPlate = nullptr;
	for (int i = 0; i < dungeon->activatorPlates.size(); i++)
	{
		if (dungeon->activatorPlates[i]->position == selectedTile->position)
			selectedActivatorPlate = dungeon->activatorPlates[i];
	}

	selectedTransporter = nullptr;
	for (int i = 0; i < dungeon->transporterPlates.size(); i++)
	{
		if (dungeon->transporterPlates[i]->position == selectedTile->position)
			selectedTransporter = dungeon->transporterPlates[i];
	}

	selectedHasSpikes = false;
	for (int i = 0; i < dungeon->spikesPlates.size(); i++)
	{
		if (dungeon->spikesPlates[i]->position == selectedTile->position)
			selectedHasSpikes = true;
	}

	selectedBlockerEnemy = nullptr;
	for (int i = 0; i < dungeon->blockers.size(); i++)
	{
		if (dungeon->blockers[i]->position == selectedTile->position)
			selectedBlockerEnemy = dungeon->blockers[i];
	}

	selectedChaseEnemy = nullptr;
	for (int i = 0; i < dungeon->chasers.size(); i++)
	{
		if (dungeon->chasers[i]->position == selectedTile->position)
			selectedChaseEnemy = dungeon->chasers[i];
	}

	selectedHasBlock = false;
	for (int i = 0; i < dungeon->pushableBlocks.size(); i++)
	{
		if (dungeon->pushableBlocks[i]->position == selectedTile->position)
			selectedHasBlock = true;
	}

	selectTileShootLasers.clear();
	for (int i = 0; i < dungeon->shootLasers.size(); i++)
	{
		if (dungeon->shootLasers[i]->position == selectedTile->position)
			selectTileShootLasers.push_back(dungeon->shootLasers[i]);
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
	dungeonFilePath = GetDungeonFilePath();
	dungeon->Save(dungeonFilePath);
	UnMarkUnsavedChanges();
}

void Crawl::DungeonEditor::Load(string path)
{	
	TileEditUnselectAll();
	dungeon->Load(path);
	dungeonFileName = dungeon->dungeonFileName;
	dungeonFileNameSaveAs = dungeon->dungeonFileName;
	dungeonFilePath = path;
	dungeonWantLoad = "";
	UnMarkUnsavedChanges();
}

std::string Crawl::DungeonEditor::GetDungeonFilePath()
{
	std::string filename;
	filename += subfolder;
	filename += dungeonFileNameSaveAs;
	filename += extension;
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

	selectedDoorWindowOpen = false;
	selectedLeverWindowOpen = false;
	selectedActivatorPlateWindowOpen = false;
	selectedTransporterWindowOpen = false;
	selectedShootLaserWindowOpen = false;
	selectedBlockerEnemyWindowOpen = false;
	selectedChaseEnemyWindowOpen = false;
}
