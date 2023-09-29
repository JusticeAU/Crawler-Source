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

void Crawl::DungeonEnemySlug::UpdateTransform()
{
	object->AddLocalPosition(dungeonPosToObjectScale(position));
	object->SetLocalRotationZ(orientationEulers[facing]);
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
		// forward, left, then right, otherwise turn around.
		bool validPath = false;
		for (int i = 0; i < 4; i++)
		{
			FACING_INDEX testDirection = dungeonRotate(facing, slugTurns[i]);
			if ((onPath->maskTraverse & orientationMasksIndex[testDirection]) == orientationMasksIndex[testDirection])
			{
				facing = testDirection;
				validPath = true;
				break;
			}
		}

		DungeonTile* fromTile = dungeon->GetTile(position);
		DungeonTile* toTile;
		if (validPath)
		{
			toTile = dungeon->GetTile(position + directions[facing]);
			fromTile->occupied = false;
			toTile->occupied = true;
			position += directions[facing];
		}
		else toTile = fromTile;

		state = MOVING;
		targetPosition = dungeonPosToObjectScale(position);
	}
	else
		LogUtils::Log("Slug is not on a path");
	
	moveCurrent = 0.0f;
	dungeon->DamageAtPosition(position, this);
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