#pragma once
#include "Dungeon.h"
#include "glm.hpp"

namespace Crawl
{
	class DungeonEditor
	{
	public:
		DungeonEditor();

		void Activate();
		void Deactivate();

		void SetDungeon(Dungeon* dungeonPtr) { dungeon = dungeonPtr; }
		void DrawGUI();
		void DrawGUIFileOperations();
		void DrawGUICursorInformation();
		void DrawGUIMode();
		void DrawGUIModeTileBrush();
		void Update();
	protected:
		void UpdateMousePosOnGrid();
		void UpdateTile(int x, int y);
		void UpdateSurroundingTiles(int x, int y);

		// Save the dungeon to file.
		void Save();

		std::string GetDungeonFilePath();

		Dungeon* dungeon = nullptr;
		glm::ivec2 gridSelected = { 0, 0 };

		// File Operations
		std::string subfolder = "crawl/dungeons/";
		std::string extension = ".dungeon";
		std::string dungeonFileName = "new_dungeon";
		std::string dungeonFileNameSaveAs = "";
		std::string dungeonFilePath = "";

		bool saveAsPrompt = false;
		bool confirmOverwritePrompt = false;
		bool didSaveAs = false;

		// Brush Mode
		int brush_tileMask = 0;
		// Auto Tile configuration
		bool brush_AutoTileEnabled = true;
		bool brush_AutoTileSurround = true;

		// Brush Tile
		bool brush_TileNorth = true;
		bool brush_TileSouth = true;
		bool brush_TileEast = true;
		bool brush_TileWest = true;

	};
}

