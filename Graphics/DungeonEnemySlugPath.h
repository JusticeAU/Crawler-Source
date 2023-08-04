#pragma once
#include "DungeonHelpers.h"
#include "serialisation.h"

class Object;

namespace Crawl
{
	class Dungeon;

	class DungeonEnemySlugPath
	{
	public:
		~DungeonEnemySlugPath();
		glm::ivec2 position;
		DungeonEnemySlugPath* neighbors[4]; // N E S W

		Object* object;
		Dungeon* dungeon;

		static std::string autoTileObjects[16];
		static float autoTileOrientations[16];

		void RefreshNeighbors();
		void RefreshObject();
	};

	static void to_json(ordered_json& j, const DungeonEnemySlugPath& object)
	{
		j = { {"position", object.position } };
	}

	static void from_json(const ordered_json& j, DungeonEnemySlugPath& object)
	{
		j.at("position").get_to(object.position);
	}
}

