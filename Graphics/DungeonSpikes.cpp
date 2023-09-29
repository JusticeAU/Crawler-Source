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

void Crawl::DungeonSpikes::UpdateTransform()
{
	object->SetLocalPosition({ position.x * DUNGEON_GRID_SCALE, position.y * DUNGEON_GRID_SCALE, -1 }); // until new asset is in
	object->SetLocalScale({ 2, 1.25, 2 }); // until new asset is in
}

void Crawl::DungeonSpikes::Disable()
{
	disabled = true;
	dungeon->GetTile(position)->occupied = false;
}