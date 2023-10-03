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
		enum class Type
		{
			GlobalEvent,
			LightFlicker,
			FTUEPrompt,
			Count
		};
		ivec2 position = {0,0};
		FACING_INDEX facing = NORTH_INDEX;

		Type type = Type::GlobalEvent;
		int eventID = -1;
		bool repeats = false;
		bool hasTriggered = false;
		bool mustBeFacing = false;

		Dungeon* dungeon;

		void Activate()
		{
			if (repeats || !hasTriggered)
			{
				hasTriggered = true;
				switch (type)
				{
				case Type::GlobalEvent:
				{
					dungeon->player->DoEvent(eventID);
					break;
				}
				case Type::LightFlicker:
				{
					dungeon->FlickerLights(eventID);
					break;
				}
				}
			}
		}
	};

	static void to_json(ordered_json& j, const DungeonEventTrigger& object)
	{
		j = { {"position", object.position}, {"repeats", object.repeats}, {"type", object.type}, {"eventID", object.eventID} };
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
		if (j.contains("type")) j.at("type").get_to(object.type);
		j.at("eventID").get_to(object.eventID);
		if (j.contains("mustBeFacing"))
		{
			j.at("mustBeFacing").get_to(object.mustBeFacing);
			j.at("facing").get_to(object.facing);
		}
	}
}