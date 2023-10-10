#include "DungeonSpikes.h"
#include "Dungeon.h"
#include "Object.h"
#include "Scene.h"

Crawl::DungeonSpikes::~DungeonSpikes()
{
	if (object)
		object->markedForDeletion = true;
}

void Crawl::DungeonSpikes::UpdateTransform()
{
	object->SetLocalPosition({ position.x * DUNGEON_GRID_SCALE, position.y * DUNGEON_GRID_SCALE, 0 });
}

void Crawl::DungeonSpikes::Disable()
{
	disabled = true;
	dungeon->GetTile(position)->occupied = false;
}