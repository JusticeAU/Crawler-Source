#pragma once
#include "glm.hpp"
#include "serialisation.h"

using glm::ivec2;
class Object;

namespace Crawl
{
	class Dungeon;

	class DungeonSpikes
	{
	public:
		~DungeonSpikes();
		ivec2 position = {0,0};
		void UpdateTransform();

		bool disabled = false; // We dont need to serialise this as it gets handled at runtime by boxes being placed in the world after.

		void Disable();
		
		Dungeon* dungeon = nullptr;
		Object* object = nullptr;

	private:
	};

	static void to_json(ordered_json& j, const DungeonSpikes& object)
	{
		j = { {"position", object.position } };
	}

	static void from_json(const ordered_json& j, DungeonSpikes& object)
	{
		j.at("position").get_to(object.position);
	}
}