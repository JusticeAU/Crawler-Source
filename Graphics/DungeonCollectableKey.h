#pragma once
#include <glm.hpp>
#include "serialisation.h"
#include "DungeonHelpers.h"

class Object;

namespace Crawl
{
	class Dungeon;
	class DungeonCollectableKey
	{
	public:
		~DungeonCollectableKey();

		void Interact();
		void UpdateTransform();
		glm::ivec2 position = {0,0};
		FACING_INDEX facing = NORTH_INDEX;

		string collectSfx = "crawler/sound/load/collect_key.wav";

		Object* object = nullptr;
		Dungeon* dungeon = nullptr;

		bool collected = false;
		int lockReleaseID = 0;
		int doorActivateID = 0;
	};

	static void to_json(ordered_json& j, const DungeonCollectableKey& key)
	{
		j = { {"position", key.position}, {"facing", key.facing}, {"lockReleaseID", key.lockReleaseID}, {"doorActivateID", key.doorActivateID } };
	}

	static void from_json(const ordered_json& j, DungeonCollectableKey& key)
	{
		j.at("position").get_to(key.position);
		j.at("facing").get_to(key.facing);
		j.at("lockReleaseID").get_to(key.lockReleaseID);
		j.at("doorActivateID").get_to(key.doorActivateID);
	}
}

