#pragma once
#include <glm.hpp>
#include "serialisation.h"

class Object;

namespace Crawl
{
	class DungeonDoor
	{
	public:
		void Toggle();
		glm::ivec2 position = { 0, 0 };
		int orientation = 0; // Use DIRECTION_MASK in DungeonHelpers.h
		unsigned int id = 0;
		
		bool open = false;
		bool startOpen = false;
		
		Object* object = nullptr;
	};

	static void to_json(ordered_json& j, const DungeonDoor& door)
	{
		j = { {"position", door.position}, {"id", door.id}, {"orientation", door.orientation}, {"open", door.startOpen} };
	}

	static void from_json(const ordered_json& j, DungeonDoor& door)
	{
		j.at("position").get_to(door.position);
		j.at("id").get_to(door.id);
		j.at("orientation").get_to(door.orientation);
		j.at("open").get_to(door.startOpen);
	}
}