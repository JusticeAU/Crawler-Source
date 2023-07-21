#include "DungeonPushableBlock.h"
#include "Object.h"

Crawl::DungeonPushableBlock::~DungeonPushableBlock()
{
	if (object)
		object->markedForDeletion = true;
}