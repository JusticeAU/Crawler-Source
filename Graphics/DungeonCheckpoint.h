#pragma once
#include "DungeonHelpers.h"
#include "serialisation.h"

using glm::ivec2;

class Object;

namespace Crawl
{
	class Dungeon;
	class DungeonPlayer;

	class DungeonCheckpoint
	{
	public:
		~DungeonCheckpoint();
		ivec2 position = {0,0};
		FACING_INDEX facing = NORTH_INDEX;

		bool IsActivated() { return activated; }
		void SetCheckpoint(DungeonPlayer* player);
		void SetActivatedMaterial();

		string activateSound = "";
		bool activated = false;

		// Dependencies
		Object* object = nullptr;
		Dungeon* dungeon = nullptr;
	};

	static void to_json(ordered_json& j, const DungeonCheckpoint& object)
	{
		j = { {"position", object.position }, {"facing", object.facing} };
		if (object.activated)
			j["activated"] = object.activated;
	}

	static void from_json(const ordered_json& j, DungeonCheckpoint& object)
	{
		j.at("position").get_to(object.position);
		j.at("facing").get_to(object.facing);
		if (j.contains("activated"))
			j.at("activated").get_to(object.activated);
	}
}