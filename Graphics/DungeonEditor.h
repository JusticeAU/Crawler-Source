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
		
		void BuildSceneFromDungeonLayout();

		Dungeon* dungeon = nullptr;

		const int GRID_SCALE = 5;
		glm::ivec2 gridSelected;

		std::string dungeonFileName = "crawl/dungeons/test.dungeon";
		std::string dungeonFileNameSaveAs = "crawl/dungeons/test.dungeon";

		bool saveAsPrompt = false;
		bool confirmOverwritePrompt = false;
		bool didSaveAs = false;
	};
}

