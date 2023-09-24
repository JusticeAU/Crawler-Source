#include "DungeonCheckpoint.h"
#include "DungeonPlayer.h"
#include "AudioManager.h"

void Crawl::DungeonCheckpoint::SetCheckpoint(DungeonPlayer* player)
{
	if (activateSound != "")
		AudioManager::PlaySound(activateSound);

	player->MakeCheckpoint(facing);
	activated = true;
}