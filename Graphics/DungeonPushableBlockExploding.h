#pragma once
#include "glm.hpp"
#include "DungeonHelpers.h"
#include <string>

class Object;

namespace Crawl
{
	class DungeonPushableBlockExploding
	{
	public:
		DungeonPushableBlockExploding(glm::vec3 position, FACING_INDEX direction);
		~DungeonPushableBlockExploding();
	private:
		Object* object;
		//std::string modelPath = "crawler/model/monster_chaser.";
		std::string modelPath = "crawler/model/interactable_crate_break_entrance.";

		//std::string animationName = modelPath + "someanimation";
		std::string animationName = modelPath + "fbxscene";
	};
}

