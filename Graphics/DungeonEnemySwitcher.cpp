#include "DungeonEnemySwitcher.h"
#include "DungeonHelpers.h"
#include "DungeonTile.h"
#include "Dungeon.h"
#include "DungeonPlayer.h"
#include "Object.h"
#include "LogUtils.h"

Crawl::DungeonEnemySwitcher::~DungeonEnemySwitcher()
{
	if (object)
		object->markedForDeletion = true;
}

void Crawl::DungeonEnemySwitcher::Update()
{
	// check that player is in line of sight
	LogUtils::Log("Switcher is checking line of sight");
	if (!dungeon->HasLineOfSight(position, facing))
		return;

	bool shouldContinue = true;
	glm::ivec2 currentPosition = position + directions[facing];
	while (shouldContinue)
	{
		DungeonTile* tile = dungeon->GetTile(currentPosition);
		if (!tile)
			return;

		if (tile->occupied)
		{
			if (dungeon->player->GetPosition() == tile->position) // player is here. we should swap.
			{
				if (dungeon->switchersMustBeLookedAt)
				{
					if (facing == facingIndexesReversed[dungeon->player->GetOrientation()])
						dungeon->player->SetShouldSwitchWith(this);
				}
				else
					dungeon->player->SetShouldSwitchWith(this);
			}
			return;
		}

		if (dungeon->HasLineOfSight(currentPosition, facing))
			currentPosition += directions[facing];
		else
			return;
	}
}

void Crawl::DungeonEnemySwitcher::SwapWithPlayer()
{
	glm::ivec2 oldPosition = position;
	FACING_INDEX oldOrientation = facing;
	position = dungeon->player->GetPosition();
	facing = dungeon->player->GetOrientation();
	object->SetLocalPosition(dungeonPosToObjectScale(position));
	object->SetLocalRotationZ(orientationEulers[facing]);
	dungeon->player->Teleport(oldPosition);
	dungeon->player->Orient(oldOrientation);
}