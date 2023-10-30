#pragma once
#include "glm.hpp"
#include "serialisation.h"

class Object;

namespace Crawl
{
	class Dungeon;

	class DungeonActivatorPlate
	{
	public:
		DungeonActivatorPlate();
		~DungeonActivatorPlate();

		bool TestPosition(bool initialConfig = false);
		void UpdateTransforms();

		glm::ivec2 position;
		unsigned int activateID = 1;

		float heightUnactivated = 0.0f;
		float heightActivated = -0.030f;
		
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