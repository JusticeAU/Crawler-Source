#include "DungeonSpikes.h"
#include "Dungeon.h"
#include "Object.h"
#include "ComponentRenderer.h"
#include "MaterialManager.h"

Crawl::DungeonSpikes::~DungeonSpikes()
{
	if (object)
		object->markedForDeletion = true;
}

void Crawl::DungeonSpikes::Disable()
{
	dungeon->GetTile(position)->occupied = false;
	disabled = true;
	((ComponentRenderer*)object->GetComponent(Component_Renderer))->materialArray[0] = MaterialManager::GetMaterial("crawler/material/prototype/spikes_covered.material");
}