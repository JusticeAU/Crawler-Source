#include "DungeonEnemyChase.h"
#include "Object.h"
#include "Dungeon.h"
#include "DungeonPlayer.h"
#include "DungeonHelpers.h"
#include "DungeonMirror.h"
#include "MathUtils.h"
#include "LogUtils.h"	
#include "ComponentAnimator.h"
#include "Animation.h"

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
		if (!dungeon->CanSee(position, facing))
			return;

		bool shouldContinue = true;
		FACING_INDEX direction = facing;
		glm::ivec2 currentPosition = position + directions[direction];
		while (shouldContinue)
		{
			if (dungeon->player->GetPosition() == currentPosition) // player is here - activate!
			{
				LogUtils::Log("Chaser saw player - activating.");
				state = IDLE;
				stateVisual = IDLE;
				if (animator) animator->StartAnimation(animationActivate);
				return;
			}

			DungeonTile* tile = dungeon->GetTile(currentPosition);
			if (!tile) return;

			if (tile->occupied)
			{
				if (sightReflectsOffMirrors)
				{
					DungeonMirror* mirror = dungeon->GetMirrorAt(currentPosition);
					if (mirror)
					{
						int reflection = mirror->ShouldReflect(direction);
						if (reflection < 0)
							return;
						else
							direction = (FACING_INDEX)reflection;
					}
					else return;
				}
			}

			if (dungeon->CanSee(currentPosition, direction)) currentPosition += directions[direction];
			else return;
		}
	}
	else // Activated.
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
		else // Yoo we gotta turn.
		{
			bool isClockwise(IsClockWiseTurn(facing, (FACING_INDEX)myTile->toDestination->enterDirection));
			facing = dungeonRotateTowards(facing, (FACING_INDEX)myTile->toDestination->enterDirection);
			stateVisual = TURNING;
			targetTurn = orientationEulers[facing];
			turnCurrent = 0.0f;
			LogUtils::Log("Start Animation Turn");
			
			if (animator)
			{
				if(isClockwise)	animator->StartAnimation(animationTurnRight); // need to differentiate between left and right turn here.
				else animator->StartAnimation(animationTurnLeft);
			}
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
	LogUtils::Log("Start Animation Walk");
	if (animator) animator->StartAnimation(animationWalkForward);
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
		if (animator->current->IsFinished()) animator->StartAnimation(animationIdle, true);
		break;
	case TURNING:
	{
		if (isDead)
		{
			stateVisual = DYING;
			return;
		}
		if (animator->current->IsFinished())
		{
			object->SetLocalRotationZ(targetTurn);

			if (animator->current->IsFinished())
			{
				if (!isDead)
				{
					animator->StartAnimation(animationIdle, true);
					stateVisual = IDLE;
				}
				else stateVisual = DYING;
			}
		}
		break;
	}
	case MOVING:
	{
		if (animator->current->IsFinished()) // animation has finished
		{
			object->SetLocalPosition(targetPosition);
			
			if (animator->current->IsFinished())
			{
				if (!isDead)
				{
					animator->StartAnimation(animationIdle, true);
					stateVisual = IDLE;
				}
				else stateVisual = DYING;
			}
		}
		break;
	}
	case KICKED:
	{
		moveCurrent += delta;
		float t = MathUtils::InverseLerp(0, kickedSpeed, glm::max(0.0f, moveCurrent));
		if (moveCurrent > kickedSpeed)
		{
			object->SetLocalPosition(targetPosition);
			if (!isDead) stateVisual = IDLE;
			else stateVisual = DYING;
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
			if(!isDead) stateVisual = IDLE;
			else stateVisual = DYING;
		}
		else
		{
			if (t > 0.5f)
				t -= (t - 0.5) * 2;

			object->SetLocalPosition(MathUtils::Lerp(oldPosition, targetPosition, t));
		}
		break;
	}
	case DYING:
	{
		break;
	}
	}
}