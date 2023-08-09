#include "DungeonSpikes.h"
#include "Dungeon.h"
#include "Object.h"
#include "Scene.h"

Crawl::DungeonSpikes::~DungeonSpikes()
{
	if (object)
		object->markedForDeletion = true;
}

void Crawl::DungeonSpikes::Disable()
{
	dungeon->GetTile(position)->occupied = false;
	disabled = true;
	object->markedForDeletion = true;
	object = Scene::CreateObject();
	object->LoadFromJSON(ReadJSONFromDisk("crawler/model/interactable_trap_spike_broken.object"));
	object->SetLocalPosition(dungeonPosToObjectScale(position));
}