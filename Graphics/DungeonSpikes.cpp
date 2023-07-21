#include "DungeonSpikes.h"
#include "Object.h"

Crawl::DungeonSpikes::~DungeonSpikes()
{
	if (object)
		object->markedForDeletion = true;
}