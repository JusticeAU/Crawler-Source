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

bool Crawl::DungeonActivatorPlate::TestPosition(bool initialConfig)
{
	DungeonTile* tile = dungeon->GetTile(position);
	if (tile)
	{
		if (tile->occupied && !down)
		{
			down = true;
			if(!initialConfig) dungeon->DoActivate(activateID, true);
		}
		else if (!tile->occupied && down)
		{
			down = false;
			if (!initialConfig) dungeon->DoActivate(activateID);
		}
		UpdateTransforms();
	}

	return true;
}

void Crawl::DungeonActivatorPlate::UpdateTransforms()
{
	// No height change for now
	if (!down)
	{
		object->children[1]->SetLocalPosition({ 0,0, heightUnactivated });
	}
	else
	{
		object->children[1]->SetLocalPosition({ 0,0, heightActivated });
	}
}