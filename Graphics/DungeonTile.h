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

		int wallVariants[4] = { 0, 0, 0 ,0 }; // North, South, East, West

		Object* object = nullptr;
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

