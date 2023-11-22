#pragma once
#include "glm.hpp"
#include <string>

class Object;

namespace Crawl
{
	class DungeonPushableBlockExploding
	{
	public:
		DungeonPushableBlockExploding(glm::vec3 position);
		~DungeonPushableBlockExploding();
	private:
		Object* object;
		//std::string modelPath = "crawler/model/interactable_crate.object";
		std::string modelPath = "crawler/model/monster_chaser.";

		//std::string animationName = modelPath + "someanimation";
		std::string animationName = modelPath + "fbxchaser.rig|chaser_activate";
	};
}

