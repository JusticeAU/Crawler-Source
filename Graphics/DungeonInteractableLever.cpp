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
	UpdateTransform();

	//dungeon->DoActivate(activateID, status);
	dungeon->DoActivate(activateID);
}

void Crawl::DungeonInteractableLever::SetID(unsigned int newID)
{
	id = newID;
	object->children[0]->children[0]->id = newID;
}

void Crawl::DungeonInteractableLever::UpdateTransform()
{
	if (status)
	{
		if (object)
		{
			object->localPosition.z = wallHeightOn;
			object->SetDirtyTransform();
		}
	}
	else
	{
		if (object)
		{
			object->localPosition.z = wallHeightOff;
			object->SetDirtyTransform();
		}
	}
}

//void Crawl::DungeonInteractableLever::Prime()
//{
//	if (object)
//	{
//		object->localPosition.z = wallHeightOn;
//		object->dirtyTransform = true;
//	}
//	dungeon->DoActivate(activateID, status);
//}

const float Crawl::DungeonInteractableLever::wallHeightOn = 1.25f;
const float Crawl::DungeonInteractableLever::wallHeightOff = 1.50f;