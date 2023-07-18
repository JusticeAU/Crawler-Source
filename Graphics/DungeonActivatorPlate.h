#pragma once
#include "Dungeon.h"
#include "glm.hpp"

class Object;

namespace Crawl
{
	class DungeonActivatorPlate
	{
	public:
		DungeonActivatorPlate();
		~DungeonActivatorPlate();

		bool TestPosition();
		void UpdateTransforms();

		glm::ivec2 position;
		unsigned int activateID = 1;
		
		bool down = false;
		
		Dungeon* dungeon = nullptr;
		Object* object = nullptr;
	};

	static void to_json(ordered_json& j, const DungeonActivatorPlate& plate)
	{
		j = { {"position", plate.position }, {"activateID", plate.activateID } };
	}

	static void from_json(const ordered_json& j, DungeonActivatorPlate& plate)
	{
		j.at("position").get_to(plate.position);
		j.at("activateID").get_to(plate.activateID);
	}
}