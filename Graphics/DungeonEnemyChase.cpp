#include "DungeonEnemyChase.h"
#include "Object.h"
#include "Dungeon.h"
#include "DungeonPlayer.h"
#include "DungeonHelpers.h"
#include "MathUtils.h"
#include "LogUtils.h"	

Crawl::DungeonEnemyChase::~DungeonEnemyChase()
{
	if (object)
		object->markedForDeletion = true;
}

void Crawl::DungeonEnemyChase::Update()
{
	if (state == STUN)
	{
		state = IDLE;
		return;
	}
	if (state == INACTIVE)
	{
		// check that player is in line of sight
		LogUtils::Log("Chaser is checking line of sight");
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
				if (dungeon->player->GetPosition() == tile->position) // player is here - activate!
				{
					LogUtils::Log("Chaser saw player - activating.");
					state = IDLE;
				}
				else
					return;
			}

			if (dungeon->HasLineOfSight(currentPosition, direction))
				currentPosition += directions[direction];
			else
				return;
		}
	}
	else
	{
		stateVisual = IDLE;
		object->SetLocalPosition(dungeonPosToObjectScale(position));
		oldPosition = dungeonPosToObjectScale(position);
		object->SetLocalRotationZ(orientationEulers[facing]);
		oldTurn = orientationEulers[facing];

		// calculate path to player
		bool canPath = dungeon->FindPath(position, dungeon->player->GetPosition(), facing);

		if (!canPath)
		{
			LogUtils::Log("Lost player. Doing nothing.");
			return;
		}
		// get intended tile to move to
		DungeonTile* myTile = dungeon->GetTile(position);
		if (!myTile)
			return;
		// if tile is in direction we're facing, move to it
		if (facing == myTile->toDestination->enterDirection)
		{
			positionWant = myTile->toDestination->position;

			// We are facing the direction we want to go.
			// check if a door is blocking us.
			if (dungeon->IsDoorBlocking(myTile, facing))
			{
				positionWant = position;
				return;
			}

		}
		else
		{
			facing = dungeonRotateTowards(facing, (FACING_INDEX)myTile->toDestination->enterDirection);
			stateVisual = TURNING;
			targetTurn = orientationEulers[facing];
			turnCurrent = 0.0f;
		}
		// else turn to face it
	}
}

void Crawl::DungeonEnemyChase::ExecuteMove()
{
	DungeonTile* myTile = dungeon->GetTile(position);
	myTile->occupied = false;
	oldPosition = dungeonPosToObjectScale(position);
	position = positionWant;
	myTile = dungeon->GetTile(position);
	myTile->occupied = true;
	stateVisual = MOVING;
	targetPosition = dungeonPosToObjectScale(position);
	moveCurrent = -0.0f;
}

void Crawl::DungeonEnemyChase::ExecuteDamage()
{
	dungeon->DamageAtPosition(position, this);
}

void Crawl::DungeonEnemyChase::UpdateVisuals(float delta)
{
	switch (stateVisual)
	{
	case IDLE:
		break;
	case TURNING:
	{
		turnCurrent += delta;
		float t = MathUtils::InverseLerp(0, turnSpeed, glm::max(0.0f,turnCurrent));
		if (turnCurrent > turnSpeed)
		{
			object->SetLocalRotationZ(targetTurn);
			stateVisual = IDLE;
		}
		else
			object->SetLocalRotationZ(MathUtils::LerpDegrees(oldTurn, targetTurn, t));
		break;
	}
	case MOVING:
	{
		moveCurrent += delta;
		float t = MathUtils::InverseLerp(0, moveSpeed, glm::max(0.0f, moveCurrent));
		if (moveCurrent > moveSpeed)
		{
			object->SetLocalPosition(targetPosition);
			stateVisual = IDLE;
		}
		else
			object->SetLocalPosition(MathUtils::Lerp(oldPosition, targetPosition, t));
		break;
	}
	case KICKED:
	{
		moveCurrent += delta;
		float t = MathUtils::InverseLerp(0, kickedSpeed, glm::max(0.0f, moveCurrent));
		if (moveCurrent > kickedSpeed)
		{
			object->SetLocalPosition(targetPosition);
			stateVisual = IDLE;
		}
		else
			object->SetLocalPosition(MathUtils::Lerp(oldPosition, targetPosition, t));
		break;
	}
	case BOUNCING:
	{
		bounceCurrent += delta;
		float t = MathUtils::InverseLerp(0, bounceSpeed, glm::max(0.0f, bounceCurrent));
		if (bounceCurrent > bounceSpeed)
		{
			object->SetLocalPosition(oldPosition);
			stateVisual = IDLE;
		}
		else
		{
			if (t > 0.5f)
				t -= (t - 0.5) * 2;

			object->SetLocalPosition(MathUtils::Lerp(oldPosition, targetPosition, t));
		}
		break;
	}
	}
}