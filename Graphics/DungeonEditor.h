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
			MurderinaPathBrush,
			MurderinaPathEdit,
			DungeonProperties,
			GameManager,
		};

		DungeonEditor();

		void Activate();
		void Deactivate();
		bool requestedGameMode = false;
		bool dirtyGameplayScene = false;

		void SetDungeon(Dungeon* dungeonPtr);
		void DrawGUI();
		void DrawGUIFileOperations();
		void DrawGUIModeSelect();
		void DrawGUIModeTileBrush();
		void DrawGUIModeTileEdit();
		void DrawGUIModeTileEditDoor();
		void DrawGUIModeTileEditLever();
		void DrawGUIModeTileEditKey();
		void DrawGUIModeTileEditPlate();
		void DrawGUIModeTileEditTransporter();
		void DrawGUIModeTileEditCheckpoint();

		void DrawGUIModeTileEditShootLaser();
		void DrawGUIModeTileEditBlocker();
		void DrawGUIModeTileEditChase();
		void DrawGUIModeTileEditSwitcher();
		void DrawGUIModeTileEditMirror();
		void DrawGUIModeTileEditMurderina();
		void DrawGUIModeTileEditDecoration();

		void DrawGUIModeTileEditStairs();
		void DrawGUIModeTileEditLight();
		void DrawGUIModeTileEditEventTrigger();
		string DrawGUIModeTileEditEventTriggerGetEventTypeString(DungeonEventTrigger* trigger);


		void DrawGUIModeRailBrush();
		void DrawGUIModeRailEdit();
		void DrawGUIModeRailLines();
		void DrawGUIModeDungeonProperties();
		void DrawGUIModeGameManager();

		void Update();
		void UpdateModeTileBrush();
		void UpdateModeTileEdit();

		void TileEditDeleteAllObjectsOnTile();
		void TileEditMoveAllObjectsOnTile(ivec2 position);
		void TileEditCopyDecorationsOnTile(ivec2 position);


		void UpdateModeMurderinaBrush();
		void UpdateModeMurderinaEdit();


		void DrawTileInformation(DungeonTile* tile, bool drawBorder = false);

		void RefreshSelectedTile();
		void RefreshSelectedTransporterData(string dungeonPath);
		void RefreshAvailableDecorations();
		void RefreshDungeonFileNames();

		void DrawGizmos();
		void DrawCompassAtCoordinate(ivec2 coordinate);
		void DrawCompassAtPosition(glm::vec3 position);

		int GetNextAvailableLeverID();
		int GetNextAvailableDoorID();
		int GetNextAvailableLightID();
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
		glm::vec3 groundPos;

		// File Operations
		std::string dungeonFileName = "new_dungeon";
		std::string dungeonFileNameSaveAs = "";
		std::string dungeonFilePath = "";
		std::string dungeonWantLoad = "";

		bool saveAsPrompt = false;
		bool confirmOverwritePrompt = false;
		bool didSaveAs = false;
		bool unsavedChanges = true;
		bool shouldConfirmSaveBeforeLoad = false;

		// Wall Variants
		const int WALL_VARIANT_COUNT = 3;

		Mode editMode = Mode::TileBrush;
		std::string editModeNames[6]{ "Tile Brush", "Tile Edit", "Rail Brush", "Rail Edit", "Dungeon Properties", "Game Manager"};

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
		std::vector<std::string> floorVarientShortNames;

		DungeonTile* selectedTile = nullptr;
		ivec2 selectedTilePosition = { 0,0 };
		ivec2 selectedTileMoveObjectsTo = { 0,0 };
		float selectedTileMoveFlash = 0.0f;;
		bool selectedTileUntraversableWalls[4] = { false, false, false, false };
		bool selectedTileSeeThroughWalls[4] = { false, false, false, false };
		bool selectedTileOccupied = false;

		std::vector<DungeonDoor*> selectedTileDoors;
		DungeonDoor* selectedDoor = nullptr;
		bool selectedDoorWindowOpen = false;

		std::vector<DungeonInteractableLever*> selectedTileLevers;
		DungeonInteractableLever* selectedLever = nullptr;
		bool selectedLeverWindowOpen = false;

		DungeonCollectableKey* selectedTileKey;
		bool selectedKeyWindowOpen = false;

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

		DungeonEnemySlug* selectedMurderinaEnemy = nullptr;
		bool selectedMurdurinaWindowOpen = false;

		DungeonMirror* selectedMirror = nullptr;
		bool selectedMirrorWindowOpen = false;

		std::vector<DungeonDecoration*> selectedTileDecorations;
		DungeonDecoration* selectedTileDecoration = nullptr;
		bool selectedDecorationWindowOpen = false;
		std::vector<string> decorations;
		bool decorationsShowAllModels = false;

		DungeonStairs* selectedStairs = nullptr;
		bool selectedStairsWindowOpen = false;
		int selectedStairsGizmoIndex = 0;

		DungeonLight* selectedLight = nullptr;
		bool selectedLightWindowOpen = false;

		DungeonEventTrigger* selectedEventTrigger = nullptr;
		bool selectedEventTriggerWindowOpen = false;

		// Slug Path Editor Mode
		Object* murderinaPathCursor = nullptr;
		DungeonEnemySlugPath* murderinaPathSelected = nullptr;
		bool murderinaPathAutoTile = true;
		bool murderinaPathUpdateNeighbors = true;
		bool murderinaSelectedPathTraversable[4] = { false, false, false, false };

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

