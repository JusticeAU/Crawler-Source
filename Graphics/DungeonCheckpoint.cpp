#include "DungeonCheckpoint.h"
#include "DungeonPlayer.h"
#include "AudioManager.h"

void Crawl::DungeonCheckpoint::SetCheckpoint(DungeonPlayer* player)
{
	if (activateSound != "")
		AudioManager::PlaySound(activateSound);

	activated = true;
	player->MakeCheckpoint(facing);
}