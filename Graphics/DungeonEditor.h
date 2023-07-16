#pragma once
#include "Dungeon.h"
#include "glm.hpp"

namespace Crawl
{
	class DungeonEditor
	{
	public:
		enum class Mode
		{
			TileBrush,
			TileEdit,
			EntityEdit
		};

		DungeonEditor();

		void Activate();
		void Deactivate();

		void SetDungeon(Dungeon* dungeonPtr) { dungeon = dungeonPtr; }
		void DrawGUI();
		void DrawGUIFileOperations();
		void DrawGUICursorInformation();
		void DrawGUIModeSelect();
		void DrawGUIModeTileBrush();
		void DrawGUIModeTileEdit();
		void DrawGUIModeTileEditDoor();
		void DrawGUIModeTileEditLever();


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

		std::string GetDungeonFilePath();

		Dungeon* dungeon = nullptr;
		glm::ivec2 gridSelected = { 0, 0 };

		// File Operations
		std::string subfolder = "crawler/dungeon/";
		std::string extension = ".dungeon";
		std::string dungeonFileName = "new_dungeon";
		std::string dungeonFileNameSaveAs = "";
		std::string dungeonFilePath = "";

		bool saveAsPrompt = false;
		bool confirmOverwritePrompt = false;
		bool didSaveAs = false;

		// Wall Variants
		const int WALL_VARIANT_COUNT = 3;

		Mode editMode = Mode::TileBrush;
		std::string editModeNames[2]{ "Tile Brush", "Tile Edit" };

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
		std::vector<DungeonDoor*> selectTileDoors;
		std::vector<DungeonInteractableLever*> selectTileLevers;
		DungeonDoor* selectedDoor = nullptr;
		bool selectedDoorWindowOpen = false;
		DungeonInteractableLever* selectedLever = nullptr;
		bool selectedLeverWindowOpen = false;

	};
}

