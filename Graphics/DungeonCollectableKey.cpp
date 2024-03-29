#include "DungeonCollectableKey.h"

#include "Object.h"
#include "Dungeon.h"
#include "DungeonGameManager.h"
#include "DungeonPlayer.h"
#include "AudioManager.h"

Crawl::DungeonCollectableKey::~DungeonCollectableKey()
{
	if (object != nullptr)
	{
		object->markedForDeletion = true;
	}
}

void Crawl::DungeonCollectableKey::Interact()
{
	if (collected) return;

	collected = true;

	// remove key
	object->children[0]->children[1]->markedForDeletion = true;
	object->children[0]->children[2]->markedForDeletion = true;

	// play key animation on player
	// TO DO

	// open door
	dungeon->DoActivate(doorActivateID);

	// mark key as collected in game manager
	DungeonGameManager::Get()->RemoveFrontDoorLock(lockReleaseID-1); // IDs are 1 indexed on collectable keys.
	dungeon->player->StartTakeKeyAnimation();
	// checkpoint??

	// SFX
	AudioManager::PlaySound(collectSfx);
}

void Crawl::DungeonCollectableKey::UpdateTransform()
{
	object->SetLocalPosition(dungeonPosToObjectScale(position));
	object->SetLocalRotationZ(orientationEulers[facing]);
}
