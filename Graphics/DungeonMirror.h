#pragma once
#include "DungeonHelpers.h"
#include "serialisation.h"

using glm::ivec2;

class Object;

namespace Crawl
{
	class DungeonMirror
	{
	public:
		~DungeonMirror();
		ivec2 position = {0,0};
		FACING_INDEX facing = NORTH_INDEX;

		void UpdateTransform();

		int ShouldReflect(FACING_INDEX approachFrom);

		Object* object = nullptr;
	};

	static void to_json(ordered_json& j, const DungeonMirror& object)
	{
		j = { {"position", object.position }, {"facing", object.facing } };
	}

	static void from_json(const ordered_json& j, DungeonMirror& object)
	{
		j.at("position").get_to(object.position);
		j.at("facing").get_to(object.facing);
	}
}