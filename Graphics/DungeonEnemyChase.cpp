#include "DungeonEnemyChase.h"
#include "Object.h"
#include "Dungeon.h"
#include "DungeonPlayer.h"
#include "DungeonHelpers.h"
#include "MathUtils.h"

Crawl::DungeonEnemyChase::~DungeonEnemyChase()
{
	if (object)
		object->markedForDeletion = true;
}

void Crawl::DungeonEnemyChase::Update()
{
	object->SetLocalPosition(dungeonPosToObjectScale(position));
	oldPosition = dungeonPosToObjectScale(position);
	object->SetLocalRotationZ(orientationEulers[facing]);
	oldTurn = orientationEulers[facing];

	if (state == STUN)
	{
		state = IDLE;
		return;
	}

	// calculate path to player
	dungeon->FindPath(position, dungeon->player->GetPosition(), facing);
	
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
		state = TURNING;
		targetTurn = orientationEulers[facing];
		turnCurrent = -0.0f;
	}
	// else turn to face it
}

void Crawl::DungeonEnemyChase::ExecuteMove()
{
	DungeonTile* myTile = dungeon->GetTile(position);
	myTile->occupied = false;
	position = positionWant;
	myTile = dungeon->GetTile(position);
	myTile->occupied = true;
	state = MOVING;
	targetPosition = dungeonPosToObjectScale(position);
	moveCurrent = -0.0f;
}

void Crawl::DungeonEnemyChase::ExecuteDamage()
{
	dungeon->DamageAtPosition(position, this);
}

void Crawl::DungeonEnemyChase::UpdateVisuals(float delta)
{
	switch (state)
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
			state = IDLE;
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
			state = IDLE;
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
			state = IDLE;
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