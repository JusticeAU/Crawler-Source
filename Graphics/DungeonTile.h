#pragma once
#include "serialisation.h"
#include "DungeonHelpers.h"

class Object;

namespace Crawl
{
	class DungeonTile
	{
	public:
		glm::ivec2 position = { 0, 0 };
		int maskTraverse = 0;
		int maskSee = 0;
		int wallVariants[4] = { -1, -1, -1 ,-1 }; // North, East, South, West. -1 is no wall, any other value is to index in to the dungeon wallVariantPaths array. visual only.

		// State
		bool occupied = false;

		// Dependencies
		Object* object = nullptr;

		// Pathfinding
		DungeonTile* neighbors[4] = { nullptr, nullptr, nullptr, nullptr }; // address in to here with FACING_INDEX
		int cost = -1;
		bool openListed = false;
		bool closedListed = false;
		int enterDirection = -1;
		DungeonTile* toDestination = nullptr;
		DungeonTile* fromDestination = nullptr;
	};

	static void to_json(ordered_json& j, const DungeonTile& tile)
	{
		j = { {"position", tile.position}, {"mask", tile.maskTraverse}, {"maskSee", tile.maskSee}, {"wallVariants", tile.wallVariants} };
	}

	static void from_json(const ordered_json& j, DungeonTile& tile)
	{
		j.at("position").get_to(tile.position);
		j.at("mask").get_to(tile.maskTraverse);
		
		if(j.contains("maskSee"))
			j.at("maskSee").get_to(tile.maskSee);
		else
		{
			if ((tile.maskTraverse & NORTH_MASK) == NORTH_MASK) // North Wall
				tile.maskSee += NORTH_MASK;
			if ((tile.maskTraverse & SOUTH_MASK) == SOUTH_MASK) // South Wall
				tile.maskSee += SOUTH_MASK;
			if ((tile.maskTraverse & EAST_MASK) == EAST_MASK) // East Wall
				tile.maskSee += EAST_MASK;
			if ((tile.maskTraverse & WEST_MASK) == WEST_MASK) // West Wall
				tile.maskSee += WEST_MASK;
		}

		if(j.contains("wallVariants"))
			j.at("wallVariants").get_to(tile.wallVariants);
		//else
		//{
		//	if ((tile.maskTraverse & NORTH_MASK) != NORTH_MASK) // North Wall
		//		tile.wallVariants[0] = 1;
		//	if ((tile.maskTraverse & SOUTH_MASK) != SOUTH_MASK) // South Wall
		//		tile.wallVariants[1] = 1;
		//	if ((tile.maskTraverse & EAST_MASK) != EAST_MASK) // East Wall
		//		tile.wallVariants[2] = 1;
		//	if ((tile.maskTraverse & WEST_MASK) != WEST_MASK) // West Wall
		//		tile.wallVariants[3] = 1;
		//}
	}
}

