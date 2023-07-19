#include "DungeonTransporter.h"
#include "Object.h"

Crawl::DungeonTransporter::~DungeonTransporter()
{
	if (object)
		object->markedForDeletion = true;
}
