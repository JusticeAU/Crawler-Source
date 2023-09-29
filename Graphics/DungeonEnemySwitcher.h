#include "glm.hpp"
#include "DungeonHelpers.h"
#include "serialisation.h"

class Object;

namespace Crawl
{
	class Dungeon;

	class DungeonEnemySwitcher
	{
	public:
		~DungeonEnemySwitcher();
		glm::ivec2 position = { 0,0 };
		FACING_INDEX facing = NORTH_INDEX;

		void UpdateTransform();

		// dependencies
		Object* object = nullptr;
		Dungeon* dungeon = nullptr;

		void Update();

		void SwapWithPlayer();
	};

	static void to_json(ordered_json& j, const DungeonEnemySwitcher& object)
	{
		j = { {"position", object.position }, {"facing", object.facing } };
	}

	static void from_json(const ordered_json& j, DungeonEnemySwitcher& object)
	{
		j.at("position").get_to(object.position);
		j.at("facing").get_to(object.facing);
	}

}