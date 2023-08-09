#include "DungeonEnemySwitcher.h"
#include "DungeonHelpers.h"
#include "DungeonTile.h"
#include "Dungeon.h"
#include "DungeonPlayer.h"
#include "DungeonMirror.h"
#include "DungeonPushableBlock.h"
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
	FACING_INDEX direction = facing;
	glm::ivec2 currentPosition = position + directions[direction];
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
					if (direction == facingIndexesReversed[dungeon->player->GetOrientation()])
						dungeon->player->SetShouldSwitchWith(this);
				}
				else
					dungeon->player->SetShouldSwitchWith(this);
			}
			else if (dungeon->IsPushableBlockAtPosition(tile->position))
			{
				LogUtils::Log("Swapping with block");
				DungeonPushableBlock* block = dungeon->GetPushableBlockAtPosition(tile->position);
				ivec2 newPos = block->position;
				block->position = position;
				position = newPos;
				object->SetLocalPosition(dungeonPosToObjectScale(position));
				block->object->SetLocalPosition(dungeonPosToObjectScale(block->position));
				block->targetPosition = dungeonPosToObjectScale(block->position);
				block->oldPosition = dungeonPosToObjectScale(block->position);
				return;
			}
			else if (dungeon->GetMirrorAt(tile->position))
			{
				// this is occupied by a mirror and we should continue but process it below!
				LogUtils::Log("There is a mirro here, testing for redirection");
				DungeonMirror* mirror = dungeon->GetMirrorAt(tile->position);
				int reflection = mirror->ShouldReflect(direction);
				if (reflection < 0)
					return;
				else
					direction = (FACING_INDEX)reflection;
			}
			else
				return;
		}

		if (dungeon->HasLineOfSight(currentPosition, direction))
		{

			currentPosition += directions[direction];
		}
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