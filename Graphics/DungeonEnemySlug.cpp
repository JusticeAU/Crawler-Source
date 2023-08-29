#include "DungeonEnemySlug.h"
#include "DungeonEnemySlugPath.h"
#include "Dungeon.h"
#include "Object.h"

#include "MathUtils.h"
#include "LogUtils.h"

Crawl::DungeonEnemySlug::~DungeonEnemySlug()
{
	if (object)
		object->markedForDeletion = true;
}

void Crawl::DungeonEnemySlug::Update()
{
	object->SetLocalPosition(dungeonPosToObjectScale(position));
	oldPosition = dungeonPosToObjectScale(position);

	// Slug Path system
	// Get path at current position
	DungeonEnemySlugPath* onPath = dungeon->GetSlugPath(position);
	if (onPath)
	{
		// Check there is a path at current position + facing
		DungeonEnemySlugPath* toPath = dungeon->GetSlugPath(position + directions[facing]);

		// check left, then right, otherwise turn around.
		if (!toPath)
		{
			for (int i = 0; i < 3; i++)
			{
				toPath = dungeon->GetSlugPath(position + directions[dungeonRotate(facing, slugTurns[i])]);
				if (toPath)
				{
					facing = dungeonRotate(facing, slugTurns[i]);
					break;
				}
			}
		}

		DungeonTile* fromTile = dungeon->GetTile(position);
		DungeonTile* toTile = dungeon->GetTile(position + directions[facing]);
		fromTile->occupied = false;
		toTile->occupied = true;
		position += directions[facing];
		state = MOVING;
		targetPosition = dungeonPosToObjectScale(position);
		moveCurrent = -0.0f;
		dungeon->DamageAtPosition(position, this);
	}
	else
		LogUtils::Log("Slug is not on a path");
}

void Crawl::DungeonEnemySlug::UpdateVisuals(float delta)
{
	switch (state)
	{
	case IDLE:
		break;
	case MOVING:
	{
		moveCurrent += delta;
		float t = MathUtils::InverseLerp(0, moveSpeed, glm::max(0.0f, moveCurrent));
		if (moveCurrent > moveSpeed)
		{
			object->SetLocalPosition(targetPosition);
			state = IDLE;
		}
		else
			object->SetLocalPosition(MathUtils::Lerp(oldPosition, targetPosition, t));

		object->AddLocalRotation({ 0,0, delta * 200.0f });
		break;
	}
	}
}