#include "DungeonSpikes.h"
#include "Dungeon.h"
#include "Object.h"
#include "Scene.h"

Crawl::DungeonSpikes::~DungeonSpikes()
{
	if (object)
		object->markedForDeletion = true;

	// Delete Me when new asset goes in.
	if (placeHolderContainerObject)
		placeHolderContainerObject->markedForDeletion = true;
}

void Crawl::DungeonSpikes::Disable()
{
	disabled = true;
	dungeon->GetTile(position)->occupied = false;
}