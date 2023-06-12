#pragma once
#include <string>
#include <map>

class Object;

namespace Crawl
{
	const int DUNGEON_GRID_SCALE = 5;

	struct Hall
	{
		int column, row;
		bool occupied = false;
		Object* object = nullptr;
	};
	struct Column
	{
		std::map<int, Hall> row;
	};
	struct Position
	{
		int column = 0;
		int row = 0;
	};
	enum DIRECTION
	{
		NORTH,
		EAST,
		SOUTH,
		WEST
	};
	class Dungeon
	{
	public:
		Dungeon();
		// Room/Hall manipulation
		Hall* AddHall(int column, int row);
		Hall* GetHall(int column, int row);
		// Deletes a hall from the grid. returns true if a deletion happened.
		bool DeleteHall(int column, int row);

		// Ensures the hall node has a visual representation in the Scene Graph.
		void CreateTileObject(Hall* hall);

		bool IsOpenHall(int column, int row);
		bool CanMove(int fromColumn, int fromRow, int toColumn, int toRow);
	
		void Save(std::string filename);
		void Load(std::string filename);
	
	protected:
		void InitialiseTileMap();

		// Before loading a dungeon, this will mark every scene object for deletion before clearing off the hallways.
		void DestroySceneFromDungeonLayout();
		// After loading a dungeon, this will build it in the Scene graph based on tile adjacency. used on editor and playmode dungeon loading.
		void BuildSceneFromDungeonLayout();
		// Calculates the tile mask based on adjacent tiles
		int GetTileMask(int col, int row);
		// returns pointer to the template tile for the Scene to duplicate.
		Object* GetTileTemplate(int mask);


		const int version = 1; // increment this when the .dungeon file schema changes and ensure backwards compatibility.
		std::map<int, Column> halls;
		Position spawn;
		Object* tilemap[16];
	};
}

