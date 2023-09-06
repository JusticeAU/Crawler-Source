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
			DungeonProperties,
			SlugPathEditor
		};

		DungeonEditor();

		void Activate();
		void Deactivate();
		bool requestedGameMode = false;
		bool dirtyGameplayScene = false;

		void SetDungeon(Dungeon* dungeonPtr);
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
		void DrawGUIModeTileEditSlug();
		void DrawGUIModeTileEditDecoration();

		void DrawGUIModeTileEditStairs();


		void DrawGUIModeDungeonProperties();

		void Update();
		void UpdateModeTileBrush();
		void UpdateModeTileEdit();
		void UpdateModeSlugPathEdit();

		void DrawTileInformation(DungeonTile* tile);

		void RefreshSelectedTile();
		void RefreshSelectedTransporterData(string dungeonPath);
		void RefreshAvailableDecorations();
		void RefreshDungeonFileNames();

		int GetNextAvailableLeverID();
		int GetNextAvailableDoorID();
	protected:
		glm::ivec2 GetMousePosOnGrid();
		void UpdateMousePosOnGrid();
		void UpdateAutoTile(ivec2 position);
		void SetDefaultWallVarients(DungeonTile* tile);
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
		std::string editModeNames[5]{ "Tile Brush", "Tile Edit", "Entity Edit", "Dungeon Properties", "Slug Path Editor"};

		// Brush Mode
		Object* brushObject = nullptr;
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
		std::vector<std::string> wallVarientShortNames;
		DungeonTile* selectedTile = nullptr;
		bool selectedTileUntraversableWalls[4] = { false, false, false, false };
		bool selectedTileSeeThroughWalls[4] = { false, false, false, false };
		bool selectedTileOccupied = false;

		std::vector<DungeonDoor*> selectedTileDoors;
		DungeonDoor* selectedDoor = nullptr;
		bool selectedDoorWindowOpen = false;

		std::vector<DungeonInteractableLever*> selectedTileLevers;
		DungeonInteractableLever* selectedLever = nullptr;
		bool selectedLeverWindowOpen = false;

		DungeonActivatorPlate* selectedActivatorPlate = nullptr;
		bool selectedActivatorPlateWindowOpen = false;

		DungeonTransporter* selectedTransporter = nullptr;
		bool selectedTransporterWindowOpen = false;
		bool selectedTransporterToDungeonLoaded = false;
		ordered_json selectedTransporterToDungeonJSON;

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

		DungeonEnemySlug* selectedSlugEnemy = nullptr;
		bool selectedSlugEnemyWindowOpen = false;

		DungeonMirror* selectedMirror = nullptr;
		bool selectedMirrorWindowOpen = false;

		std::vector<DungeonDecoration*> selectedTileDecorations;
		DungeonDecoration* selectedTileDecoration = nullptr;
		bool selectedDecorationWindowOpen = false;
		std::vector<string> decorations;

		DungeonStairs* selectedStairs = nullptr;
		bool selectedStairsWindowOpen = false;
		int selectedStairsGizmoIndex = 0;

		bool unsavedChanges = false;
		bool shouldConfirmSaveBeforeLoad = false;

		// Slug Path Editor Mode
		Object* slugPathCursor = nullptr;

		//Path Finding Dev
		ivec2 from = { 0,0 };
		ivec2 to = { 0,0 };
		std::vector<Object*> pathFindObjects;
		ordered_json path_template = ReadJSONFromDisk("crawler/object/testing/path.object");
		int facingTest = 0;

		// Beauty Scene Config
		Object* beautySceneLights = nullptr;
	};
}

