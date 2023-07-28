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
	UpdateTransforms();
}

void Crawl::DungeonDoor::Toggle(bool on)
{
	//open = on;
	//UpdateTransforms();
	if (on)
		power += 1;
	else
		power -= 1;

	Update();
}

void Crawl::DungeonDoor::Update()
{
	bool oldState = open;
	if (power > 0 && !open)
		open = true;
	if (power == 0 && open)
		open = false;

	if(oldState != open)
		UpdateTransforms();
}

void Crawl::DungeonDoor::UpdateTransforms()
{
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
