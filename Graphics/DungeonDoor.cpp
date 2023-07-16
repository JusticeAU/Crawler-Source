#include "DungeonDoor.h"
#include "Object.h"

Crawl::DungeonDoor::~DungeonDoor()
{
	if(object)
		object->markedForDeletion = true;
}

void Crawl::DungeonDoor::Toggle()
{
	open = !open;
	if (open)
	{
		if (object)
		{	
			object->localPosition.z = 5.0f;
			object->dirtyTransform = true;
		}
	}
	else
	{
		if (object)
		{
			object->localPosition.z = 0.0f;
			object->dirtyTransform = true;
		}
	}
}
