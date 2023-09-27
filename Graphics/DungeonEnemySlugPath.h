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
		int maskTraverse = 0;

		DungeonEnemySlugPath* neighbors[4]; // N E S W
		Object* object;
		Dungeon* dungeon;

		static std::string autoTileObjects[16];
		static float autoTileOrientations[16];

		void AutoGenerateMask();
		void RefreshNeighbors();
		void RefreshObject();
		void RemoveConnectionsFromNeighbors();

		bool IsValidConfiguration();
	};

	static void to_json(ordered_json& j, const DungeonEnemySlugPath& object)
	{
		j = { {"position", object.position }, {"maskTraverse", object.maskTraverse} };
	}

	static void from_json(const ordered_json& j, DungeonEnemySlugPath& object)
	{
		j.at("position").get_to(object.position);
		if (j.contains("maskTraverse")) j.at("maskTraverse").get_to(object.maskTraverse);
		else object.maskTraverse = -1; // flag this is one that needs to be auto generated.
	}
}

