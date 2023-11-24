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
		DungeonPushableBlockExploding(glm::vec3 position);
		DungeonPushableBlockExploding(glm::vec3 position, FACING_INDEX direction);
		~DungeonPushableBlockExploding();
	private:
		Object* object;
		std::string modelPathMurderina = "crawler/model/interactable_crate_break_entrance.";
		std::string animationNameMurderina = modelPathMurderina + "fbxscene";

		std::string modelPathBlocker = "crawler/model/interactable_crate_break_blocker.";
		std::string animationNameBlocker = modelPathBlocker + "fbxscene";
	};
}

