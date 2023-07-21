#pragma once
#include <glm.hpp>
#include <string>

namespace Crawl
{
	// Useful for auto tiling. If you reverse the order of the bits you'll get the reverse direction.
	enum DIRECTION_MASK
	{
		NORTH_MASK = 1,
		EAST_MASK = 4,
		SOUTH_MASK = 8,
		WEST_MASK = 2,
	};

	static unsigned int orientationMasksIndex[4] =
	{
		NORTH_MASK,
		EAST_MASK,
		SOUTH_MASK,
		WEST_MASK,
	};

	static std::string orientationNames[4] = {
		"North",
		"East",
		"South",
		"West"
	};

	static float orientationEulers[4] = {
		0.0f,
		-90.0f,
		180.0f,
		90.0f
	};

	static float orientationEulersReversed[4] = {
		180.0f,
		90.0f,
		0.0f,
		-90.0f
	};

	const glm::ivec2 NORTH_COORDINATE	= { 0,1 };
	const glm::ivec2 SOUTH_COORDINATE	= { 0,-1};
	const glm::ivec2 EAST_COORDINATE	= { 1,0 };
	const glm::ivec2 WEST_COORDINATE	= {-1,0 };

	// With the below FACING_INDEX, DIRECTION_INDEX and directions[4], if you add a facing index with a direction index, wrap it, you can get a cartesian coordinate.
	enum FACING_INDEX
	{
		NORTH_INDEX,
		EAST_INDEX,
		SOUTH_INDEX,
		WEST_INDEX
	};

	enum DIRECTION_INDEX
	{
		FORWARD_INDEX,
		RIGHT_INDEX,
		BACK_INDEX,
		LEFT_INDEX
	};

	// this is indexed in to by GetMoveCardinalIndex;
	inline glm::ivec2 directions[4] = {
		NORTH_COORDINATE,
		EAST_COORDINATE,
		SOUTH_COORDINATE,
		WEST_COORDINATE
	};
}