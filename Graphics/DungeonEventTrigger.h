#pragma once
#include <glm.hpp>
#include "serialisation.h"
#include "Dungeon.h"
#include "DungeonPlayer.h"

namespace Crawl
{
	class DungeonEventTrigger
	{
	public:
		ivec2 position = {0,0};
		bool repeats = false;
		bool hasTriggered = false;
		int eventID = -1;
		bool mustBeFacing = false;
		FACING_INDEX facing = NORTH_INDEX;

		Dungeon* dungeon;

		void Activate()
		{
			if (repeats || !hasTriggered)
			{
				hasTriggered = true;
				dungeon->player->DoEvent(eventID);
			}
		}
	};

	static void to_json(ordered_json& j, const DungeonEventTrigger& object)
	{
		j = { {"position", object.position}, {"repeats", object.repeats}, {"eventID", object.eventID} };
		if (object.mustBeFacing)
		{
			j["mustBeFacing"] = object.mustBeFacing;
			j["facing"] = object.facing;

		}
	}

	static void from_json(const ordered_json& j, DungeonEventTrigger& object)
	{
		j.at("position").get_to(object.position);
		j.at("repeats").get_to(object.repeats);
		j.at("eventID").get_to(object.eventID);
		if (j.contains("mustBeFacing"))
		{
			j.at("mustBeFacing").get_to(object.mustBeFacing);
			j.at("facing").get_to(object.facing);
		}
	}
}