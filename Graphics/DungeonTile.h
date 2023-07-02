#pragma once
#include "serialisation.h"

class Object;

namespace Crawl
{
	class DungeonTile
	{
	public:
		int xPos, yPos;
		int mask = 0;
		bool occupied = false;
		Object* object = nullptr;
	};

	static void to_json(ordered_json& j, const DungeonTile& tile)
	{
		j = { {"x", tile.xPos}, {"y", tile.yPos}, {"mask", tile.mask} };
	}

	static void from_json(const ordered_json& j, DungeonTile& tile)
	{
		j.at("x").get_to(tile.xPos);
		j.at("y").get_to(tile.yPos);
		j.at("mask").get_to(tile.mask);
	}
}

