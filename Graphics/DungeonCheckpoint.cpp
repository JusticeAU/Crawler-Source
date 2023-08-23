#include "DungeonCheckpoint.h"
#include "DungeonPlayer.h"
#include "Object.h"
#include "ComponentRenderer.h"
#include "MaterialManager.h"
#include "AudioManager.h"

Crawl::DungeonCheckpoint::~DungeonCheckpoint()
{
	if (object)
		object->markedForDeletion = true;
}

void Crawl::DungeonCheckpoint::SetCheckpoint(DungeonPlayer* player)
{
	if (activateSound != "")
		AudioManager::PlaySound(activateSound);

	player->MakeCheckpoint();
	activated = true;
	SetActivatedMaterial();
}

void Crawl::DungeonCheckpoint::SetActivatedMaterial()
{
	((ComponentRenderer*)object->GetComponent(Component_Renderer))->materialArray[0] = MaterialManager::GetMaterial("crawler/material/prototype/checkpoint_checked.material");
}