#include "DungeonMirror.h"
#include "Object.h"

Crawl::DungeonMirror::~DungeonMirror()
{
	if (object)
		object->markedForDeletion = true;
}

int Crawl::DungeonMirror::ShouldReflect(FACING_INDEX approachFrom)
{
	switch (approachFrom)
	{
	case NORTH_INDEX:
	{
		if (facing == SOUTH_INDEX)
			return EAST_INDEX;
		else if (facing == WEST_INDEX)
			return  WEST_INDEX;
		else
			return -1;
		break;
	}
	case EAST_INDEX:
	{
		if (facing == WEST_INDEX)
			return SOUTH_INDEX;
		else if (facing == NORTH_INDEX)
			return NORTH_INDEX;
		else
			return -1;
		break;
	}
	case SOUTH_INDEX:
	{
		if (facing == NORTH_INDEX)
			return WEST_INDEX;
		else if (facing == EAST_INDEX)
			return EAST_INDEX;
		else
			return -1;
		break;
	}
	case WEST_INDEX:
	{
		if (facing == EAST_INDEX)
			return NORTH_INDEX;
		else if (facing == SOUTH_INDEX)
			return SOUTH_INDEX;
		else
			return -1;
		break;
	}
	}

	return -1; // Shouldn't get here
}