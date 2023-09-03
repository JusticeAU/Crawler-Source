#include "DungeonStairs.h"
#include "gtx/spline.hpp"

glm::vec3 Crawl::DungeonStairs::EvaluatePosition(float t)
{
	return glm::hermite(startWorldPosition, startOffset, endWorldPosition, endOffset, t);
}

void Crawl::DungeonStairs::BuildDefaultSpline()
{
	// Take the start and finish grid positions and build a basic untweaked spline based off of this. maybe it will end op being fine?
	startOffset = dungeonPosToObjectScale(directions[directionStart]);
	startWorldPosition = dungeonPosToObjectScale(startPosition) - startOffset;
	if (!up)
	{
		startOffset.z = 3.0f;
		startWorldPosition.z = 3.0f;
	}

	endOffset = dungeonPosToObjectScale(directionsReversed[directionEnd]);
	endWorldPosition = dungeonPosToObjectScale(endPosition) - endOffset;
	if (up)
	{
		endOffset.z = 3.0f;
		endWorldPosition.z = 3.0f;
	}
}
