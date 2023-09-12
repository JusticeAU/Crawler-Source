#include "DungeonShootLaser.h"
#include "Dungeon.h"
#include "DungeonMirror.h"
#include "Object.h"
#include "LogUtils.h"
#include "ComponentRenderer.h"
#include "MaterialManager.h"
#include "AudioManager.h"

Crawl::DungeonShootLaser::~DungeonShootLaser()
{
	if (object)
		object->markedForDeletion = true;
}

void Crawl::DungeonShootLaser::Update()
{
	LogUtils::Log("Shooter Updated");
	if (!primed && detectsLineOfSight)
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

			if (tile->occupied)
			{
				if(dungeon->GetMirrorAt(tile->position))
					return;

				LogUtils::Log("Detected object in line of sight.");
				if (firesImmediately)
				{
					LogUtils::Log("Fires Immediately");
					Fire();
				}
				else
					Prime();
				break;
			}

			if (dungeon->CanSee(currentPosition, facing))
				currentPosition += directions[facing];
			else
				haveTileToCheck = false;
		}

		if(!haveTileToCheck)
			LogUtils::Log("Nothing in line of sight.");
	}
	else if (primed)
	{
		if(turnPrimed == dungeon->turn)
			LogUtils::Log("Shooter already primed this turn.");
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
	AudioManager::PlaySound("crawler/sound/load/laser_prime.wav", object->GetWorldSpacePosition());
	((ComponentRenderer*)object->children[0]->children[0]->GetComponent(Component_Renderer))->materialArray[0] = MaterialManager::GetMaterial("crawler/material/prototype/shoot_laser_primed.material");
	primed = true;
	turnPrimed = dungeon->turn;
}

void Crawl::DungeonShootLaser::Fire()
{
	AudioManager::PlaySound("crawler/sound/load/laser_shoot.wav", object->localPosition);
	if (!firesProjectile) // full line of sight attack
	{
		LogUtils::Log("Shooter Fired a full line of sight attack");
		
		// Damage Line Of Sight
		bool shouldContinue = true;
		FACING_INDEX direction = facing;
		ivec2 currentPosition = position;
		while (shouldContinue)
		{
			dungeon->CreateDamageVisual(currentPosition);
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
				bool wasOccupied = tile->occupied;

				dungeon->DamageAtPosition(currentPosition, this, false, Dungeon::DamageType::Energy);

				if (wasOccupied)
					break;
			}

			if (dungeon->CanSee(currentPosition, direction))
				currentPosition += directions[direction];
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
	((ComponentRenderer*)object->children[0]->children[0]->GetComponent(Component_Renderer))->materialArray[0] = MaterialManager::GetMaterial("crawler/material/prototype/shoot_laser.material");
	primed = false;
}
