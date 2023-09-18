#pragma once
#include "glm.hpp"
#include "ext\matrix_transform.hpp"
#include <string>

using glm::mat4;

namespace Crawl
{
	const int DUNGEON_GRID_SCALE = 2;
	const mat4 CRAWLER_TRANSFORM = glm::scale(mat4(1), { 0.01, 0.01, 0.01 }) * glm::rotate(mat4(1), glm::radians(90.0f), { 1,0,0 });
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

	const glm::ivec2 NORTH_COORDINATE = { 0,1 };
	const glm::ivec2 EAST_COORDINATE = { 1,0 };
	const glm::ivec2 SOUTH_COORDINATE = { 0,-1 };
	const glm::ivec2 WEST_COORDINATE = { -1,0 };
	const glm::ivec2 NORTHEAST_COORDINATE = { 1,1 };
	const glm::ivec2 SOUTHEAST_COORDINATE = { 1,-1 };
	const glm::ivec2 SOUTHWEST_COORDINATE = { -1,-1 };
	const glm::ivec2 NORTHWEST_COORDINATE = { -1,1 };


	// With the below FACING_INDEX, DIRECTION_INDEX and directions[4], if you add a facing index with a direction index, wrap it, you can get a cartesian coordinate.
	enum FACING_INDEX
	{
		NORTH_INDEX,
		EAST_INDEX,
		SOUTH_INDEX,
		WEST_INDEX
	};

	inline FACING_INDEX facingIndexesReversed[4] =
	{
		SOUTH_INDEX,
		WEST_INDEX,
		NORTH_INDEX,
		EAST_INDEX,
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

	inline glm::ivec2 directionsReversed[4] = {
		SOUTH_COORDINATE,
		WEST_COORDINATE,
		NORTH_COORDINATE,
		EAST_COORDINATE
	};

	inline glm::ivec2 directionsDiagonal[4] = {
		NORTHEAST_COORDINATE,
		SOUTHEAST_COORDINATE,
		SOUTHWEST_COORDINATE,
		NORTHWEST_COORDINATE
	};

	inline glm::ivec2 tileToPillarCoordinates[4] = {
		{ 0, 0},
		{ 0,-1},
		{-1,-1},
		{ -1,0}
	};

	inline glm::ivec2 pillarToTileCoordinates[4] = {
		{ 1, 1},
		{ 1, 0},
		{ 0, 0},
		{ 0, 1}
	};

	enum PILLAR_INDEX
	{
		NORTHEAST_INDEX,
		SOUTHEAST_INDEX,
		SOUTHWEST_INDEX,
		NORTHWEST_INDEX
	};

	static std::string maskToString[16]
	{
		"None",
		"North",
		"West",
		"North, West",
		"East",
		"North, East",
		"East, West",
		"North, East, West",
		"South",
		"North, South",
		"South, West",
		"North, South, West",
		"East, South",
		"North, East, South",
		"East, South, West",
		"North, East, South, West"
	};


	static glm::vec3 dungeonPosToObjectScale(glm::ivec2 pos)
	{
		return glm::vec3(pos.x * DUNGEON_GRID_SCALE, pos.y * DUNGEON_GRID_SCALE, 0);
	}

	static FACING_INDEX dungeonRotateTowards(FACING_INDEX from, FACING_INDEX to)
	{
		int difference = to - from;
		if (difference == 3) difference -= 4;
		if (difference == -3) difference += 4;
		int clamped = glm::clamp(difference, -1, 1);
		int newDirection = (FACING_INDEX)(from + clamped);
		if (newDirection == -1) newDirection = 3;
		if (newDirection == 4) newDirection = 0;
		return (FACING_INDEX)newDirection;
	}

	static FACING_INDEX dungeonRotate(FACING_INDEX from, int direction)
	{
		int newIndex = (FACING_INDEX)(from + direction);
		if (newIndex < 0) newIndex += 4;
		if (newIndex > 3) newIndex -= 4;
		return (FACING_INDEX)newIndex;
	}

	// a 180 will be clockwise, a turn to the same direction will be considered invalid and return bogus data
	static bool IsClockWiseTurn(FACING_INDEX from, FACING_INDEX to)
	{
		int difference = to - from;
		if (difference == 3) difference -= 4;
		if (difference == -3) difference += 4;
		int sign = glm::sign(difference);
		return sign == 1;
	}

	static int GetInvertedMask(int mask)
	{
		int flipped = ~mask;
		return flipped & 0b1111;
	}
}