#pragma once
#include "glm.hpp"

namespace Crawl
{
	class DungeonInteractable
	{
	public:
		virtual void Toggle() = 0;
		
		glm::ivec2 position = { 0, 0 };
		int orientation = 0; // Use DIRECTION_MASK in DungeonHelpers.h
		unsigned int id = 0;
	};
}