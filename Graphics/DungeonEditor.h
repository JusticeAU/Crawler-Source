#pragma once
#include "Dungeon.h"
#include "glm.hpp"
#include "Window.h"
#include <vector>

namespace Crawl
{
	class DungeonEditor
	{
	public:
		enum class Mode
		{
			TileBrush,
			TileEdit,
			EntityEdit,
			DungeonProperties
		};

		DungeonEditor();

		void Activate();
		void Deactivate();
		bool requestedGameMode = false;
		bool dirtyGameplayScene = false;

		void SetDungeon(Dungeon* dungeonPtr) { dungeon = dungeonPtr; }
		void DrawGUI();
		void DrawGUIFileOperations();
		void DrawGUICursorInformation();
		void DrawGUIModeSelect();
		void DrawGUIModeTileBrush();
		void DrawGUIModeTileEdit();
		void DrawGUIModeTileEditDoor();
		void DrawGUIModeTileEditLever();
		void DrawGUIModeTileEditPlate();
		void DrawGUIModeTileEditTransporter();
		void DrawGUIModeTileEditCheckpoint();

		void DrawGUIModeTileEditShootLaser();
		void DrawGUIModeTileEditBlocker();
		void DrawGUIModeTileEditChase();
		void DrawGUIModeTileEditSwitcher();
		void DrawGUIModeTileEditMirror();


		void DrawGUIModeDungeonProperties();

		void Update();
		void UpdateModeTileBrush();
		void UpdateModeTileEdit();

		void RefreshSelectedTile();

		int GetNextAvailableLeverID();
		int GetNextAvailableDoorID();
	protected:
		glm::ivec2 GetMousePosOnGrid();
		void UpdateMousePosOnGrid();
		void UpdateAutoTile(ivec2 position);
		void UpdateWallVariants(DungeonTile* tile);
		void UpdateSurroundingTiles(ivec2 position);

		// Save the dungeon to file.
		void Save();
		void Load(string path);

		std::string GetDungeonFilePath();

		void MarkUnsavedChanges();
		void UnMarkUnsavedChanges();

		void TileEditUnselectAll();

		Dungeon* dungeon = nullptr;
		glm::ivec2 gridSelected = { 0, 0 };

		// File Operations
		std::string subfolder = "crawler/dungeon/";
		std::string extension = ".dungeon";
		std::string dungeonFileName = "new_dungeon";
		std::string dungeonFileNameSaveAs = "";
		std::string dungeonFilePath = "";
		std::string dungeonWantLoad = "";

		bool saveAsPrompt = false;
		bool confirmOverwritePrompt = false;
		bool didSaveAs = false;

		// Wall Variants
		const int WALL_VARIANT_COUNT = 3;

		Mode editMode = Mode::TileBrush;
		std::string editModeNames[4]{ "Tile Brush", "Tile Edit", "Entity Edit", "Dungeon Properties" };

		// Brush Mode
		unsigned int brush_tileMask = 0;
		// Auto Tile configuration
		bool brush_AutoTileEnabled = true;
		bool brush_AutoTileSurround = true;

		// Brush Tile
		bool brush_TileNorth = true;
		bool brush_TileSouth = true;
		bool brush_TileEast = true;
		bool brush_TileWest = true;

		// Tile edit mode
		DungeonTile* selectedTile = nullptr;
		bool selectedTileOpenWalls[4] = { false, false, false, false };
		bool selectedTileOccupied = false;
		std::vector<DungeonDoor*> selectedTileDoors;
		std::vector<DungeonInteractableLever*> selectedTileLevers;
		DungeonDoor* selectedDoor = nullptr;
		bool selectedDoorWindowOpen = false;
		DungeonInteractableLever* selectedLever = nullptr;
		bool selectedLeverWindowOpen = false;
		DungeonActivatorPlate* selectedActivatorPlate = nullptr;
		bool selectedActivatorPlateWindowOpen = false;
		DungeonTransporter* selectedTransporter = nullptr;
		bool selectedTransporterWindowOpen = false;
		DungeonCheckpoint* selectedCheckpoint = nullptr;
		bool selectedCheckpointWindowOpen = false;
		bool selectedHasSpikes = false;
		bool selectedHasBlock = false;

		std::vector<DungeonShootLaser*> selectedTileShootLasers;
		DungeonShootLaser* selectedTileShootLaser = nullptr;
		bool selectedShootLaserWindowOpen = false;

		DungeonEnemyBlocker* selectedBlockerEnemy = nullptr;
		bool selectedBlockerEnemyWindowOpen = false;

		DungeonEnemyChase* selectedChaseEnemy = nullptr;
		bool selectedChaseEnemyWindowOpen = false;

		DungeonEnemySwitcher* selectedSwitcherEnemy = nullptr;
		bool selectedSwitcherEnemyWindowOpen = false;

		DungeonMirror* selectedMirror = nullptr;
		bool selectedMirrorWindowOpen = false;

		bool unsavedChanges = false;
		bool shouldConfirmSaveBeforeLoad = false;

		//Path Finding Dev
		ivec2 from = { 0,0 };
		ivec2 to = { 0,0 };
		std::vector<Object*> pathFindObjects;
		ordered_json path_template = ReadJSONFromDisk("crawler/object/testing/path.object");
		int facingTest = 0;
	};
}

