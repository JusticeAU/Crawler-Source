#pragma once
#include "glm.hpp"
#include "serialisation.h"

using glm::ivec2;
class Object;

namespace Crawl
{
	class DungeonPushableBlock
	{
	public:
		~DungeonPushableBlock();
		ivec2 position;
		Object* object;
	private:
	};

	static void to_json(ordered_json& j, const DungeonPushableBlock& object)
	{
		j = { {"position", object.position } };
	}

	static void from_json(const ordered_json& j, DungeonPushableBlock& object)
	{
		j.at("position").get_to(object.position);
	}
}