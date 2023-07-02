#pragma once
#include "serialisation.h"

class Object;

namespace Crawl
{
	class DungeonTile
	{
	public:
		glm::ivec2 position;
		int mask = 0;
		bool occupied = false;
		Object* object = nullptr;
	};

	static void to_json(ordered_json& j, const DungeonTile& tile)
	{
		j = { {"position", tile.position}, {"mask", tile.mask} };
	}

	static void from_json(const ordered_json& j, DungeonTile& tile)
	{
		j.at("position").get_to(tile.position);
		j.at("mask").get_to(tile.mask);
	}
}

