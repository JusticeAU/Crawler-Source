#pragma once
#include <glm.hpp>
#include "serialisation.h"
#include "DungeonHelpers.h"


namespace Crawl
{
	class Dungeon;

	class DungeonLightToggler
	{
	public:
		glm::ivec2 position;
		FACING_INDEX facing = NORTH_INDEX;

		bool onlyOnce = false;
		bool twoWay = false;

		bool triggered = false;

		int enableID = -1;
		int disableID = -1;

		Dungeon* dungeon = nullptr;

		void Trigger();
		void ReverseTrigger();
	};


	static void to_json(ordered_json& j, const DungeonLightToggler& object)
	{
		j = { {"position", object.position}, {"facing", object.facing}, {"enableID", object.enableID}, {"disableID", object.disableID} };
		if (object.onlyOnce) j["onlyOnce"] = true;
		if (object.twoWay) j["twoWay"] = true;
		if (object.triggered) j["triggered"] = true;
	}

	static void from_json(const ordered_json& j, DungeonLightToggler& object)
	{
		j.at("position").get_to(object.position);
		j.at("facing").get_to(object.facing);
		j.at("enableID").get_to(object.enableID);
		j.at("disableID").get_to(object.disableID);
		if (j.contains("onlyOnce")) j.at("onlyOnce").get_to(object.onlyOnce);
		if (j.contains("twoWay")) j.at("twoWay").get_to(object.twoWay);
		if (j.contains("triggered")) j.at("triggered").get_to(object.triggered);

	}
}