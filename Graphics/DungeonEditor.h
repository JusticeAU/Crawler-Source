#pragma once
#include "Dungeon.h"
#include "glm.hpp"

namespace Crawl
{
	class DungeonEditor
	{
	public:
		DungeonEditor();

		void DrawGUI();
		void Update();
	protected:
		void UpdateMousePosOnGrid();
		void UpdateSurroundingTiles(int column, int row);

		// Save the dungeon to file.
		void Save();

		std::string GetDungeonFilePath();

		Dungeon* dungeon = nullptr;
		glm::ivec2 gridSelected = { 0, 0 };

		std::string subfolder = "crawl/dungeons/";
		std::string extension = ".dungeon";
		std::string dungeonFileName = "new_dungeon";
		std::string dungeonFileNameSaveAs = "";
		std::string dungeonFilePath = "";

		// GUI helpers
		bool saveAsPrompt = false;
		bool confirmOverwritePrompt = false;
		bool didSaveAs = false;
	};
}

