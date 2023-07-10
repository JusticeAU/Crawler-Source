#pragma once

#include "DungeonTile.h"
#include <string>
#include <map>
#include <vector>

namespace Crawl
{
	const int DUNGEON_GRID_SCALE = 5;

	struct Column
	{
		std::map<int, DungeonTile> row;
	};
	struct Position
	{
		int column = 0;
		int row = 0;
	};
	class Dungeon
	{
	public:
		Dungeon();
		// Room/Hall manipulation
		DungeonTile* AddHall(int x, int y);
		void AddHall(DungeonTile& hall);
		bool SetHallMask(int x, int y, int mask);
		DungeonTile* GetHall(int x, int y);
		// Deletes a hall from the grid. returns true if a deletion happened.
		bool DeleteHall(int x, int y);

		// Ensures the hall node has a visual representation in the Scene Graph.
		void CreateTileObject(DungeonTile* hall);


		bool IsOpenHall(int x, int y);
		bool CanMove(int xFrom, int yFrom, int xTo, int yTo);
	
		void Save(std::string filename);
		void Load(std::string filename);
	
		// Calculates the tile mask based on adjacent tiles
		int GetAutoTileMask(int x, int y);
		// returns pointer to the template tile for the Scene to duplicate.
		Object* GetTileTemplate(int mask);
		void SetParentTileObject(Object* object);

	protected:
		void InitialiseTileMap();

		// Before loading a dungeon, this will mark every scene object for deletion before clearing off the hallways.
		void DestroySceneFromDungeonLayout();
		// After loading a dungeon, this will build it in the Scene graph based on tile adjacency. used on editor and playmode dungeon loading.
		void BuildSceneFromDungeonLayout();

		const int version = 1; // increment this when the .dungeon file schema changes and ensure backwards compatibility.
		std::map<int, Column> halls;
		Object* tile_template;
		Object* tilesParentObject = nullptr;

		std::vector<string> wallVariantPaths;
	};
}

