#include "DungeonActivatorPlate.h"
#include "Dungeon.h"
#include "Object.h"
#include "imgui.h"
#include <iostream>

Crawl::DungeonActivatorPlate::DungeonActivatorPlate()
{
	
}

Crawl::DungeonActivatorPlate::~DungeonActivatorPlate()
{
	if (object)
		object->markedForDeletion = true;
}

bool Crawl::DungeonActivatorPlate::TestPosition()
{
	DungeonTile* tile = dungeon->GetTile(position);
	if (tile)
	{
		if (tile->occupied && !down)
		{
			down = true;
			dungeon->DoActivate(activateID, true);
		}
		else if (!tile->occupied && down)
		{
			down = false;
			dungeon->DoActivate(activateID);
		}
		UpdateTransforms();
	}

	return true;
}

void Crawl::DungeonActivatorPlate::UpdateTransforms()
{
	// No height change for now
	/*if (!down)
	{
		object->localPosition.z = heightUnactivated;
		object->dirtyTransform = true;
	}
	else
	{
		object->localPosition.z = heightActivated;
		object->dirtyTransform = true;
	}*/
}