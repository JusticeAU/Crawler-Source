#pragma once
#include "glm.hpp"
class Object;

using glm::ivec2;

namespace Crawl
{
	class DungeonDamageVisual
	{
	public:
		~DungeonDamageVisual();
		ivec2 position = { 0,0 };
		Object* object = nullptr;
	};
}