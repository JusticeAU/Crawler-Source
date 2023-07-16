#include "DungeonInteractableLever.h"
#include "Dungeon.h"
#include "Object.h"

Crawl::DungeonInteractableLever::~DungeonInteractableLever()
{
	if (object)
		object->markedForDeletion = true;
}

void Crawl::DungeonInteractableLever::Toggle()
{
	status = !status;
	if (status)
	{
		if (object)
		{
			object->localPosition.z = 2.0f;
			object->dirtyTransform = true;
		}
	}
	else
	{
		if (object)
		{
			object->localPosition.z = 2.5f;
			object->dirtyTransform = true;
		}
	}

	// hardcode doors
	dungeon->DoDoor(doorID);
}

void Crawl::DungeonInteractableLever::SetID(unsigned int newID)
{
	id = newID;
	object->children[0]->children[0]->id = newID;
}
