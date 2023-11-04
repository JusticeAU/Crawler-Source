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
#include "AudioManager.h"

Crawl::DungeonEnemyChase::~DungeonEnemyChase()
{
	if (object)
		object->markedForDeletion = true;
}

void Crawl::DungeonEnemyChase::UpdateTransform()
{
	object->SetLocalPosition(dungeonPosToObjectScale(position));
	object->SetLocalRotationZ(orientationEulers[facing]);
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
				NewAnimationState(AnimationState::Activating);
				// Play audio
				PlaySFXActivate();
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
		// We're performing a game update so we need to skip them to their actual live position if they havent finished updating yet.
		state = IDLE;
		object->SetLocalPosition(dungeonPosToObjectScale(position));
		object->SetLocalRotationZ(orientationEulers[facing]);
		if (animationState != AnimationState::Idle)
			NewAnimationState(AnimationState::Idle);

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
			if (positionWant == dungeon->player->GetPosition()) // kill player
			{
				dungeon->DamageAtPosition(positionWant, this, false, Dungeon::DamageType::Chaser);
				NewAnimationState(AnimationState::Killing);
				state = KILLING;
			}
			else state = MOVING;
		}
		else // Yoo we gotta turn.
		{
			animationTurnIsRight = IsClockWiseTurn(facing, (FACING_INDEX)myTile->toDestination->enterDirection);
			facing = dungeonRotateTowards(facing, (FACING_INDEX)myTile->toDestination->enterDirection);
			NewAnimationState(AnimationState::Turning);
			LogUtils::Log("Start Animation Turn");
		}
		// else turn to face it
	}
}

void Crawl::DungeonEnemyChase::ExecuteMove()
{
	DungeonTile* myTile = dungeon->GetTile(position);
	myTile->occupied = false;
	positionPrevious = position;
	position = positionWant;
	myTile = dungeon->GetTile(position);
	myTile->occupied = true;
	NewAnimationState(AnimationState::Walking);
}

void Crawl::DungeonEnemyChase::ExecuteDamage()
{
	dungeon->DamageAtPosition(position, this, false, Dungeon::DamageType::Chaser);
}

void Crawl::DungeonEnemyChase::Kick(FACING_INDEX inDirection)
{
	state = DungeonEnemyChase::STUN;
	// Clean up from any unfinished animations
	object->SetLocalPosition(dungeonPosToObjectScale(position));
	object->SetLocalRotationZ(orientationEulers[facing]);
	animator->StartAnimation(animationIdle, true);

	//targetPosition = dungeonPosToObjectScale(position + directions[inDirection]);
	positionWant = position + directions[inDirection];
	position = position + directions[inDirection];

	// Compute which animation to play
	switch (facing)
	{
	case NORTH_INDEX:
	{
		switch (inDirection)
		{
		case NORTH_INDEX:
		{
			stunAnimationToUse = animationPushFront;
			break;
		}
		case EAST_INDEX:
		{
			stunAnimationToUse = animationPushRight;
			break;
		}
		case SOUTH_INDEX:
		{
			stunAnimationToUse = animationPushBack;
			break;
		}
		case WEST_INDEX:
		{
			stunAnimationToUse = animationPushLeft;
			break;
		}
		}
		break;
	}
	case EAST_INDEX:
	{
		switch (inDirection)
		{
		case NORTH_INDEX:
		{
			stunAnimationToUse = animationPushLeft;
			break;
		}
		case EAST_INDEX:
		{
			stunAnimationToUse = animationPushFront;
			break;
		}
		case SOUTH_INDEX:
		{
			stunAnimationToUse = animationPushRight;
			break;
		}
		case WEST_INDEX:
		{
			stunAnimationToUse = animationPushBack;
			break;
		}
		}
		break;
	}
	case SOUTH_INDEX:
	{
		switch (inDirection)
		{
		case NORTH_INDEX:
		{
			stunAnimationToUse = animationPushBack;
			break;
		}
		case EAST_INDEX:
		{
			stunAnimationToUse = animationPushLeft;
			break;
		}
		case SOUTH_INDEX:
		{
			stunAnimationToUse = animationPushFront;
			break;
		}
		case WEST_INDEX:
		{
			stunAnimationToUse = animationPushRight;
			break;
		}
		}
		break;
	}
	case WEST_INDEX:
	{
		switch (inDirection)
		{
		case NORTH_INDEX:
		{
			stunAnimationToUse = animationPushRight;
			break;
		}
		case EAST_INDEX:
		{
			stunAnimationToUse = animationPushBack;
			break;
		}
		case SOUTH_INDEX:
		{
			stunAnimationToUse = animationPushLeft;
			break;
		}
		case WEST_INDEX:
		{
			stunAnimationToUse = animationPushFront;
			break;
		}
		}
		break;
	}

	}
	NewAnimationState(AnimationState::Stunned);
	//animator->BlendToAnimation(animToPlay, 0.1f); // little bit of blend cause this could be from inactive or idle (or any other animation actually)
}

void Crawl::DungeonEnemyChase::Bonk()
{
	animationState = DungeonEnemyChase::AnimationState::Bonked;
	animator->StartAnimation(animationBonk);
	positionWant = position;
}

void Crawl::DungeonEnemyChase::Kill(Dungeon::DamageType damageType)
{
	isDead = true;
	diedTo = damageType;
	switch (diedTo)
	{
	case Dungeon::DamageType::Spikes:
	{
		deathAnimationToUse = animationDeathSpike;
		break;
	}
	case Dungeon::DamageType::Shooter:
	{
		deathAnimationToUse = animationDeathLaser;
		break;
	}
	case Dungeon::DamageType::Blocker:
	{
		deathAnimationToUse = animationDeathBlocker;
		break;
	}
	default:
	{
		deathAnimationToUse = animationDeathBlocker;
		break;
	}

	}
	dungeon->GetTile(position)->occupied = false;
}

void Crawl::DungeonEnemyChase::UpdateVisuals(float delta)
{
	animationTime += delta;
	switch (animationState)
	{
	case AnimationState::Inactive:
	{
		if (isDead)
		{
			NewAnimationState(AnimationState::Dying);
		}
		break;
	}
	case AnimationState::Activating:
	{
		if (animator->current->IsFinished())
			NewAnimationState(AnimationState::Idle);
		break;
	}
	case AnimationState::Idle:
		if (isDead)
		{
			NewAnimationState(AnimationState::Dying);
			break;
		}
		if (animator->current->IsFinished()) animator->StartAnimation(animationIdle, true);
		break;
	case AnimationState::Turning:
	{
		if (animator->current->IsFinished())
		{
			object->SetLocalRotationZ(orientationEulers[facing]);
			NewAnimationState(AnimationState::Idle);
		}

		if(isDead) NewAnimationState(AnimationState::Dying);

		break;
	}
	case AnimationState::Walking:
	{
		if (animator->current->IsFinished()) // animation has finished
		{
			object->SetLocalPosition(dungeonPosToObjectScale(position));
			
			if (!isDead) NewAnimationState(AnimationState::Idle);
			else NewAnimationState(AnimationState::Dying);
		}
		break;
	}
	case AnimationState::Stunned:
	{
		if (animator->current->IsFinished() || (animationTime > animationMinStunTime && isDead)) // animation has finished
		{
			object->SetLocalPosition(dungeonPosToObjectScale(position));
			
			if (!isDead) NewAnimationState(AnimationState::Idle);
			else NewAnimationState(AnimationState::Dying);
		}

		break;
	}
	case AnimationState::Bonked:
	{
		if (animator->current->IsFinished()) // animation has finished
		{
			object->SetLocalPosition(dungeonPosToObjectScale(position));
			
			if (!isDead) NewAnimationState(AnimationState::Idle);
			else NewAnimationState(AnimationState::Dying);
		}
		break;
	}
	case AnimationState::Dying:
	{
		if (animator->IsFinished() && animationTime > 2.0f) // animations have finished
		{
			canRemove = true;
		}
		break;
	}
	}
}

void Crawl::DungeonEnemyChase::NewAnimationState(AnimationState newState, bool blend)
{
	animationTime = 0.0f;
	switch (newState)
	{
	case AnimationState::Inactive:
	{

		break;
	}
	case AnimationState::Activating:
	{
		animator->StartAnimation(animationActivate);
		animationState = AnimationState::Activating;
		break;
	}

	case AnimationState::Idle:
	{
		animator->StartAnimation(animationIdle, true);
		animationState = AnimationState::Idle;
		break;
	}
	case AnimationState::Walking:
	{
		animationState = AnimationState::Walking;
		if (animator) animator->StartAnimation(animationWalkForward);
		break;
	}
	case AnimationState::Turning:
	{
		if (animator)
		{
			if (animationTurnIsRight)	animator->StartAnimation(animationTurnRight); // need to differentiate between left and right turn here.
			else animator->StartAnimation(animationTurnLeft);
		}
		animationState = AnimationState::Turning;
		break;
	}
	case AnimationState::Bonked:
	{

		break;
	}
	case AnimationState::Stunned:
	{
		animator->BlendToAnimation(stunAnimationToUse, 0.1f); // little bit of blend cause this could be from inactive or idle (or any other animation actually)
		animationState = AnimationState::Stunned;
		break;
	}
	case AnimationState::Dying:
	{
		if (blend) animator->BlendToAnimation(deathAnimationToUse, 0.1f);
		else animator->StartAnimation(deathAnimationToUse);
		animationState = AnimationState::Dying;
		break;
	}
	case AnimationState::Killing:
	{
		if (blend) animator->BlendToAnimation(animationPlayerKill, 0.1f);
		else animator->StartAnimation(animationPlayerKill);
		animationState = AnimationState::Killing;
		break;
	}
	}
}

void Crawl::DungeonEnemyChase::PlaySFXActivate()
{
	int randomIndex = rand() % 13;
	AudioManager::PlaySound(audioActivateSFX[randomIndex], object->GetWorldSpacePosition());
}
