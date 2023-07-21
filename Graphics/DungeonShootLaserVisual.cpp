#include "DungeonShootLaserVisual.h"
#include "Object.h"

Crawl::DungeonShootLaserVisual::~DungeonShootLaserVisual()
{
	if (object)
		object->markedForDeletion = true;
}
