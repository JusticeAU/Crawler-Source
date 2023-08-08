#pragma once
#include "serialisation.h"

class Object;

namespace Crawl
{
	class DungeonTile
	{
	public:
		glm::ivec2 position = { 0, 0 };
		int mask = 0;

		// State
		bool occupied = false;

		// Dependencies
		Object* object = nullptr;

		// Pathfinding
		DungeonTile* neighbors[4] = { nullptr, nullptr, nullptr, nullptr }; // address in to here with FACING_INDEX

		// garbage
		int wallVariants[4] = { 0, 0, 0 ,0 }; // North, South, East, West

		// pathfind dev
		int cost = -1;
		bool openListed = false;
		bool closedListed = false;
		int enterDirection = -1;
		DungeonTile* toDestination = nullptr;
		DungeonTile* fromDestination = nullptr;
	};

	static void to_json(ordered_json& j, const DungeonTile& tile)
	{
		j = { {"position", tile.position}, {"mask", tile.mask}, {"wallVariants", tile.wallVariants} };
	}

	static void from_json(const ordered_json& j, DungeonTile& tile)
	{
		j.at("position").get_to(tile.position);
		j.at("mask").get_to(tile.mask);
		if(j.contains("wallVariants"))
			j.at("wallVariants").get_to(tile.wallVariants);
		else
		{
			if ((tile.mask & 1) != 1) // North Wall
				tile.wallVariants[0] = 1;
			if ((tile.mask & 8) != 8) // South Wall
				tile.wallVariants[1] = 1;
			if ((tile.mask & 4) != 4) // East Wall
				tile.wallVariants[2] = 1;
			if ((tile.mask & 2) != 2) // West Wall
				tile.wallVariants[3] = 1;
		}
	}
}

