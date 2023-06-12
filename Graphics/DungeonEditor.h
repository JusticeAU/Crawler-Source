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
		void UpdateMousePosOnGrid();
		void UpdateSurroundingTiles(int column, int row);

		Dungeon* dungeon = nullptr;
		glm::ivec2 gridSelected = { 0, 0 };

		std::string dungeonFileName = "crawl/dungeons/test.dungeon";
		std::string dungeonFileNameSaveAs = "crawl/dungeons/test.dungeon";

		// GUI helpers
		bool saveAsPrompt = false;
		bool confirmOverwritePrompt = false;
		bool didSaveAs = false;
	};
}

