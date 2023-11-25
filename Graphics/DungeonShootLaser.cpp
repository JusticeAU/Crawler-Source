#include "DungeonShootLaser.h"
#include "Dungeon.h"
#include "DungeonMirror.h"
#include "Object.h"
#include "LogUtils.h"
#include "ComponentRenderer.h"
#include "MaterialManager.h"
#include "AudioManager.h"
#include "DungeonHelpers.h"
#include "ComponentRenderer.h"

Crawl::DungeonShootLaser::DungeonShootLaser()
{
	AudioManager::SetAudioSourceAttentuation(audioPrime, 2, 0.7);
	AudioManager::SetAudioSourceMinMaxDistance(audioPrime, 5, 15);

	AudioManager::SetAudioSourceAttentuation(audioShoot, 2, 0.7);
	AudioManager::SetAudioSourceMinMaxDistance(audioShoot, 5, 15);

}

Crawl::DungeonShootLaser::~DungeonShootLaser()
{
	if (object)
		object->markedForDeletion = true;
}

void Crawl::DungeonShootLaser::UpdateTransform()
{
	object->SetLocalPosition({ position.x * DUNGEON_GRID_SCALE, position.y * DUNGEON_GRID_SCALE, 0 });
	object->SetLocalRotationZ(orientationEulersReversed[facing]);
	renderer->emissiveScale = 0.0f;
}

void Crawl::DungeonShootLaser::Update()
{
	LogUtils::Log("Shooter Updated");
	if (justFired)
	{
		SetMouthOpen(false);
		justFired = false;
	}

	if (!primed && detectsLineOfSight)
	{
		ivec2 atPosition;
		void* thingAtCurrentPosition = AcquireTarget(atPosition);

		if (thingAtTargetPosition == thingAtCurrentPosition && atPosition == targetPosition)
		{
			LogUtils::Log("Same object in line of sight.");
			return;
		}

		LogUtils::Log("Detected new in line of sight.");
		if (firesImmediately)
		{
			LogUtils::Log("Fires Immediately");
			Fire();
		}
		else
		{
			thingAtTargetPosition = thingAtCurrentPosition;
			targetPosition = atPosition;
			Prime();
		}
	}
	else if (primed)
	{
		if(turnPrimed == dungeon->turn)
			LogUtils::Log("Shooter already primed this turn."); // from when pressure plates could activate them
		else
			Fire();
	}
}

void Crawl::DungeonShootLaser::Activate()
{
	LogUtils::Log("Shooter Activated");
	if (firesImmediately)
	{
		LogUtils::Log("Shooter fires immediately");
		Fire();
	}
	else
		Prime();
}

void Crawl::DungeonShootLaser::Prime()
{
	LogUtils::Log("Shooter has primed");
	AudioManager::PlaySound(audioPrime, object->GetWorldSpacePosition());
	SetMouthOpen();
	primed = true;
	turnPrimed = dungeon->turn;
}

void Crawl::DungeonShootLaser::Fire()
{
	AudioManager::PlaySound(audioShoot, object->GetWorldSpacePosition());
	if (!firesProjectile) // full line of sight attack
	{
		LogUtils::Log("Shooter Fired a full line of sight attack");

		// Damage Line Of Sight
		bool shouldContinue = true;
		FACING_INDEX direction = facing;
		ivec2 currentPosition = position;
		bool killedSomething = false;

		// Create a shot from mouth to centre of tile
		dungeon->CreateDamageVisual(currentPosition, facingIndexesReversed[direction]);

		while (shouldContinue)
		{
			DungeonTile* tile = dungeon->GetTile(currentPosition);
			if (!tile)
				break;

			DungeonMirror* mirror = dungeon->GetMirrorAt(currentPosition);
			if (mirror)
			{
				int reflection = mirror->ShouldReflect(direction);
				if (reflection < 0)
					break;
				else
					direction = (FACING_INDEX)reflection;
			}
			else
			{
				bool wasOccupied = tile->occupied || (dungeon->GetMurderinaAtPosition(currentPosition, false) != nullptr);

				dungeon->DamageAtPosition(currentPosition, this, false, Dungeon::DamageType::Shooter);

				// reset our current target
				thingAtTargetPosition = AcquireTarget(targetPosition);

				if (wasOccupied)
					break;
			}

			dungeon->CreateDamageVisual(currentPosition, direction);

			if (dungeon->CanSee(currentPosition, direction))
			{
				currentPosition += directions[direction];
				dungeon->CreateDamageVisual(currentPosition, facingIndexesReversed[direction]);
			}
			else
				break;
		}
	}
	else // spawn a slow moving projectile
	{
		LogUtils::Log("Shooter spawned a projectile");
		dungeon->CreateShootLaserProjectile(this, position, facing);
	}
	LogUtils::Log("Shooter is no longer primed");
	primed = false;
	justFired = true;
}

void Crawl::DungeonShootLaser::SetMouthOpen(bool open)
{
	if (open)
	{
		jawObject->SetLocalRotation({ jawOpenAngle, 0, 0 });
		renderer->emissiveScale = 1.0f;
	}
	else
	{
		jawObject->SetLocalRotation(vec3(0));
		renderer->emissiveScale = 0.0f;
	}
}

void* Crawl::DungeonShootLaser::AcquireTarget(ivec2& positionOut)
{
	// Check line of sight
	bool haveTileToCheck = true;
	ivec2 currentPosition = position;
	while (haveTileToCheck)
	{
		DungeonTile* tile = dungeon->GetTile(currentPosition);
		if (!tile)
		{
			haveTileToCheck = false;
			break;
		}

		void* thingAtCurrentPosition = dungeon->GetOccupyingObjectAtPosition(currentPosition);
		if (thingAtCurrentPosition)
		{
			positionOut = currentPosition;
			return thingAtCurrentPosition;
		}

		if (dungeon->CanSee(currentPosition, facing))
			currentPosition += directions[facing];
		else
			haveTileToCheck = false;
	}

	if (!haveTileToCheck)
		LogUtils::Log("Nothing in line of sight.");

	positionOut = currentPosition;
	return nullptr;
}

void Crawl::DungeonShootLaser::SetInitialTarget()
{
	thingAtTargetPosition = AcquireTarget(targetPosition);
}
