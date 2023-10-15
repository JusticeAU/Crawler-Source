#include "DungeonEnemySlug.h"
#include "DungeonEnemySlugPath.h"
#include "Dungeon.h"
#include "Object.h"

#include "MathUtils.h"
#include "LogUtils.h"

#include "DungeonPlayer.h"
#include "DungeonEnemyChase.h"

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
			//fromTile->occupied = false;
			//toTile->occupied = true;
			positionPrevious = position;
			position += directions[facing];
		}
		else toTile = fromTile;

		state = MOVING;
		targetPosition = dungeonPosToObjectScale(position);
	}
	else
		LogUtils::Log("Slug is not on a path");

	// Check if player moved through us
	if (dungeon->player->GetPositionPrevious() == position && dungeon->player->GetPosition() == positionPrevious)
		dungeon->player->TakeDamage();

	// Check if a chaser has moved through us
	for (auto& chaser : dungeon->chasers)
	{
		if (dungeon->player->GetPositionPrevious() == chaser->position && dungeon->player->GetPosition() == chaser->positionPrevious)
			chaser->Kill(Dungeon::DamageType::Murderina);
	}
	
	moveCurrent = 0.0f;
	// Damage has been moved to PostUpdate
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
			object->SetLocalRotationZ(-180);
			state = IDLE;
		}
		else
		{
			object->SetLocalPosition(MathUtils::Lerp(oldPosition, targetPosition, t));
			object->SetLocalRotationZ(MathUtils::Lerp(-180, 180, t));

		}

		
		break;
	}
	}
}