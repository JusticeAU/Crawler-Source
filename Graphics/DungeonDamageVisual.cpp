#include "DungeonDamageVisual.h"
#include "Object.h"

Crawl::DungeonDamageVisual::~DungeonDamageVisual()
{
	if (object)
		object->markedForDeletion = true;
}
