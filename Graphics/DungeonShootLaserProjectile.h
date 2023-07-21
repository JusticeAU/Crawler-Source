#pragma once
#include "glm.hpp"
#include "DungeonHelpers.h"

class Object;

using glm::ivec2;

namespace Crawl
{
	class Dungeon;
	class DungeonShootLaserProjectile
	{
	public:
		~DungeonShootLaserProjectile();
		ivec2 position = {0,0};
		FACING_INDEX facing = NORTH_INDEX;
		unsigned int turnCreated = 0;

		bool shouldDestrySelf = false;

		Object* object = nullptr;
		Dungeon* dungeon = nullptr;
		void Update();
	};
}

